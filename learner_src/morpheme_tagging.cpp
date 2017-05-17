#include <stdio.h>
#include <string.h>
#include "hsplit.h"
#include "definitions.h"
#include "tool_pos_tagged_corpus.h"

/****************************************************************************/
/* 어절을 분석하는 함수  */
int ej_analysis_for_morpheme_tagging(char *str, FILE *outfp, char delimiter) {

  int i, j;
  int num_char = 0;

  char raw_ej[MAX_WORD]; /* 표층 어절 (원시) */  
  int morph_num = 0; /* 어절 내의 형태소 수 */
  char morphs[MAX_WORD][MAX_WORD]; /* 형태소 열 */
  char tags[MAX_WORD][MAX_WORD]; /* 품사 태그 열 */
  int spacing_tags[MAX_WORD]; /* 띄어쓰기 태그 열 */
  
  char morphs_syllable_unit[MAX_WORD][3]; /* 음절 단위의 형태소 */

  /*****************************************************************************/
  /* 어절 내의 형태소와 품사 태그를 알아낸다. */
  /* 입력 (str) : 품사 부착 말뭉치 형태 */
  if (!get_morphs_tags(str, raw_ej, &morph_num, morphs, tags, spacing_tags, delimiter)) return 1;

  ///**/fprintf(stderr, "morph_num = %d\n", morph_num);

  /* 결과 출력 */
  /* 어휘층 어절 구하기 */
  for (i = 0; i < morph_num; i++) { /* 각 형태소에 대해 */

    num_char = split_by_char(morphs[i], morphs_syllable_unit); /* 문자 단위로 쪼갠다 */
    
    for (j = 0; j < num_char; j++) { /* 한 음절씩 출력한다. */
      fprintf(outfp, "%s", morphs_syllable_unit[j]);
      ///**/fprintf(stderr, "%s", morphs_syllable_unit[j]);
    }
    fprintf(outfp, "\t%s\n", tags[i]); /* 형태소 태그 */
    ///**/fprintf(stderr, "\t%s\n", tags[i]); /* 형태소 태그 */
  }
  
  fprintf(outfp, "\n"); /* 어절의 끝일때 */
  ///**/fprintf(stderr, "\n"); /* 어절의 끝일때 */

  return 1;
} 

/*****************************************************************************/
/* infile : 입력 파일명 (품사 부착 말뭉치) */
/* outfile : 출력 파일명 */
/* 출력 파일 형식 :
  약속    nc

  장소    nc
  이      co
  ㄴ      etm
*/

int morpheme_tagging(char *infile, char *outfile, char delimiter) {

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
    if (!ej_analysis_for_morpheme_tagging(InputString, outfp, delimiter)) {
      fprintf(stderr, "Error: 올바르지 않은 문장 [%d] : %s\n", line_count, InputString);
    }
  }
  
  fclose(infp);
  fclose(outfp);

  return 1;
}
