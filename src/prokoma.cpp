#include <stdio.h>
#include <math.h>
#include "FST.h"
#include "definitions.h"

#include "hsplit.h"
#include "phonetic_change.h"
#include "prokoma_e.h"
#include "prokoma_m.h"
#include "prokoma_s.h"
#include "get_morph_tag.h"

#include "env.h"

using namespace std;

/**/extern int num_eojeol_anal;
/**/extern int num_morpheme_anal;
/**/extern int num_syllable_anal;
/**/extern int num_eojeol_anal_try;
/**/extern int num_morpheme_anal_try;
/**/extern int num_syllable_anal_try;

/******************************************************************************/
int prokoma_open(void **rmej_fst, char ***rmej_info, int **rmej_freq, 
                 PROB_MAP &phonetic_prob, PROB_MAP &phonetic_info, PROB_MAP &syllable_dic,
                 PROB_MAP &transition_prob, PROB_MAP &lexical_prob,
                 void **tag_s_fst, double **tag_s_prob,
                 void **syllable_s_fst, double **syllable_s_prob,
                 void **s_transition_fst,
                 int processing_unit) {

  if (processing_unit & EOJEOL_ANALYSIS) { // ���� ���� �м��� �ϴ� ��츸
    if (!prokoma_e_open(rsc_file_with_full_path[RMEJ_FST], 
                        rsc_file_with_full_path[RMEJ_HASH],
                        rsc_file_with_full_path[RMEJ_INFO], 
                        rsc_file_with_full_path[RMEJ_FREQ], 
                        rmej_fst, rmej_info, rmej_freq)) return 0;
  }

  if (processing_unit & (MORPHEME_ANALYSIS | SYLLABLE_ANALYSIS)) { // ���¼ҳ� ���� ���� �м��� �ؾ�
    if (!prokoma_phonetic_open(rsc_file_with_full_path[PHONETIC_PRB], 
                               rsc_file_with_full_path[PHONETIC_INFO], 
                               rsc_file_with_full_path[SYLLABLE_DIC],
                               phonetic_prob, phonetic_info, syllable_dic)) return 0;
  }
  
  if (processing_unit & MORPHEME_ANALYSIS) { // ���¼� ���� �м��� �ϴ� ��츸
    if (!prokoma_m_open(rsc_file_with_full_path[LEXICAL_PRB], 
                        rsc_file_with_full_path[TRANSITION_PRB],
                        lexical_prob, transition_prob)) return 0;
  }

  if (processing_unit & SYLLABLE_ANALYSIS) { // ���� ���� �м��� �ϴ� ��츸
    if (!prokoma_s_open(rsc_file_with_full_path[TAG_S_FST], 
                        rsc_file_with_full_path[TAG_S_HASH], 
                        rsc_file_with_full_path[TAG_S_PROB],
                        rsc_file_with_full_path[SYLLABLE_S_FST], 
                        rsc_file_with_full_path[SYLLABLE_S_HASH],
                        rsc_file_with_full_path[SYLLABLE_S_PROB],
                        rsc_file_with_full_path[S_TRANSITION_FST], 
                        tag_s_fst, tag_s_prob,
                        syllable_s_fst, syllable_s_prob,
                        s_transition_fst)) return 0;
  }

  return 1;
}

/******************************************************************************/
int prokoma_close(void *rmej_fst, char **rmej_info, int *rmej_freq, 
                  void *tag_s_fst, double *tag_s_prob, void *syllable_s_fst, double *syllable_s_prob,
                  void *s_transition_fst,
                  int processing_unit) {
  
  if (processing_unit & EOJEOL_ANALYSIS) { // ���� ���� �м��� �ϴ� ���
    prokoma_e_close(rmej_fst, rmej_info, rmej_freq);
  }

  if (processing_unit & SYLLABLE_ANALYSIS) { // ���� ���� �м��� �ϴ� ���
    prokoma_s_close(tag_s_fst, tag_s_prob, syllable_s_fst, syllable_s_prob);
  }

  if (s_transition_fst) FreeTransducer(s_transition_fst);

  return 1;
}

/******************************************************************************/
/* Ȯ���� ���¼� �м� */
/* ���ϰ� : 0 = �м���� ����, 1 = �м� ��� ���� */
/* input_ej : �Է� ���� */
/* cutoff_threshold : (Ȯ������ ����) �м� ������� ���̱� ���� �Ӱ谪 */
/* analyzed_result : �м� ��� (Ȯ������ �α�) */
int prokoma(const char *input_ej, 
            void *rmej_fst, int *rmej_freq, char **rmej_info, 
            PROB_MAP &phonetic_prob, PROB_MAP &phonetic_info, PROB_MAP &syllable_dic,
            PROB_MAP &transition_prob, PROB_MAP &lexical_prob, 
            void *tag_s_fst, double *tag_s_prob,
            void *syllable_s_fst, double *syllable_s_prob,
            void *s_transition_fst, 
            double cutoff_threshold_m, double cutoff_threshold_s, int beam_size,
            ANALYZED_RESULT &analyzed_result, char delimiter, int processing_unit) {

  int num_result = 0; /* �м��� ����� �� */

  analyzed_result.clear(); /* �м� ��� �ʱ�ȭ */

  /* ���� ���� �м� ******************************************************/
  if (processing_unit & EOJEOL_ANALYSIS) {

//    /**/num_eojeol_anal_try++;

    num_result = prokoma_e(rmej_fst, rmej_info, rmej_freq, input_ej, analyzed_result);
   
    /* ���� ���� �м� ����� ������ */
    if (num_result) {
//      /**/num_eojeol_anal++;
      return EOJEOL;
    }
  }
  /*****************************************/

  // ���¼� ���� �м��̳� ���� ���� �м��� ������
  if (processing_unit & (MORPHEME_ANALYSIS | SYLLABLE_ANALYSIS)) {

    RESTORED_RESULT restored_ej; /* ���� ���� ���� (Ȯ�� + ������ ����) */
    RESTORED_STAGS str_syl_tag_seq; /* ������ ����, ���� �±� �ĺ��� */

    /* ���� ���� ���� */
    /* �Է� : input_ej */
    /* ��� : restored_ej */
    /* ���ϰ� : ������ ������ �� */
    int syllable_only; // �̵�� ������ �ִ� ��� 1�� ������

    if (!phonetic_recovery(phonetic_prob, phonetic_info, syllable_dic,
                           input_ej, restored_ej, str_syl_tag_seq, syllable_only)) {
      return 0;
    }

    //#define DEBUG
    #ifdef DEBUG /**************************************************/
    /* ������ ������ ��� */
    for (RESTORED_STAGS::iterator ssitr = str_syl_tag_seq.begin(); ssitr != str_syl_tag_seq.end(); ++ssitr) {
      fprintf(stderr, "\n[%s]\n", ssitr->first.c_str());
      print_syl_tag_seq(ssitr->second);
    }
    #endif /**************************************************/

    /**///fprintf(stderr, "syllable_only = %d\n", syllable_only);

    /* �̵�� ������ �־ ���¼� ���� �м��� ���ǹ��Ѱ�? */
    if ((processing_unit & MORPHEME_ANALYSIS) && !syllable_only) {

//      /**/num_morpheme_anal_try++;

      /* ���¼� ���� �м� **************************************************/
      num_result = prokoma_m(transition_prob, lexical_prob, restored_ej, str_syl_tag_seq, 
                             analyzed_result, cutoff_threshold_m, delimiter);

      /* ���¼� ���� �м� ����� ������ */
      if (num_result) {
//        /**/num_morpheme_anal++;
        return MORPHEME;
      }
    }

    /* ���� ���� �м� **************************************************/
    if (processing_unit & SYLLABLE_ANALYSIS) {

//      /**/num_syllable_anal_try++;

      num_result = prokoma_s(tag_s_fst, tag_s_prob,
                             syllable_s_fst, syllable_s_prob,
                             s_transition_fst,
                             restored_ej, str_syl_tag_seq, 
                             analyzed_result, cutoff_threshold_s, beam_size, delimiter);
  
      /* ���� ���� �м� ����� ������ */
      if (num_result) {
//        /**/num_syllable_anal++;
        return SYLLABLE;
      }
    }
  }
  
  // ������� ���� ���� �м� ����� ���� ��
  {
    char unknown[100];
    sprintf(unknown, "%s%c%s", input_ej, delimiter, "??");
    string tt = unknown;

    analyzed_result.push_back(make_pair(1.0, tt)); /* ���� */
  }

  return 0;
}


/******************************************************************************/
/* ��� ��� */
/* ���ϰ� : �м� ����� �� */
int print_analyzed_result(FILE *fp, const char *input_ej, ANALYZED_RESULT &analyzed_result, 
                          char delimiter, int output_style) {

  int num_result = (int) analyzed_result.size();

  // ��� ��Ÿ��
  if (output_style) {
    for (int i = 0; i < num_result; i++) {
      
      if (!i) fprintf(fp, "%s", input_ej);

      /* �м� ���, Ȯ�� */
      fprintf(fp, "\t%s\t%12.11e\n", analyzed_result[i].second.c_str(), analyzed_result[i].first);
    }
  }

  // ������ȹ ��Ÿ��
  else {
    int morph_num = 0; /* ���� ���� ���¼� �� */
    char morphs[MAX_WORD][MAX_WORD] = {0}; /* ���¼� �� */
    char tags[MAX_WORD][MAX_WORD] = {0}; /* ǰ�� �±� �� */
    int spacing_tags[MAX_WORD] = {0}; /* ���� �±� �� */

    for (int i = 0; i < num_result; i++) {
      if (i) fprintf(fp, "\t", input_ej);
      else fprintf(fp, "%s\t", input_ej); // ������

      get_morphs_tags((char *)analyzed_result[i].second.c_str(), &morph_num,
                        morphs, tags, spacing_tags, delimiter);


      // �м� ���
      for (int j = 0; j < morph_num; j++) {
        if (j) fprintf(fp, " + %s%c%s", morphs[j], delimiter, tags[j]); 
        else fprintf(fp, "%s%c%s", morphs[j], delimiter, tags[j]);
      }

      // Ȯ��
      fprintf(fp, "\t%12.11e\n", analyzed_result[i].first);
    }
  }

  return num_result; /* �м��� ����� �� */
}
