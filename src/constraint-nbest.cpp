#include <stdio.h>
#include <string>
#include <vector>
#include <string.h>
#include <math.h>

#include "definitions.h"
#include "hsplit.h"
#include "get_morph_tag.h"

/*****************************************************************************/
// 문자열 s에서 문자 c가 나타난 회수를 리턴한다.
int count_strchr(const char *s, int c) {
  char *ptr = (char *)s;
  char *ptr2;
  int count = 0;
  
  while ((ptr2 = strchr(ptr, c)) != NULL) {
    count++;
    ptr = ++ptr2;
  }
  return count;
}

/*****************************************************************************/
/* 태깅 결과 출력 */
/* ejs : 문장내 어절 */
/* morph_analyzed_result : (모든) 형태소 분석 결과 */
/* num_word : 어절의 수 */
/* tagging_result : (중의성 해소된) 태깅 결과 (형태소 분석 결과의 번호) */
void print_nbest_tagging_result(FILE *fp, vector<string> ejs, 
                          vector<ANALYZED_RESULT> &morph_analyzed_result, 
                          int num_word, int *tagging_result,
                          char delimiter, int output_style, int constraint,
                          int relative_threshold, int absolute_threshold) {
                            
  int num_result;
  int print_nbest = 0;
  double relative_ratio;
  int num_morph;

  // 각 어절에 대해
  for (int i = 1; i <= num_word; i++) {
    
    num_result = (int) morph_analyzed_result[i].size(); // 어절의 형태소 분석 결과의 수
    
    string anal_result = morph_analyzed_result[i][0].second; // 첫 번째 형태소 분석 결과
    
    //**/fprintf(fp, "%s num_result = %d, tagging_result = %d\n", ejs[i].c_str(), num_result, tagging_result[i]);


    // 분석 결과가 오직 하나이고, 미분석이 아니라도 n-best 출력이 필요한 경우도 있음 (절대임계값에 의해)

    // 형태소 분석과 태거의 일치 제약
    // 제약을 만족하지 못하면
    // 태깅 결과와 형태소 분석 결과가 같지 않거나, 미분석이면
    if (tagging_result[i] != 0 || strstr(anal_result.c_str(), "??") != NULL) {
      print_nbest = 1;
    }
    
    // 형태소 분석 확률 상대 임계값 제약
    if (!print_nbest && (constraint & RELATIVE_MORPH_CONSTRAINT)) {
      
      if (num_result > 1) {

        relative_ratio = log(morph_analyzed_result[i][0].first) - log(morph_analyzed_result[i][1].first); // 두 확률의 차이
        
        // 임계값보다 크지 않으면 여러 분석 결과를 출력
        // 임계값이 크면 확정 단어율이 낮아짐 (확정 정확도는 높아짐)
        // 임계값이 작으면... 약간의 확률 차이만 나도 하나의 결과만 출력하므로 확정 단어율이 높아짐 (대신 확정 정확도는 낮아지겠지)
        if (relative_ratio < relative_threshold) { 
          print_nbest = 1;
        }
      }
    }
    
    // 형태소 분석 확률 절대 임계값 제약
    if (!print_nbest && (constraint & ABSOLUTE_MORPH_CONSTRAINT)) {
      num_morph = count_strchr(anal_result.c_str(), '+');
      num_morph++;
      
      if (log(morph_analyzed_result[i][0].first) < log(pow(0.001 * absolute_threshold, num_morph))) {
        print_nbest = 1;
      }
    }
  
    // 출력부 //////////////////////////////

    // 고대 스타일
    if (output_style) {
      // 다수 출력
      if (print_nbest) {
        // 각 분석 결과에 대해

        fprintf(fp, "@\t%s", ejs[i].c_str()); // 원어절
        for (int j = 0; j < num_result; j++) {
          if (!j) fprintf(fp, "\t%s\n", morph_analyzed_result[i][j].second.c_str()); // 분석 결과
          else fprintf(fp, "@\t%s\n", morph_analyzed_result[i][j].second.c_str()); // 분석 결과
        }
      }
      else { // 하나만 출력
        fprintf(fp, "%s", ejs[i].c_str()); // 원어절
        fprintf(fp, "\t%s\n", anal_result.c_str()); // 분석 결과
      }
      print_nbest = 0;
    }

    // 세종계획 스타일
    else {
      int morph_num = 0; /* 어절 내의 형태소 수 */
      char morphs[MAX_WORD][MAX_WORD]; /* 형태소 열 */
      char tags[MAX_WORD][MAX_WORD]; /* 품사 태그 열 */
      int spacing_tags[MAX_WORD]; /* 띄어쓰기 태그 열 */

      // 다수 출력
      if (print_nbest) {
        // 각 분석 결과에 대해
        for (int j = 0; j < num_result; j++) {

          if (!j) fprintf(fp, "@\t%s\t", ejs[i].c_str()); // 원어절

          // 탭
          else {
            fprintf(fp, "@\t");
          }

          // 분석결과
          get_morphs_tags((char *)morph_analyzed_result[i][j].second.c_str(), &morph_num,
                      morphs, tags, spacing_tags, delimiter);

          // 형태소마다
          for (int k = 0; k < morph_num; k++) {
            if (k) fprintf(fp, " + %s%c%s", morphs[k], delimiter, tags[k]); 
            else fprintf(fp, "%s%c%s", morphs[k], delimiter, tags[k]);
          }
          fprintf(fp, "\n");

        }
      }
      else { // 하나만 출력
        fprintf(fp, "%s\t", ejs[i].c_str()); // 원어절

        // 분석결과 (첫번째)
        get_morphs_tags((char *)morph_analyzed_result[i][0].second.c_str(), &morph_num,
                    morphs, tags, spacing_tags, delimiter);

        // 형태소마다
        for (int k = 0; k < morph_num; k++) {
          if (k) fprintf(fp, " + %s%c%s", morphs[k], delimiter, tags[k]); 
          else fprintf(fp, "%s%c%s", morphs[k], delimiter, tags[k]);
        }
        fprintf(fp, "\n");
      }
      print_nbest = 0;
    }

  } // end of for
}

/*****************************************************************************/
/* 태깅 결과 출력 */
/* ejs : 문장내 어절 */
/* morph_analyzed_result : (모든) 형태소 분석 결과 */
/* num_word : 어절의 수 */
/* tagging_result : (중의성 해소된) 태깅 결과 (형태소 분석 결과의 번호) */
// 리턴값 : 수작업 해야 할 어절 수
int print_nbest_tagging_result2(FILE *fp, vector<string> ejs, 
                          vector<ANALYZED_RESULT> &morph_analyzed_result, 
                          int num_word, int *tagging_result,
                          char delimiter, int output_style, int constraint,
                          int relative_threshold, int absolute_threshold) {
                            
  int num_result;
  int print_nbest = 0;
  double relative_ratio;
  int num_morph;

  int to_be_revised = 0;

  // 각 어절에 대해
  for (int i = 1; i <= num_word; i++) {
    
    num_result = (int) morph_analyzed_result[i].size(); // 어절의 형태소 분석 결과의 수
    
    string anal_result = morph_analyzed_result[i][0].second; // 첫 번째 형태소 분석 결과
    
    //**/fprintf(fp, "%s num_result = %d, tagging_result = %d\n", ejs[i].c_str(), num_result, tagging_result[i]);


    // 분석 결과가 오직 하나이고, 미분석이 아니라도 n-best 출력이 필요한 경우도 있음 (절대임계값에 의해)

    // 형태소 분석과 태거의 일치 제약
    // 제약을 만족하지 못하면
    // 태깅 결과와 형태소 분석 결과가 같지 않거나, 미분석이면
    if (tagging_result[i] != 0 || strstr(anal_result.c_str(), "??") != NULL) {
      print_nbest = 1;
    }
    
    // 형태소 분석 확률 상대 임계값 제약
    if (!print_nbest && (constraint & RELATIVE_MORPH_CONSTRAINT)) {
      
      if (num_result > 1) {

        relative_ratio = log(morph_analyzed_result[i][0].first) - log(morph_analyzed_result[i][1].first); // 두 확률의 차이
        
        // 임계값보다 크지 않으면 여러 분석 결과를 출력
        // 임계값이 크면 확정 단어율이 낮아짐 (확정 정확도는 높아짐)
        // 임계값이 작으면... 약간의 확률 차이만 나도 하나의 결과만 출력하므로 확정 단어율이 높아짐 (대신 확정 정확도는 낮아지겠지)
        if (relative_ratio < relative_threshold) { 
          print_nbest = 1;
        }
      }
    }
    
    // 형태소 분석 확률 절대 임계값 제약
    if (!print_nbest && (constraint & ABSOLUTE_MORPH_CONSTRAINT)) {
      num_morph = count_strchr(anal_result.c_str(), '+');
      num_morph++;
      
      if (log(morph_analyzed_result[i][0].first) < log(pow(0.001 * absolute_threshold, num_morph))) {
        print_nbest = 1;
      }
    }
  
    // 출력부 //////////////////////////////

    // 고대 스타일
    if (output_style) {
      // 다수 출력
      if (print_nbest) {
        // 각 분석 결과에 대해

        to_be_revised++;

        fprintf(fp, "%s", ejs[i].c_str()); // 원어절
        for (int j = 0; j < num_result; j++) {
          if (!j) fprintf(fp, "\t@@ %s\n", morph_analyzed_result[i][j].second.c_str()); // 분석 결과
          else fprintf(fp, "\t@ %s\n", morph_analyzed_result[i][j].second.c_str()); // 분석 결과
        }
      }
      else { // 하나만 출력
        fprintf(fp, "%s", ejs[i].c_str()); // 원어절
        fprintf(fp, "\t%s\n", anal_result.c_str()); // 분석 결과
      }
      print_nbest = 0;
    }

    // 세종계획 스타일
    else {
      int morph_num = 0; /* 어절 내의 형태소 수 */
      char morphs[MAX_WORD][MAX_WORD]; /* 형태소 열 */
      char tags[MAX_WORD][MAX_WORD]; /* 품사 태그 열 */
      int spacing_tags[MAX_WORD]; /* 띄어쓰기 태그 열 */

      // 다수 출력
      if (print_nbest) {

        to_be_revised++;

        // 각 분석 결과에 대해
        for (int j = 0; j < num_result; j++) {

          if (!j) fprintf(fp, "%s\t@@ ", ejs[i].c_str()); // 원어절

          // 탭
          else {
            fprintf(fp, "\t@ ");
          }

          // 분석결과
          get_morphs_tags((char *)morph_analyzed_result[i][j].second.c_str(), &morph_num,
                      morphs, tags, spacing_tags, delimiter);

          // 형태소마다
          for (int k = 0; k < morph_num; k++) {
            if (k) fprintf(fp, " + %s%c%s", morphs[k], delimiter, tags[k]); 
            else fprintf(fp, "%s%c%s", morphs[k], delimiter, tags[k]);
          }
          fprintf(fp, "\n");

        }
      }
      else { // 하나만 출력
        fprintf(fp, "%s\t", ejs[i].c_str()); // 원어절

        // 분석결과 (첫번째)
        get_morphs_tags((char *)morph_analyzed_result[i][0].second.c_str(), &morph_num,
                    morphs, tags, spacing_tags, delimiter);

        // 형태소마다
        for (int k = 0; k < morph_num; k++) {
          if (k) fprintf(fp, " + %s%c%s", morphs[k], delimiter, tags[k]); 
          else fprintf(fp, "%s%c%s", morphs[k], delimiter, tags[k]);
        }
        fprintf(fp, "\n");
      }
      print_nbest = 0;
    }

  } // end of for

  return to_be_revised;
}
