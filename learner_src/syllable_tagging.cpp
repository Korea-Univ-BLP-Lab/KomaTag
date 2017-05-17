#include <stdio.h>
#include <string.h>
#include "hsplit.h"
#include "tool_pos_tagged_corpus.h"

/* 출력 모드 */
#define BI    1 /* 시작, 중간 */
#define BIS   2 /* 시작, 중간, 단독 */
#define IE    3 /* 시작, 끝 */    
#define IES   4 /* 시작, 끝, 단독 */

#define SYLLABLE_TAGGING 1  /* 품사 부착 말뭉치 -> 음절 단위 태깅 */
#define ALIGNMENT        2  /* 품사 부착 말뭉치 -> 표층 어절과 어휘층 어절 정렬 (음운현상에 따라) */


/*****************************************************************************/
/* 어절을 분석하는 함수  */
int ej_analysis_for_syllable_tagging(char *str, FILE *outfp, int print_mode, char delimiter) {
  int i;
  
  int morph_num = 0; /* 어절 내의 형태소 수 */
  char morphs[MAX_WORD][MAX_WORD]; /* 형태소 */
  char tags[MAX_WORD][MAX_WORD]; /* 품사 태그 */

  int lexical_ej_len = 0; /* 어휘층 어절 길이 */
  char lexical_ej[MAX_WORD][3]; /* 어휘층 어절 */
  
  char raw_ej[MAX_WORD]; /* 표층 어절 (원시) */
  int surface_ej_len = 0; /* 표층 어절 길이 */
  char surface_ej[MAX_WORD][3]; /* 표층 어절 */
  
  char syllable_tag[MAX_WORD][30]; /* 표층 어절에 대한 음절 단위의 품사 태그 */
  int spacing_tags[MAX_WORD]; /* 띄어쓰기 태그 열 */

  /*****************************************************************************/
  /* 어절 내의 형태소와 품사 태그를 알아낸다. */
  if (!get_morphs_tags(str, raw_ej, &morph_num, morphs, tags, spacing_tags, delimiter)) return 1;
  /*****************************************************************************/

  /* 결과 출력 */
  
  /* 어휘층 어절 구하기 */
  {
    int num_char = 0;
    for (i = 0; i < morph_num; i++) { /* 각 형태소에 대해 */

      num_char = split_by_char(morphs[i], &lexical_ej[lexical_ej_len]); /* 문자 단위로 쪼갠다 */

      get_syllable_tagging_result(print_mode, &lexical_ej[lexical_ej_len], num_char, tags[i], &syllable_tag[lexical_ej_len]);
    
      lexical_ej_len += num_char;
    }
  }

  /*****************************************************************************/
  /* 표층 어절 */
  surface_ej_len = split_by_char(raw_ej, surface_ej); /* 문자 단위로 쪼갠다 */
    
  /* 어휘층 어절 */
  for (i = 0; i < lexical_ej_len; i++) {
    fprintf(outfp, "%s\t%s\n", lexical_ej[i], syllable_tag[i]);
  }
  
  fprintf(outfp, "\n"); /* 어절의 끝일때 */
    
  return 1;
} 

/*****************************************************************************/
/* infile : 품사 부착 말뭉치 */
/* outfile의 형식:
약      B-nc
속      I-nc

장      B-nc
소      I-nc
이      B-co
ㄴ      B-etm
*/
int syllable_tagging(char *infile, char *outfile, char delimiter) {
  int line_count = 0;
  char InputString[MAX_WORD];

  FILE *infp;
  FILE *outfp;

  if ((infp = fopen(infile, "rt")) == NULL) {
    fprintf(stderr, "File open error : %s\n", infile);
    return 0;
  }

  if ((outfp = fopen(outfile, "wt")) == NULL) {
    fprintf(stderr, "File open error : %s\n", outfile);
    return 0;
  }

  while (fgets(InputString, MAX_WORD, infp) != NULL) {
    ++line_count;

    /* 각 어절에 대해 처리 */
    if (!ej_analysis_for_syllable_tagging(InputString, outfp, BI, delimiter)) {
      fprintf(stderr, "Error: 올바르지 않은 문장 [%d] : %s\n", line_count, InputString);
    }
  }

  fclose(infp);
  fclose(outfp);


  return 1;
}

