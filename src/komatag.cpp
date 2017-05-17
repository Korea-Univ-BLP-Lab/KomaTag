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

  // ���¼� �м� ���� ���ҽ�
  /* ���� ���� �м� */
  void *rmej_fst;
  int *rmej_freq;
  char **rmej_info;

  /* ����� ���� */
  PROB_MAP phonetic_prob; /* ����� Ȯ�� */
  PROB_MAP phonetic_info; /* ����� ���� (�±�) */
  PROB_MAP syllable_dic; /* �̵�� ������ ���� �±� ���� */

  /* ���¼� ���� �м� */
  PROB_MAP transition_prob; /* ���� Ȯ�� */
  PROB_MAP lexical_prob; /* ���� Ȯ�� */

  /* ���� ���� �м� */
  void *tag_s_fst;
  double *tag_s_prob;

  void *syllable_s_fst;
  double *syllable_s_prob;

  void *s_transition_fst;

  void *sb_fst; /* ���� �и��� */

  // ǰ�� �°ſ� ���ҽ�
  PROB_MAP intra_transition_prob; /* ���� ���� Ȯ�� */
  PROB_MAP inter_transition_prob; /* �ܺ� ���� Ȯ�� */

#ifdef MAX_ENT_TAGGING
  // maximum entory model class.
  MaxentModel head_m;
  MaxentModel dummy_head_m;
  MaxentModel tail_m;
#endif

  WORD_FREQ full_morpheme_map;

  int result_sequence[1024];//MAX_WORD]; // �±� ���
  vector<ANALYZED_RESULT> morph_analyzed_result; // ���¼� �м� ��� (���� ���� ����)

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

  /*************************************************************************/
  /* ���¼Һм��� �ʱ�ȭ */
  if (!g->tagging_only) { // ǰ�� �±븸 �ϴ� ���� �ƴϸ�

    report(2, "Initializing the morphological analyzer..\n");

    /* ���¼� �м� ���ҽ� ���� */
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

  /* ���� �и� ���� FST ���� */
  if (g->run_sentence_breaking) {
    sbd_open(&sb_fst, rsc_file_with_full_path[SB_FST]);
  }
  
  /* �°� �ʱ�ȭ */
  if (!g->morphological_analysis_only) { // ���¼Һм��� �ϴ� ���� �ƴϸ�
    
    report(2, "Initializing the POS tagger..\n");

    #ifdef HMM_TAGGING
    // ǰ�� �°� ���ҽ� ����
    if (!neohantag_open(rsc_file_with_full_path[INTER_TRANS_PRB], 
                        rsc_file_with_full_path[INTRA_TRANS_PRB],
                        inter_transition_prob, intra_transition_prob)) {
      fprintf(stderr, "\n[ERROR] Initialization failure.\n");
      return 0;
    }
    #endif

    #ifdef HMM_TAGGING_EJ
    // ���� ���� ǰ�� �°� ���ҽ� ����
    if (!neohantag_open(rsc_file_with_full_path[INTER_TRANS_EJ_PRB], 
                        rsc_file_with_full_path[INTRA_TRANS_PRB],
                        inter_transition_prob, intra_transition_prob)) {
      fprintf(stderr, "\n[ERROR] Initialization failure.\n");
      return 0;
    }
    #endif

    #ifdef MAX_ENT_TAGGING
    // �ִ� ��Ʈ���� ��� �°� ���ҽ� ����
    if (!me_neohantag_open(head_m, tail_m, dummy_head_m, 
                           full_morpheme_map)) {
      fprintf(stderr, "\n[ERROR] Initialization failure.\n");
      return 0;
    }
    #endif

    report(2, "Initialization..\t[done]\n");
  }


  /***************************************************************************/
  vector<string> words; /* ���峻 �ܾ� */
  int num_word;

  /* ȭ�� ���� ó�� */
  FILE *infp;    /* �Է� ���� ������ */
  FILE *outfp; /* ��� ���� ������ */
  int is_stdin = 0; // ǥ�� ����� ó�� ����

  char outfile_name[200];
  long filesize = 0;
  int progress = 0;

  int ret;
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

      // ����и��� ���¼� �м� ����� �Է�
      if (g->tagging_only && !g->run_sentence_breaking) {
        num_word = get_sentence_from_morphological_analyzed_text(
                            infp, words, morph_analyzed_result);
      }

      // ����и� �ȵ� ���¼� �м� ����� �Է�
      else if (g->tagging_only && g->run_sentence_breaking) {
        num_word = get_sentence_from_morphological_analyzed_text_with_sbd(
                          sb_fst, infp, words, morph_analyzed_result);
      }

      // ����и� �ȵǾ��ִ� ��� (either row format or column format)
      else if (g->run_sentence_breaking) {
        morph_analyzed_result.clear();

        num_word = get_sentence_with_sbd(sb_fst, infp, words);
      }
    
      // row ���� + ����и� �Ǿ��ִ� ���
      else if (!g->column_format && !g->run_sentence_breaking) {
        num_word = get_sentence_from_row_format(infp, words);
      }

      // column ���� + ����и� �Ǿ��ִ� ���
      else if (g->column_format && !g->run_sentence_breaking) {
        num_word = get_sentence_from_column_format(infp, words);
      }

      if (num_word == 0) break; // �Էµ� ������ ������ ����

      if (!g->tagging_only) { // �±븸 �ϴ� ���� �ƴϸ� (���¼� �м��� �ؾ��ϸ�)

        { // �ʱ�ȭ (�±븸 �ϴ� ��쿡�� �� �ʿ���� (���� ���� �� �̹� �����Ƿ�)
          // 0��° ä��
          morph_analyzed_result.clear();
          
          ANALYZED_RESULT temp;
          temp.push_back(make_pair(1.0, BOW_TAG_1));
          morph_analyzed_result.push_back(temp);
        }

        // �� ������ ���� ���¼� �м�
        // 1���� �����ϴ� �Ϳ� ����!
        for (int i = 1; i <= num_word; i++) {
      
          ANALYZED_RESULT one_analyzed_result;

//          /**/total_word++;

          /* Ȯ���� ���¼� �м� */
          ret = prokoma(words[i].c_str(), rmej_fst, rmej_freq, rmej_info, 
                        phonetic_prob, phonetic_info, syllable_dic,
                        transition_prob, lexical_prob, 
                        tag_s_fst, tag_s_prob,
                        syllable_s_fst, syllable_s_prob,
                        s_transition_fst, 
                        g->cutoff_threshold_m, g->cutoff_threshold_s, g->beam_size,
                        one_analyzed_result, g->delimiter, g->processing_unit);

          morph_analyzed_result.push_back(one_analyzed_result); // �м���� ����

          if (!ret) num_no_result++;
          num_total_ej++;

        } /* end of for */

        // ���¼� �м��� �ϴ� ��쿡�� ����� ���� ����ؾ� ��
        if (g->morphological_analysis_only) {
          for (int i = 1; i <= num_word; i++) {
            // ���¼� �м� ��� ���
            // i��° ������ ���� ���
            print_analyzed_result(outfp, words[i].c_str(), morph_analyzed_result[i], 
                                  g->delimiter, g->ku_style);
          }
          // ���� ��踦 ����Ѵ�.
          if (g->print_sentence_breaking) {
            fprintf(outfp, "\n");
          }
        } // end of if
      } // end of if

      ////////////////////////////////////////////////////////////////
      /* ǰ�� ���� */
      if (!g->morphological_analysis_only) { // ���¼� �м��� �ϴ� ���� �ƴϸ�

        // ǰ���±�

        #ifdef HMM_TAGGING
        // HMM ��
        bigram_viterbi_search(intra_transition_prob, inter_transition_prob,
                              morph_analyzed_result, num_word, result_sequence, g->delimiter);
        #endif

        #ifdef MAX_ENT_TAGGING
        // �ִ� ��Ʈ���� ��
        me_viterbi_search(head_m, tail_m, dummy_head_m, full_morpheme_map,
                          morph_analyzed_result, num_word, result_sequence, g->delimiter);
        #endif

        #ifdef HMM_TAGGING_EJ
        // ���� ���� HMM ��
        bigram_viterbi_search_ej(inter_transition_prob,
                              morph_analyzed_result, num_word, result_sequence, g->delimiter);
        #endif

        /* �±� ��� ��� */
        // 1 ~ num_word
        
        print_tagging_result(outfp, words, morph_analyzed_result, num_word, result_sequence,
                              g->delimiter, g->ku_style);

        // ���� ��踦 ����Ѵ�.
        if (g->print_sentence_breaking) {
          fprintf(outfp, "\n");
        }
      }
    } /* end of while */

    fprintf(stderr, "\r100%% done..\n"); // �Ϸ�
    
    if (!g->tagging_only) {
      report(4, "%.3lf%% (%d/%d) Eojeols are not analyzed\n", (double) num_no_result / num_total_ej * 100, num_no_result, num_total_ej);
    }

    /* ȭ�� �ݱ� */
    fclose (infp);
    fclose (outfp);

  } // end of while

  /*************************************************************************/
  /* ���¼� �м� ���ҽ� �ݱ� */
  fprintf(stderr, "Closing the morphological analyzer..");

  if (!g->tagging_only) { // ǰ�� �±븸 �ϴ� ���� �ƴϸ�  
    prokoma_close(rmej_fst, rmej_info, rmej_freq, 
                  tag_s_fst, tag_s_prob, syllable_s_fst, syllable_s_prob,
                  s_transition_fst,
                  g->processing_unit);
     
    fprintf(stderr, "\t[done]\n");
  }

  /* ���� �и� ���� FST �ݱ� */
  if (g->run_sentence_breaking) {
    sbd_close(sb_fst);
  }

  free_globals(g);

  /*************************************************************************/
  ct2 = clock (); // ��ħ �ð�
  fprintf (stderr, "Total time = %.2lf(sec)\n", (double) (ct2 - ct1) / CLOCKS_PER_SEC);

  return 1;
}

