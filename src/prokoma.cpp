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

  if (processing_unit & EOJEOL_ANALYSIS) { // 어절 단위 분석을 하는 경우만
    if (!prokoma_e_open(rsc_file_with_full_path[RMEJ_FST], 
                        rsc_file_with_full_path[RMEJ_HASH],
                        rsc_file_with_full_path[RMEJ_INFO], 
                        rsc_file_with_full_path[RMEJ_FREQ], 
                        rmej_fst, rmej_info, rmej_freq)) return 0;
  }

  if (processing_unit & (MORPHEME_ANALYSIS | SYLLABLE_ANALYSIS)) { // 형태소나 음절 단위 분석을 해야
    if (!prokoma_phonetic_open(rsc_file_with_full_path[PHONETIC_PRB], 
                               rsc_file_with_full_path[PHONETIC_INFO], 
                               rsc_file_with_full_path[SYLLABLE_DIC],
                               phonetic_prob, phonetic_info, syllable_dic)) return 0;
  }
  
  if (processing_unit & MORPHEME_ANALYSIS) { // 형태소 단위 분석을 하는 경우만
    if (!prokoma_m_open(rsc_file_with_full_path[LEXICAL_PRB], 
                        rsc_file_with_full_path[TRANSITION_PRB],
                        lexical_prob, transition_prob)) return 0;
  }

  if (processing_unit & SYLLABLE_ANALYSIS) { // 음절 단위 분석을 하는 경우만
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
  
  if (processing_unit & EOJEOL_ANALYSIS) { // 어절 단위 분석을 하는 경우
    prokoma_e_close(rmej_fst, rmej_info, rmej_freq);
  }

  if (processing_unit & SYLLABLE_ANALYSIS) { // 음절 단위 분석을 하는 경우
    prokoma_s_close(tag_s_fst, tag_s_prob, syllable_s_fst, syllable_s_prob);
  }

  if (s_transition_fst) FreeTransducer(s_transition_fst);

  return 1;
}

/******************************************************************************/
/* 확률적 형태소 분석 */
/* 리턴값 : 0 = 분석결과 없음, 1 = 분석 결과 있음 */
/* input_ej : 입력 어절 */
/* cutoff_threshold : (확률값이 낮은) 분석 결과수를 줄이기 위한 임계값 */
/* analyzed_result : 분석 결과 (확률값은 로그) */
int prokoma(const char *input_ej, 
            void *rmej_fst, int *rmej_freq, char **rmej_info, 
            PROB_MAP &phonetic_prob, PROB_MAP &phonetic_info, PROB_MAP &syllable_dic,
            PROB_MAP &transition_prob, PROB_MAP &lexical_prob, 
            void *tag_s_fst, double *tag_s_prob,
            void *syllable_s_fst, double *syllable_s_prob,
            void *s_transition_fst, 
            double cutoff_threshold_m, double cutoff_threshold_s, int beam_size,
            ANALYZED_RESULT &analyzed_result, char delimiter, int processing_unit) {

  int num_result = 0; /* 분석된 결과의 수 */

  analyzed_result.clear(); /* 분석 결과 초기화 */

  /* 어절 단위 분석 ******************************************************/
  if (processing_unit & EOJEOL_ANALYSIS) {

//    /**/num_eojeol_anal_try++;

    num_result = prokoma_e(rmej_fst, rmej_info, rmej_freq, input_ej, analyzed_result);
   
    /* 어절 단위 분석 결과가 있으면 */
    if (num_result) {
//      /**/num_eojeol_anal++;
      return EOJEOL;
    }
  }
  /*****************************************/

  // 형태소 단위 분석이나 음절 단위 분석이 있으면
  if (processing_unit & (MORPHEME_ANALYSIS | SYLLABLE_ANALYSIS)) {

    RESTORED_RESULT restored_ej; /* 음운 현상 복원 (확률 + 복원된 어절) */
    RESTORED_STAGS str_syl_tag_seq; /* 복원된 어절, 음절 태그 후보들 */

    /* 음운 현상 복원 */
    /* 입력 : input_ej */
    /* 출력 : restored_ej */
    /* 리턴값 : 복원된 어절의 수 */
    int syllable_only; // 미등록 음절이 있는 경우 1로 설정됨

    if (!phonetic_recovery(phonetic_prob, phonetic_info, syllable_dic,
                           input_ej, restored_ej, str_syl_tag_seq, syllable_only)) {
      return 0;
    }

    //#define DEBUG
    #ifdef DEBUG /**************************************************/
    /* 복원된 어절들 출력 */
    for (RESTORED_STAGS::iterator ssitr = str_syl_tag_seq.begin(); ssitr != str_syl_tag_seq.end(); ++ssitr) {
      fprintf(stderr, "\n[%s]\n", ssitr->first.c_str());
      print_syl_tag_seq(ssitr->second);
    }
    #endif /**************************************************/

    /**///fprintf(stderr, "syllable_only = %d\n", syllable_only);

    /* 미등록 음절이 있어서 형태소 단위 분석이 무의미한가? */
    if ((processing_unit & MORPHEME_ANALYSIS) && !syllable_only) {

//      /**/num_morpheme_anal_try++;

      /* 형태소 단위 분석 **************************************************/
      num_result = prokoma_m(transition_prob, lexical_prob, restored_ej, str_syl_tag_seq, 
                             analyzed_result, cutoff_threshold_m, delimiter);

      /* 형태소 단위 분석 결과가 있으면 */
      if (num_result) {
//        /**/num_morpheme_anal++;
        return MORPHEME;
      }
    }

    /* 음절 단위 분석 **************************************************/
    if (processing_unit & SYLLABLE_ANALYSIS) {

//      /**/num_syllable_anal_try++;

      num_result = prokoma_s(tag_s_fst, tag_s_prob,
                             syllable_s_fst, syllable_s_prob,
                             s_transition_fst,
                             restored_ej, str_syl_tag_seq, 
                             analyzed_result, cutoff_threshold_s, beam_size, delimiter);
  
      /* 음절 단위 분석 결과가 있으면 */
      if (num_result) {
//        /**/num_syllable_anal++;
        return SYLLABLE;
      }
    }
  }
  
  // 여기까지 오는 경우는 분석 결과가 없을 때
  {
    char unknown[100];
    sprintf(unknown, "%s%c%s", input_ej, delimiter, "??");
    string tt = unknown;

    analyzed_result.push_back(make_pair(1.0, tt)); /* 삽입 */
  }

  return 0;
}


/******************************************************************************/
/* 결과 출력 */
/* 리턴값 : 분석 결과의 수 */
int print_analyzed_result(FILE *fp, const char *input_ej, ANALYZED_RESULT &analyzed_result, 
                          char delimiter, int output_style) {

  int num_result = (int) analyzed_result.size();

  // 고대 스타일
  if (output_style) {
    for (int i = 0; i < num_result; i++) {
      
      if (!i) fprintf(fp, "%s", input_ej);

      /* 분석 결과, 확률 */
      fprintf(fp, "\t%s\t%12.11e\n", analyzed_result[i].second.c_str(), analyzed_result[i].first);
    }
  }

  // 세종계획 스타일
  else {
    int morph_num = 0; /* 어절 내의 형태소 수 */
    char morphs[MAX_WORD][MAX_WORD] = {0}; /* 형태소 열 */
    char tags[MAX_WORD][MAX_WORD] = {0}; /* 품사 태그 열 */
    int spacing_tags[MAX_WORD] = {0}; /* 띄어쓰기 태그 열 */

    for (int i = 0; i < num_result; i++) {
      if (i) fprintf(fp, "\t", input_ej);
      else fprintf(fp, "%s\t", input_ej); // 원어절

      get_morphs_tags((char *)analyzed_result[i].second.c_str(), &morph_num,
                        morphs, tags, spacing_tags, delimiter);


      // 분석 결과
      for (int j = 0; j < morph_num; j++) {
        if (j) fprintf(fp, " + %s%c%s", morphs[j], delimiter, tags[j]); 
        else fprintf(fp, "%s%c%s", morphs[j], delimiter, tags[j]);
      }

      // 확률
      fprintf(fp, "\t%12.11e\n", analyzed_result[i].first);
    }
  }

  return num_result; /* 분석된 결과의 수 */
}
