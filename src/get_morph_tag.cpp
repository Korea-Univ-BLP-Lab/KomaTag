#include <string.h>
#include "hsplit.h"
#include "definitions.h"
#include "get_morph_tag.h"

/*****************************************************************************/
/* 어절 내의 형태소와 품사 태그를 알아낸다. */
/* str : 품사 부착된 형식의 입력 어절 */
/* morph_num : 형태소 수 */
/* morphs : 형태소 열 */
/* tags : 품사태그 열 */
/* spacing_tags : 띄어쓰기 태그 열 */
int get_morphs_tags(char *str, int *morph_num, 
                    char morphs[][MAX_WORD], char tags[][MAX_WORD], int spacing_tags[MAX_WORD],
                    char delimiter) {

  char cur_char;
  int len; // 문자열의 길이
  int cur;

  char tag_sub[MAX_WORD];
  int tag_i = 0;
  
  char morph_sub[MAX_WORD];
  int morph_i = 0;

  len = (int) strlen(str); /* 문자열의 길이 */
//  if (len == 1) return 0; /* 줄에 아무 문자도 없는 경우('\n'만 있을 때) */

  *morph_num = 0;

  for (cur = 0; cur < len; cur++) {
    cur_char = str[cur]; /* 현재 글자 */
    morph_sub[morph_i++] = cur_char;

    if (cur_char == delimiter) { /* delimiter이면 */
      morph_sub[--morph_i] = (char) NULL;
      cur_char = str[++cur];
      
      if (cur_char == delimiter) { /* delimiter이면 */
        morph_sub[morph_i++] = cur_char;
        morph_sub[morph_i] = (char) NULL;
        cur_char = str[++cur];
      }
      
      /* 초기화 */
      tag_i = 0;
      morph_i = 0;
      
      while(cur_char != 0 && cur_char != '+' && cur_char != '\n' && cur_char != ' ' && cur_char != '\t' && cur_char != ';') {
        tag_sub[tag_i++] = cur_char;
        cur++;
        cur_char = str[cur];
      }
      tag_sub[tag_i] = (char) NULL;

      strcpy(tags[*morph_num], tag_sub); /* 품사 태그 */
      strcpy(morphs[*morph_num], morph_sub); /* 형태소 */
      spacing_tags[*morph_num] = 0; /* 띄어쓰기 태그 */ /* 일단 0을 만든다 */
      (*morph_num)++;
      
    } /* end of if */
  } /* end of for */
  
  spacing_tags[(*morph_num)-1] = 1; /* 마지막 형태소는 띄어쓰기 태그를 1로 만든다. */
  
  return 1;
}

