#include "definitions.h"
#include "probability_tool.h"
#include "extract_frequency.h"

#define TRANSITION_S_fst "TAG_S.fst"
#define TRANSITION_S_hash "TAG_S.hsh"
#define TRANSITION_S_PROB "TAG_S.prob"

#define LEXICAL_S_fst "SYLLABLE_S.fst"
#define LEXICAL_S_hash "SYLLABLE_S.hsh"
#define LEXICAL_S_PROB "SYLLABLE_S.prob"

#define SYLLABLE_DIC "syllable.dic"

/******************************************************************************/
/******************************************************************************/
// 분모+분자  / 분모
// PROB_MAP : "분모 분자" 순으로 저장
int get_MLE_2_probs(PROB_MAP &prob, CC_FREQ &_12_freq, C_FREQ &_1_freq) {

  CC_FREQ::iterator    itr;
  C_FREQ::iterator     itr2;

  for (itr = _12_freq.begin(); itr != _12_freq.end(); ++itr)  {
    for (itr2 = _12_freq[itr->first].begin(); itr2 != _12_freq[itr->first].end(); ++itr2) {

      if (_1_freq[itr->first] != 0) {
        prob[itr->first][itr2->first] = (double)itr2->second / _1_freq[itr->first];
      }
    }
  }

  return 1;
}

/******************************************************************************/
int prokoma_learn_s(char *filename) {

  C_FREQ     u_freq; // 태그
  C_FREQ     c_freq; // 음절

  CC_FREQ    cu_freq;
  CC_FREQ    uc_freq;

  CCC_FREQ   cuc_freq;
  CCC_FREQ   ucu_freq;

  CCCC_FREQ  ucuc_freq;
  CCCC_FREQ  cucu_freq;

  CCCCC_FREQ ucucu_freq;
  CCCCC_FREQ cucuc_freq;

  /* for 미등록 음절 */
  C_FREQ     c_type_freq; // 음절 유형
  CC_FREQ    c_type_u_freq; // 음절 유형 / 태그

  FILE *fp; /* 입력 화일 */

  int total_sentence = 0; /* 모든 문장의 수 */

  /*************************************************************************/
  /* 입력 화일 열기 */
  if ((fp = fopen (filename, "rt")) == NULL) {
    fprintf(stderr, "File open error : %s\n", filename);
    return 0;
  }

  fprintf(stderr, "Extracting information from [%s]..\n", filename);

  /* 화일로부터 (확률에 필요한 빈도) 정보를 추출 */
  total_sentence += extract_frequency_s(fp, u_freq, c_freq, uc_freq, cu_freq, 
                                      ucu_freq, cuc_freq, ucuc_freq, cucu_freq,
                                      ucucu_freq, cucuc_freq,
                                      c_type_freq, c_type_u_freq);

  fprintf(stderr, "Total %d of sentences are processed!\n", total_sentence);

  fclose (fp); /* 화일 닫기 */

  /* 태그셋 출력 */
//  fprintf(stderr, "Tagset\n");
//  for (C_FREQ::iterator it2 = u_freq.begin(); it2 != u_freq.end(); ++it2)  {
//    fprintf(stdout, "%s %d\n", it2->first.c_str(), it2->second);
//  }

  /*************************************************************************/
  /* 확률 추정 */
  PROB_MAP transition_prob, syllable_prob;

  calc_LINEAR_INTERPOL_probability5(transition_prob, 
                              ucucu_freq, ucuc_freq, cucu_freq, cuc_freq, 
                              ucu_freq, uc_freq, cu_freq, c_freq, u_freq);

  calc_LINEAR_INTERPOL_probability5(syllable_prob,
                              cucuc_freq, cucu_freq, ucuc_freq, ucu_freq, 
                              cuc_freq, cu_freq, uc_freq, u_freq, c_freq);

  /*************************************************************************/

  /* 계산된 확률을 출력 */

  /* 전이 확률*/
  fprintf(stderr, "printing transition probabilities.\n");
  fst_print_probability(TRANSITION_S_fst, TRANSITION_S_hash, TRANSITION_S_PROB, transition_prob);

  /* 어휘 확률 */
  fprintf(stderr, "printing lexical probabilities.\n");
  fst_print_probability(LEXICAL_S_fst, LEXICAL_S_hash, LEXICAL_S_PROB, syllable_prob);


  /* 어휘 사전 */
  {
    PROB_MAP syllable_dic;

    /* 어휘 사전을 구한다. */
    get_MLE_2_probs(syllable_dic, c_type_u_freq, c_type_freq);

    fprintf(stderr, "printing syllable dictionary.\n");
    map_print_probability(SYLLABLE_DIC, syllable_dic, "t"); 

  }

  /* 태그셋 출력 */
  /*fprintf(stderr, "Tagset\n");
  for (C_FREQ::iterator it = u_freq.begin(); it != u_freq.end(); ++it)  {
    fprintf(stdout, "%s %d\n", it->first.c_str(), it->second);
  }*/

  return 1;
}
