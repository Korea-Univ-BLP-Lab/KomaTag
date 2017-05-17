#include <stdio.h>
#include <string>
#include <math.h>

#include "definitions.h"
#include "env.h"
#include "hsplit.h"
#include "probability_tool.h"
#include "get_morph_tag.h"

/******************************************************************************/
int neohantag_open(char *inter_transition_Path, char *intra_transition_Path,
                   PROB_MAP &inter_transition_prob, PROB_MAP &intra_transition_prob) {

  /* Ȯ���� �Է� */

  /* �ܺ����� Ȯ�� */
  fprintf(stderr, "\tReading inter-transition probabilities.. [%s]", inter_transition_Path);
  if (!map_scan_probability(inter_transition_Path, inter_transition_prob, "t")) {
    fprintf(stderr, "\t[ERROR] can't read the inter-transition probabilities! [%s]\n", inter_transition_Path);
    return 0;
  }
  fprintf(stderr, "\t[done]\n");

  /* �������� Ȯ�� */
  fprintf(stderr, "\tReading intra-transition probabilities.. [%s]", intra_transition_Path);
  if (!map_scan_probability(intra_transition_Path, intra_transition_prob, "t")) {
    fprintf(stderr, "\t[ERROR] can't read the intra-transition probabilities! [%s]\n", intra_transition_Path);
    return 0;
  }
  fprintf(stderr, "\t[done]\n");

  return 1;
}

/******************************************************************************/
/* Ȯ���� ���¼� �м� */
/* ���ϰ� : 0 = �м���� ����, 1 = �м� ��� ���� */
/* input_ej : �Է� ���� */
/* analyzed_result : �м� ��� (Ȯ������ �α�) */
int neohantag() {


  return 1;
}

/*****************************************************************************/
/* �±� ��� ��� */
/* ejs : ���峻 ���� */
/* morph_analyzed_result : (���) ���¼� �м� ��� */
/* num_word : ������ �� */
/* result : (���Ǽ� �ؼҵ�) �±� ��� (���¼� �м� ����� ��ȣ) */
void print_tagging_result(FILE *fp, vector<string> ejs, 
                          vector<ANALYZED_RESULT> &morph_analyzed_result, 
                          int num_word, int *tagging_result,
                          char delimiter, int output_style) {

  // ��� ��Ÿ��
  if (output_style) {

    /* �� ������ ���� */
    for (int i = 1; i <= num_word; i++) {
      /* ������, �м���� */
      fprintf(fp, "%s\t%s\n", ejs[i].c_str(), morph_analyzed_result[i][tagging_result[i]].second.c_str()); /* ���� */
    }
  }

  // ������ȹ ��Ÿ��
  else {
    int morph_num = 0; /* ���� ���� ���¼� �� */
    char morphs[MAX_WORD][MAX_WORD] = {0}; /* ���¼� �� */
    char tags[MAX_WORD][MAX_WORD] = {0}; /* ǰ�� �±� �� */
    int spacing_tags[MAX_WORD] = {0}; /* ���� �±� �� */

    // ��������
    for (int i = 1; i <= num_word; i++) {
      /* ������ */
      fprintf(fp, "%s\t", ejs[i].c_str());

      // �м����
      get_morphs_tags((char *)morph_analyzed_result[i][tagging_result[i]].second.c_str(), &morph_num,
                      morphs, tags, spacing_tags, delimiter);

      // ���¼Ҹ���
      for (int j = 0; j < morph_num; j++) {
        if (j) fprintf(fp, " + %s%c%s", morphs[j], delimiter, tags[j]); 
        else fprintf(fp, "%s%c%s", morphs[j], delimiter, tags[j]);
      }
      fprintf(fp, "\n");
    }
  }
}
