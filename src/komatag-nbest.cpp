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

  void *sb_fst; /* ���� �и��� */

  // ǰ�� �°ſ� ���ҽ�
  PROB_MAP intra_transition_prob; /* ���� ���� Ȯ�� */
  PROB_MAP inter_transition_prob; /* �ܺ� ���� Ȯ�� */

  WORD_FREQ full_morpheme_map;

  int result_sequence[1024];//MAX_WORD]; // �±� ���
  vector<ANALYZED_RESULT> morph_analyzed_result; // ���¼� �м� ��� (���� ���� ����)

  int total_ej_num = 0; // �� ���� ��
  int to_be_revised_ej_num = 0; // ���۾��ؾ� �� ���� ��

  int all_total_ej_num = 0; // �� ���� �� (��� ���Ͽ� ����)
  int all_to_be_revised_ej_num = 0; // ���۾��ؾ� �� ���� �� (��� ���Ͽ� ����)

  ct1 = clock ();

  /***************************************************************************/

  fprintf(stderr, "\n%s %s\n", Description, Version); /* ���α׷� ���� ��� */

  g = init_globals(NULL);  /* �޸� �Ҵ� �� �ʱ�ȭ */
  g->cmd = strdup(get_basename(argv[0], NULL)); /* �� ��ɾ� �̸��� ã�� ���� */
  
  /* �ɼ� �˾Ƴ��� */
  int inputstart = get_options(g, argc, argv);

  // �ɼ� ���
  if (verbosity > 3) print_globals(g);

  /* ȯ�� ���� �˻� */
  if (!komatag_CheckEnv(g->rsc_path)) return 0;

  /* ���� �и� ���� FST ���� */
  if (g->run_sentence_breaking) {
    sbd_open(&sb_fst, rsc_file_with_full_path[SB_FST]);
  }
  
  /* �°� �ʱ�ȭ */
  report(2, "Initializing the POS tagger..\n");

  // ǰ�� �°� ���ҽ� ����
  if (!neohantag_open(rsc_file_with_full_path[INTER_TRANS_PRB], rsc_file_with_full_path[INTRA_TRANS_PRB],
                      inter_transition_prob, intra_transition_prob)) {
    fprintf(stderr, "\n[ERROR] Initialization failure.\n");
    return 0;
  }

  report(2, "Initialization..\t[done]\n");

  /***************************************************************************/

  vector<string> words; /* ���峻 �ܾ� */
  int num_word;

  /* ȭ�� ���� ó�� */
  FILE *infp;    /* �Է� ���� ������ */
  FILE *outfp; /* ��� ���� ������ */
  int is_stdin = 0;

  char outfile_name[200];
  long filesize = 0;
  int progress = 0;

//  int ret;
  int num_no_result = 0; /* �̺м� ������ �� */
  int num_total_ej = 0; /* ������ �� �� */

  ///**/fprintf(stderr, "inputstart = %d, argc = %d\n", inputstart, argc);

  // ����� ���� ����
  int num_files_left = argc - inputstart;

  if (!num_files_left) is_stdin = 1; // ǥ�� ����� ó��

  /*************************************************************************/

  while (is_stdin || num_files_left) { // ǥ�� ����� ó���̰ų�, ���� ������ �ִ� ��쿡 �ݺ�

    if (is_stdin) { // ǥ�� ����� ó���̸�
      infp = stdin;
      outfp = stdout;
      is_stdin = 0; // �� �̻� �ݺ��ϸ� �ȵǴϱ�
    }

    else { // ���� ó���̸�
      if ((infp = fopen (argv[inputstart], "rt")) == NULL) {
        fprintf (stderr, "File open error : %s\n", argv[inputstart]);
        return 0;
      }

      /* ��� ���� ���� */
      sprintf (outfile_name, "%s.out", argv[inputstart]);
      if ((outfp = fopen (outfile_name, "wt")) == NULL) {
        fprintf (stderr, "File open error : %s\n", outfile_name);
        return 0;
      }

      /* ȭ�� ũ�� �˾Ƴ��� */
      fseek (infp, 0, SEEK_END); filesize = ftell (infp); fseek (infp, 0, SEEK_SET); 
      progress = -1;

      fprintf (stderr, " %s -> %s\n", argv[inputstart], outfile_name);

      inputstart++; // ���� ��ġ ����
      num_files_left--; // ���� ���� �� ����
    }

    while (1) { // ��� ���忡 ���� �ݺ�

      // ǥ�� �Է� ó���� �ƴϸ�
      if (!is_stdin) progress = print_progress(progress, filesize, infp); // ���� ���� ǥ��

      // ���� �Է� ////////////////////////////////////////////////////////////

      // ����и� �ȵ� ���¼� �м� ����� �Է�
      if (g->run_sentence_breaking) {
        num_word = get_sentence_from_morphological_analyzed_text_with_sbd(sb_fst, infp, words, morph_analyzed_result);
      }
      
      // ����и��� ���¼� �м� ����� �Է�
      else {
        num_word = get_sentence_from_morphological_analyzed_text(infp, words, morph_analyzed_result);
      }

      if (num_word == 0) break; // �Էµ� ������ ������ ����

      total_ej_num += num_word;

      ////////////////////////////////////////////////////////////////
      /* ǰ�� ���� */

      // ǰ���±�
      bigram_viterbi_search(intra_transition_prob, inter_transition_prob,
                            morph_analyzed_result, num_word, result_sequence, g->delimiter);


      /* �±� ��� ��� */
      // 1 ~ num_word
      to_be_revised_ej_num += 
              print_nbest_tagging_result2(outfp, words, morph_analyzed_result, num_word, result_sequence,
                                          g->delimiter, g->ku_style,
                                          g->constraint, g->relative_threshold, g->absolute_threshold);

      // ���� ��踦 ����Ѵ�.
      if (g->print_sentence_breaking) {
        fprintf(outfp, "\n");
      }
    } /* end of while */

    fprintf(stderr, "\r100%% done..\n"); // �Ϸ�
    
    /* ȭ�� �ݱ� */
    fclose (infp);
    fclose (outfp);

    fprintf(stderr, "�� ���� �� = %d\n", total_ej_num);
    fprintf(stderr, "���۾� �䱸 ���� �� = %d\n", to_be_revised_ej_num);
    fprintf(stderr, "���۾� �䱸�� = %.2lf\n", (double) to_be_revised_ej_num / total_ej_num * 100);

    all_total_ej_num += total_ej_num;
    all_to_be_revised_ej_num += to_be_revised_ej_num;

    total_ej_num = 0;
    to_be_revised_ej_num = 0;

  } // end of while

  /* ���� �и� ���� FST �ݱ� */
  if (g->run_sentence_breaking) {
    sbd_close(sb_fst);
  }

  free_globals(g);

  /*************************************************************************/

  fprintf(stderr, "\n(��� ������) �� ���� �� = %d\n", all_total_ej_num);
  fprintf(stderr, "(��� ������) ���۾� �䱸 ���� �� = %d\n", all_to_be_revised_ej_num);
  fprintf(stderr, "(���) ���۾� �䱸�� = %.2lf\n", 
                   (double) all_to_be_revised_ej_num / all_total_ej_num * 100);

  ct2 = clock (); // ��ħ �ð�
  fprintf (stderr, "Total time = %.2lf(sec)\n", (double) (ct2 - ct1) / CLOCKS_PER_SEC);

  return 1;
}

