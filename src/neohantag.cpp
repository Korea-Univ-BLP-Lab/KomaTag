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

  /* 확률을 입력 */

  /* 외부전이 확률 */
  fprintf(stderr, "\tReading inter-transition probabilities.. [%s]", inter_transition_Path);
  if (!map_scan_probability(inter_transition_Path, inter_transition_prob, "t")) {
    fprintf(stderr, "\t[ERROR] can't read the inter-transition probabilities! [%s]\n", inter_transition_Path);
    return 0;
  }
  fprintf(stderr, "\t[done]\n");

  /* 내부전이 확률 */
  fprintf(stderr, "\tReading intra-transition probabilities.. [%s]", intra_transition_Path);
  if (!map_scan_probability(intra_transition_Path, intra_transition_prob, "t")) {
    fprintf(stderr, "\t[ERROR] can't read the intra-transition probabilities! [%s]\n", intra_transition_Path);
    return 0;
  }
  fprintf(stderr, "\t[done]\n");

  return 1;
}

/******************************************************************************/
/* 확률적 형태소 분석 */
/* 리턴값 : 0 = 분석결과 없음, 1 = 분석 결과 있음 */
/* input_ej : 입력 어절 */
/* analyzed_result : 분석 결과 (확률값은 로그) */
int neohantag() {


  return 1;
}

/*****************************************************************************/
/* 태깅 결과 출력 */
/* ejs : 문장내 어절 */
/* morph_analyzed_result : (모든) 형태소 분석 결과 */
/* num_word : 어절의 수 */
/* result : (중의성 해소된) 태깅 결과 (형태소 분석 결과의 번호) */
void print_tagging_result(FILE *fp, vector<string> ejs, 
                          vector<ANALYZED_RESULT> &morph_analyzed_result, 
                          int num_word, int *tagging_result,
                          char delimiter, int output_style) {

  // 고대 스타일
  if (output_style) {

    /* 각 어절에 대해 */
    for (int i = 1; i <= num_word; i++) {
      /* 원어절, 분석결과 */
      fprintf(fp, "%s\t%s\n", ejs[i].c_str(), morph_analyzed_result[i][tagging_result[i]].second.c_str()); /* 어절 */
    }
  }

  // 세종계획 스타일
  else {
    int morph_num = 0; /* 어절 내의 형태소 수 */
    char morphs[MAX_WORD][MAX_WORD] = {0}; /* 형태소 열 */
    char tags[MAX_WORD][MAX_WORD] = {0}; /* 품사 태그 열 */
    int spacing_tags[MAX_WORD] = {0}; /* 띄어쓰기 태그 열 */

    // 어절마다
    for (int i = 1; i <= num_word; i++) {
      /* 원어절 */
      fprintf(fp, "%s\t", ejs[i].c_str());

      // 분석결과
      get_morphs_tags((char *)morph_analyzed_result[i][tagging_result[i]].second.c_str(), &morph_num,
                      morphs, tags, spacing_tags, delimiter);

      // 형태소마다
      for (int j = 0; j < morph_num; j++) {
        if (j) fprintf(fp, " + %s%c%s", morphs[j], delimiter, tags[j]); 
        else fprintf(fp, "%s%c%s", morphs[j], delimiter, tags[j]);
      }
      fprintf(fp, "\n");
    }
  }
}
