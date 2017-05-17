#include <stdio.h>
#include <time.h>               /* for debugging */
#include <string>
#include <vector>

#include "hsplit.h"
#include "FST.h"
#include "report.h"

#include "definitions.h"
#include "prokoma.h"

#include "env.h"
#include "common.h" // print_progress
#include "global_option.h"
#include "get_sentence.h"

#include "neohantag.h"

#ifdef MAX_ENT_TAGGING
#include "me_neohantag.h"
#include "me_viterbi.h"
#endif

///**/int num_eojeol_anal = 0;
///**/int num_morpheme_anal = 0;
///**/int num_syllable_anal = 0;
///**/int num_eojeol_anal_try = 0;
///**/int num_morpheme_anal_try = 0;
///**/int num_syllable_anal_try = 0;
///**/int total_word = 0;


/*****************************************************************************/
int main (int argc, char *argv[]) {
  clock_t ct1, ct2;

  // 형태소 분석 관련 리소스
  /* 어절 단위 분석 */
  void *rmej_fst;
  int *rmej_freq;
  char **rmej_info;

  /* 음운복원 관련 */
  PROB_MAP phonetic_prob; /* 음운복원 확률 */
  PROB_MAP phonetic_info; /* 음운복원 정보 (태그) */
  PROB_MAP syllable_dic; /* 미등록 음절에 대한 태그 저장 */

  /* 형태소 단위 분석 */
  PROB_MAP transition_prob; /* 전이 확률 */
  PROB_MAP lexical_prob; /* 어휘 확률 */

  /* 음절 단위 분석 */
  void *tag_s_fst;
  double *tag_s_prob;

  void *syllable_s_fst;
  double *syllable_s_prob;

  void *s_transition_fst;

  void *sb_fst; /* 문장 분리용 */

  // 품사 태거용 리소스
  PROB_MAP intra_transition_prob; /* 내부 전이 확률 */
  PROB_MAP inter_transition_prob; /* 외부 전이 확률 */

#ifdef MAX_ENT_TAGGING
  // maximum entory model class.
  MaxentModel head_m;
  MaxentModel dummy_head_m;
  MaxentModel tail_m;
#endif

  WORD_FREQ full_morpheme_map;

  int result_sequence[1024];//MAX_WORD]; // 태깅 결과
  vector<ANALYZED_RESULT> morph_analyzed_result; // 형태소 분석 결과 (어절 별로 저장)

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

  /*************************************************************************/
  /* 형태소분석기 초기화 */
  if (!g->tagging_only) { // 품사 태깅만 하는 것이 아니면

    report(2, "Initializing the morphological analyzer..\n");

    /* 형태소 분석 리소스 열기 */
    if (!prokoma_open(&rmej_fst, &rmej_info, &rmej_freq,
                      phonetic_prob, phonetic_info, syllable_dic,
                      transition_prob, lexical_prob, 
                      &tag_s_fst, &tag_s_prob,
                      &syllable_s_fst, &syllable_s_prob,
                      &s_transition_fst,
                      g->processing_unit)) {
      fprintf(stderr, "\n[ERROR] Initialization failure.\n");
      return 0;
    }
    report(2, "Initialization..\t[done]\n");


  }

  /* 문장 분리 정보 FST 열기 */
  if (g->run_sentence_breaking) {
    sbd_open(&sb_fst, rsc_file_with_full_path[SB_FST]);
  }
  
  /* 태거 초기화 */
  if (!g->morphological_analysis_only) { // 형태소분석만 하는 것이 아니면
    
    report(2, "Initializing the POS tagger..\n");

    #ifdef HMM_TAGGING
    // 품사 태거 리소스 열기
    if (!neohantag_open(rsc_file_with_full_path[INTER_TRANS_PRB], 
                        rsc_file_with_full_path[INTRA_TRANS_PRB],
                        inter_transition_prob, intra_transition_prob)) {
      fprintf(stderr, "\n[ERROR] Initialization failure.\n");
      return 0;
    }
    #endif

    #ifdef HMM_TAGGING_EJ
    // 어절 단위 품사 태거 리소스 열기
    if (!neohantag_open(rsc_file_with_full_path[INTER_TRANS_EJ_PRB], 
                        rsc_file_with_full_path[INTRA_TRANS_PRB],
                        inter_transition_prob, intra_transition_prob)) {
      fprintf(stderr, "\n[ERROR] Initialization failure.\n");
      return 0;
    }
    #endif

    #ifdef MAX_ENT_TAGGING
    // 최대 엔트로피 기반 태거 리소스 열기
    if (!me_neohantag_open(head_m, tail_m, dummy_head_m, 
                           full_morpheme_map)) {
      fprintf(stderr, "\n[ERROR] Initialization failure.\n");
      return 0;
    }
    #endif

    report(2, "Initialization..\t[done]\n");
  }


  /***************************************************************************/
  vector<string> words; /* 문장내 단어 */
  int num_word;

  /* 화일 단위 처리 */
  FILE *infp;    /* 입력 파일 포인터 */
  FILE *outfp; /* 출력 파일 포인터 */
  int is_stdin = 0; // 표준 입출력 처리 여부

  char outfile_name[200];
  long filesize = 0;
  int progress = 0;

  int ret;
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

      // 문장분리된 형태소 분석 결과를 입력
      if (g->tagging_only && !g->run_sentence_breaking) {
        num_word = get_sentence_from_morphological_analyzed_text(
                            infp, words, morph_analyzed_result);
      }

      // 문장분리 안된 형태소 분석 결과를 입력
      else if (g->tagging_only && g->run_sentence_breaking) {
        num_word = get_sentence_from_morphological_analyzed_text_with_sbd(
                          sb_fst, infp, words, morph_analyzed_result);
      }

      // 문장분리 안되어있는 경우 (either row format or column format)
      else if (g->run_sentence_breaking) {
        morph_analyzed_result.clear();

        num_word = get_sentence_with_sbd(sb_fst, infp, words);
      }
    
      // row 형식 + 문장분리 되어있는 경우
      else if (!g->column_format && !g->run_sentence_breaking) {
        num_word = get_sentence_from_row_format(infp, words);
      }

      // column 형식 + 문장분리 되어있는 경우
      else if (g->column_format && !g->run_sentence_breaking) {
        num_word = get_sentence_from_column_format(infp, words);
      }

      if (num_word == 0) break; // 입력된 문장이 없으면 종료

      if (!g->tagging_only) { // 태깅만 하는 것이 아니면 (형태소 분석을 해야하면)

        { // 초기화 (태깅만 하는 경우에는 할 필요없음 (문장 읽을 때 이미 했으므로)
          // 0번째 채움
          morph_analyzed_result.clear();
          
          ANALYZED_RESULT temp;
          temp.push_back(make_pair(1.0, BOW_TAG_1));
          morph_analyzed_result.push_back(temp);
        }

        // 각 어절에 대한 형태소 분석
        // 1부터 시작하는 것에 주의!
        for (int i = 1; i <= num_word; i++) {
      
          ANALYZED_RESULT one_analyzed_result;

//          /**/total_word++;

          /* 확률적 형태소 분석 */
          ret = prokoma(words[i].c_str(), rmej_fst, rmej_freq, rmej_info, 
                        phonetic_prob, phonetic_info, syllable_dic,
                        transition_prob, lexical_prob, 
                        tag_s_fst, tag_s_prob,
                        syllable_s_fst, syllable_s_prob,
                        s_transition_fst, 
                        g->cutoff_threshold_m, g->cutoff_threshold_s, g->beam_size,
                        one_analyzed_result, g->delimiter, g->processing_unit);

          morph_analyzed_result.push_back(one_analyzed_result); // 분석결과 저장

          if (!ret) num_no_result++;
          num_total_ej++;

        } /* end of for */

        // 형태소 분석만 하는 경우에는 결과를 지금 출력해야 함
        if (g->morphological_analysis_only) {
          for (int i = 1; i <= num_word; i++) {
            // 형태소 분석 결과 출력
            // i번째 어절에 대한 결과
            print_analyzed_result(outfp, words[i].c_str(), morph_analyzed_result[i], 
                                  g->delimiter, g->ku_style);
          }
          // 문장 경계를 출력한다.
          if (g->print_sentence_breaking) {
            fprintf(outfp, "\n");
          }
        } // end of if
      } // end of if

      ////////////////////////////////////////////////////////////////
      /* 품사 부착 */
      if (!g->morphological_analysis_only) { // 형태소 분석만 하는 것이 아니면

        // 품사태깅

        #ifdef HMM_TAGGING
        // HMM 모델
        bigram_viterbi_search(intra_transition_prob, inter_transition_prob,
                              morph_analyzed_result, num_word, result_sequence, g->delimiter);
        #endif

        #ifdef MAX_ENT_TAGGING
        // 최대 엔트로피 모델
        me_viterbi_search(head_m, tail_m, dummy_head_m, full_morpheme_map,
                          morph_analyzed_result, num_word, result_sequence, g->delimiter);
        #endif

        #ifdef HMM_TAGGING_EJ
        // 어절 단위 HMM 모델
        bigram_viterbi_search_ej(inter_transition_prob,
                              morph_analyzed_result, num_word, result_sequence, g->delimiter);
        #endif

        /* 태깅 결과 출력 */
        // 1 ~ num_word
        
        print_tagging_result(outfp, words, morph_analyzed_result, num_word, result_sequence,
                              g->delimiter, g->ku_style);

        // 문장 경계를 출력한다.
        if (g->print_sentence_breaking) {
          fprintf(outfp, "\n");
        }
      }
    } /* end of while */

    fprintf(stderr, "\r100%% done..\n"); // 완료
    
    if (!g->tagging_only) {
      report(4, "%.3lf%% (%d/%d) Eojeols are not analyzed\n", (double) num_no_result / num_total_ej * 100, num_no_result, num_total_ej);
    }

    /* 화일 닫기 */
    fclose (infp);
    fclose (outfp);

  } // end of while

  /*************************************************************************/
  /* 형태소 분석 리소스 닫기 */
  fprintf(stderr, "Closing the morphological analyzer..");

  if (!g->tagging_only) { // 품사 태깅만 하는 것이 아니면  
    prokoma_close(rmej_fst, rmej_info, rmej_freq, 
                  tag_s_fst, tag_s_prob, syllable_s_fst, syllable_s_prob,
                  s_transition_fst,
                  g->processing_unit);
     
    fprintf(stderr, "\t[done]\n");
  }

  /* 문장 분리 정보 FST 닫기 */
  if (g->run_sentence_breaking) {
    sbd_close(sb_fst);
  }

  free_globals(g);

  /*************************************************************************/
  ct2 = clock (); // 마침 시간
  fprintf (stderr, "Total time = %.2lf(sec)\n", (double) (ct2 - ct1) / CLOCKS_PER_SEC);

  return 1;
}

