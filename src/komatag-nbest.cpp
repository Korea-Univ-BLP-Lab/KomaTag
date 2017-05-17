#include <stdio.h>
#include <time.h>               /* for debugging */
#include <string>
#include <vector>

#include "report.h"

#include "definitions.h"
#include "prokoma.h"

#include "env.h"
#include "common.h"
#include "global_option-nbest.h"
#include "get_sentence.h"

#include "neohantag.h"
#include "constraint-nbest.h"

//#include "me_neohantag.h"
//#include "me_viterbi.h"

//#include "maxentmodel.hpp"

//using namespace maxent;

/*****************************************************************************/
int main (int argc, char *argv[]) {
  clock_t ct1, ct2;

  void *sb_fst; /* 문장 분리용 */

  // 품사 태거용 리소스
  PROB_MAP intra_transition_prob; /* 내부 전이 확률 */
  PROB_MAP inter_transition_prob; /* 외부 전이 확률 */

  WORD_FREQ full_morpheme_map;

  int result_sequence[1024];//MAX_WORD]; // 태깅 결과
  vector<ANALYZED_RESULT> morph_analyzed_result; // 형태소 분석 결과 (어절 별로 저장)

  int total_ej_num = 0; // 총 어절 수
  int to_be_revised_ej_num = 0; // 수작업해야 할 어절 수

  int all_total_ej_num = 0; // 총 어절 수 (모든 파일에 대해)
  int all_to_be_revised_ej_num = 0; // 수작업해야 할 어절 수 (모든 파일에 대해)

  ct1 = clock ();

  /***************************************************************************/

  fprintf(stderr, "\n%s %s\n", Description, Version); /* 프로그램 정보 출력 */

  g = init_globals(NULL);  /* 메모리 할당 및 초기화 */
  g->cmd = strdup(get_basename(argv[0], NULL)); /* 순 명령어 이름만 찾기 위해 */
  
  /* 옵션 알아내기 */
  int inputstart = get_options(g, argc, argv);

  // 옵션 출력
  if (verbosity > 3) print_globals(g);

  /* 환경 설정 검사 */
  if (!komatag_CheckEnv(g->rsc_path)) return 0;

  /* 문장 분리 정보 FST 열기 */
  if (g->run_sentence_breaking) {
    sbd_open(&sb_fst, rsc_file_with_full_path[SB_FST]);
  }
  
  /* 태거 초기화 */
  report(2, "Initializing the POS tagger..\n");

  // 품사 태거 리소스 열기
  if (!neohantag_open(rsc_file_with_full_path[INTER_TRANS_PRB], rsc_file_with_full_path[INTRA_TRANS_PRB],
                      inter_transition_prob, intra_transition_prob)) {
    fprintf(stderr, "\n[ERROR] Initialization failure.\n");
    return 0;
  }

  report(2, "Initialization..\t[done]\n");

  /***************************************************************************/

  vector<string> words; /* 문장내 단어 */
  int num_word;

  /* 화일 단위 처리 */
  FILE *infp;    /* 입력 파일 포인터 */
  FILE *outfp; /* 출력 파일 포인터 */
  int is_stdin = 0;

  char outfile_name[200];
  long filesize = 0;
  int progress = 0;

//  int ret;
  int num_no_result = 0; /* 미분석 어절의 수 */
  int num_total_ej = 0; /* 어절의 총 수 */

  ///**/fprintf(stderr, "inputstart = %d, argc = %d\n", inputstart, argc);

  // 입출력 파일 설정
  int num_files_left = argc - inputstart;

  if (!num_files_left) is_stdin = 1; // 표준 입출력 처리

  /*************************************************************************/

  while (is_stdin || num_files_left) { // 표준 입출력 처리이거나, 남은 파일이 있는 경우에 반복

    if (is_stdin) { // 표준 입출력 처리이면
      infp = stdin;
      outfp = stdout;
      is_stdin = 0; // 더 이상 반복하면 안되니까
    }

    else { // 파일 처리이면
      if ((infp = fopen (argv[inputstart], "rt")) == NULL) {
        fprintf (stderr, "File open error : %s\n", argv[inputstart]);
        return 0;
      }

      /* 출력 파일 열기 */
      sprintf (outfile_name, "%s.out", argv[inputstart]);
      if ((outfp = fopen (outfile_name, "wt")) == NULL) {
        fprintf (stderr, "File open error : %s\n", outfile_name);
        return 0;
      }

      /* 화일 크기 알아내기 */
      fseek (infp, 0, SEEK_END); filesize = ftell (infp); fseek (infp, 0, SEEK_SET); 
      progress = -1;

      fprintf (stderr, " %s -> %s\n", argv[inputstart], outfile_name);

      inputstart++; // 파일 위치 증가
      num_files_left--; // 남은 파일 수 감소
    }

    while (1) { // 모든 문장에 대해 반복

      // 표준 입력 처리가 아니면
      if (!is_stdin) progress = print_progress(progress, filesize, infp); // 진행 상태 표시

      // 문장 입력 ////////////////////////////////////////////////////////////

      // 문장분리 안된 형태소 분석 결과를 입력
      if (g->run_sentence_breaking) {
        num_word = get_sentence_from_morphological_analyzed_text_with_sbd(sb_fst, infp, words, morph_analyzed_result);
      }
      
      // 문장분리된 형태소 분석 결과를 입력
      else {
        num_word = get_sentence_from_morphological_analyzed_text(infp, words, morph_analyzed_result);
      }

      if (num_word == 0) break; // 입력된 문장이 없으면 종료

      total_ej_num += num_word;

      ////////////////////////////////////////////////////////////////
      /* 품사 부착 */

      // 품사태깅
      bigram_viterbi_search(intra_transition_prob, inter_transition_prob,
                            morph_analyzed_result, num_word, result_sequence, g->delimiter);


      /* 태깅 결과 출력 */
      // 1 ~ num_word
      to_be_revised_ej_num += 
              print_nbest_tagging_result2(outfp, words, morph_analyzed_result, num_word, result_sequence,
                                          g->delimiter, g->ku_style,
                                          g->constraint, g->relative_threshold, g->absolute_threshold);

      // 문장 경계를 출력한다.
      if (g->print_sentence_breaking) {
        fprintf(outfp, "\n");
      }
    } /* end of while */

    fprintf(stderr, "\r100%% done..\n"); // 완료
    
    /* 화일 닫기 */
    fclose (infp);
    fclose (outfp);

    fprintf(stderr, "총 어절 수 = %d\n", total_ej_num);
    fprintf(stderr, "수작업 요구 어절 수 = %d\n", to_be_revised_ej_num);
    fprintf(stderr, "수작업 요구율 = %.2lf\n", (double) to_be_revised_ej_num / total_ej_num * 100);

    all_total_ej_num += total_ej_num;
    all_to_be_revised_ej_num += to_be_revised_ej_num;

    total_ej_num = 0;
    to_be_revised_ej_num = 0;

  } // end of while

  /* 문장 분리 정보 FST 닫기 */
  if (g->run_sentence_breaking) {
    sbd_close(sb_fst);
  }

  free_globals(g);

  /*************************************************************************/

  fprintf(stderr, "\n(모든 파일의) 총 어절 수 = %d\n", all_total_ej_num);
  fprintf(stderr, "(모든 파일의) 수작업 요구 어절 수 = %d\n", all_to_be_revised_ej_num);
  fprintf(stderr, "(평균) 수작업 요구율 = %.2lf\n", 
                   (double) all_to_be_revised_ej_num / all_total_ej_num * 100);

  ct2 = clock (); // 마침 시간
  fprintf (stderr, "Total time = %.2lf(sec)\n", (double) (ct2 - ct1) / CLOCKS_PER_SEC);

  return 1;
}

