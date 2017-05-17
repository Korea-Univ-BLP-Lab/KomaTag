#include <stdio.h>
#include "definitions.h"
#include "extract_frequency.h"
#include "probability_tool.h"
#include "report.h"
#include "global_option.h"

#define INTRA_TRANS "INTRA_TRANS.prb"
#define INTER_TRANS "INTER_TRANS.prb"

//#define INTRA_TRANS_EJ "INTRA_TRANS_EJ.prb"
#define INTER_TRANS_EJ "INTER_TRANS_EJ.prb"

#define INTER_TRANS_EJ_BY_EJ "INTER_TRANS_EJ_BY_EJ.prb"


int neohantag_learn(char *filename, char delimiter) {

  C_FREQ   tag_unigram_freq;
  CC_FREQ  tag_bigram_freq;

  C_FREQ   inter_tag_unigram_freq;
  CC_FREQ  inter_tag_bigram_freq;

  C_FREQ   inter_r_unigram_freq;
  CC_FREQ  inter_r_bigram_freq;

  FILE *fp; /* �Է� ȭ�� */

  int total_sentence = 0; /* ��� ������ �� */


  /* ȭ�� ���� */
  if ((fp = fopen (filename, "rt")) == NULL) {
    error("File open error : %s\n", filename);
    return 0;
  }

  report(2, "Extracting information from %s\n", filename);

  /* ȭ�Ϸκ��� (Ȯ���� �ʿ��� ��) ������ ���� */
  total_sentence += 

#ifdef BIGRAM_TAGGING
           // bigram ��
           extract_frequency_bigram (fp, tag_unigram_freq, tag_bigram_freq, 
                              inter_tag_unigram_freq, inter_tag_bigram_freq,
                              delimiter);
#endif
#ifdef TRIGRAM_TAGGING
           // trigram ��
           extract_frequency_trigram (fp, tag_unigram_freq, tag_bigram_freq,
                              inter_tag_unigram_freq, inter_tag_bigram_freq,
                              delimiter);
#endif


  report(1, "Total %d of sentences are processed!\n", total_sentence);

  rewind(fp); // ó�� ��ġ�� ������

  // ���� ���� bigram ��
  extract_frequency_bigram_ej_by_ej (fp, inter_r_unigram_freq, inter_r_bigram_freq,
                              delimiter);

  report(1, "Total %d of sentences are processed!\n", total_sentence);

  fclose (fp); /* ȭ�� �ݱ� */

  /*************************************************************************/
  /* Ȯ�� ���� */
  PROB_MAP intra_transition_prob, inter_transition_prob;
  PROB_MAP inter_transition_prob_ej_by_ej;
  
  report(1, "Calculating probabilities!\n");

  calc_MLE_probability_with_freq_threshold(tag_unigram_freq, tag_bigram_freq, 
                                           intra_transition_prob, 3);

  calc_MLE_probability_with_freq_threshold(inter_tag_unigram_freq, inter_tag_bigram_freq, 
                                           inter_transition_prob, 3);

  calc_MLE_probability_with_freq_threshold(inter_r_unigram_freq, inter_r_bigram_freq, 
                                           inter_transition_prob_ej_by_ej, 5); // �� 5 �̻� ����

  /*************************************************************************/

  /* ���� Ȯ���� ��� */

  report(1, "printing intra-transition probabilities.\n");
  map_print_probability(INTRA_TRANS, intra_transition_prob, "t"); /* ������ ���� Ȯ��*/

  report(1, "printing inter-transition probabilities.\n");
  map_print_probability(INTER_TRANS, inter_transition_prob, "t"); /* ������ ���� Ȯ�� */

  report(1, "printing inter-transition ej by ej probabilities.\n");
  map_print_probability(INTER_TRANS_EJ_BY_EJ, inter_transition_prob_ej_by_ej, "t"); /* ������ ���� Ȯ�� */
  return 1;
}

/*****************************************************************************/
// ���� ���� HMM �н�
int neohantag_ej_learn(char *filename, char delimiter) {

  C_FREQ   inter_tag_unigram_freq;
  CC_FREQ  inter_tag_bigram_freq;

  FILE *fp; /* �Է� ȭ�� */

  int total_sentence = 0; /* ��� ������ �� */

  /* ȭ�� ���� */
  if ((fp = fopen (filename, "rt")) == NULL) {
    error("File open error : %s\n", filename);
    return 0;
  }

  report(2, "Extracting information from %s\n", filename);

  /* ȭ�Ϸκ��� (Ȯ���� �ʿ��� ��) ������ ���� */
  total_sentence += 
           // ���� ���� bigram ��
           extract_frequency_bigram_ej (fp, inter_tag_unigram_freq, inter_tag_bigram_freq,
                              delimiter);


  report(1, "Total %d of sentences are processed!\n", total_sentence);

  fclose (fp); /* ȭ�� �ݱ� */

  /*************************************************************************/
  /* Ȯ�� ���� */
  PROB_MAP inter_transition_prob;

  report(1, "Calculating probabilities!\n");
  
  calc_MLE_probability_with_freq_threshold(inter_tag_unigram_freq, inter_tag_bigram_freq, 
                                           inter_transition_prob, 3);

  /*************************************************************************/

  /* ���� Ȯ���� ��� */
  report(1, "printing inter-transition probabilities.\n");
  map_print_probability(INTER_TRANS_EJ, inter_transition_prob, "t"); /* ������ ���� Ȯ�� */

  return 1;
}
