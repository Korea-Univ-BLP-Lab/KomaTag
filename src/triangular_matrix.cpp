#include <string.h>
#include <stdio.h>
#include "definitions.h" // SUB_STRING
#include "triangular_matrix.h"

/****************************************************************************/
void setpos(t_TAB *pos, int x, int y) {
  pos->x = x;
  pos->y = y;
}

/****************************************************************************/
/* pos위치에 문자열이 없을 때 */
short is_empty(t_TAB pos) {

  if (pos.x == pos.y) return 1;
  else return 0;
}

/****************************************************************************/
/* 삼각행렬의 위치 pos를 받아 문자열 str의 해당 sub 문자열을 dst에 저장 */
char *TabPos2String(char *src, char *dst, t_TAB *pos) {
  char *s1;

  s1 = src + (pos->x)*2;

  strncpy(dst, s1, ((pos->y) - (pos->x)) * 2);
  dst[((pos->y) - (pos->x)) * 2] = 0;

  return dst;
}

/****************************************************************************/
/* tabular의 셀 위치(일차원 배열)를 입력받아 (x), 삼각행렬의 위치(i, j)를 넘겨준다.
/* n은 문자열의 길이 (음절의 수) = Len */
/* see also TabPos2to1 */
short TabPos1to2(int x, int n, t_TAB *pos) {
  int k;

  pos->x = -1;
  for (k = 0; k < n; k++) {
    if ( x < n+(2*n-k-1)*k/2 ) {
      pos->x = k;
      break;
    }
  }

  if ((pos->x) == -1) {
    fprintf(stderr, "Error: TabPos2Pos() : out of range!\n");
    return 0;
  }
        
  pos->y = x - (n + (2*n-(pos->x)) * ((pos->x)-1) / 2) + (pos->x) + 1;

  return 1;
}

/******************************************************************************/
/* tabular 방식으로 주어진 문자열(source_str)의 부분 문자열 저장 */
/* 예) 현재까지의 -> 현, 현재, 현재까, 현재까지, 현재까지의, 재, 재까, ..., 의 */
/*int save_partial_str(int len, char *source_str, char target_tabular[][MAX_WORD]) {
	int i, j, k;

  // 부분 문자열 저장
  for (k = i = 0; i < len; i++) {
    for (j = i; j < len; j++, k++) {
      strncpy(target_tabular[k], source_str+i*2, (j-i+1)*2);
      target_tabular[k][(j-i+1)*2] = 0;
    }
  }
  return 1;
}
*/
int save_partial_str(int len, char *source_str, SUB_STRING &target_tabular) {
	int i, j, k;

  char str[MAX_WORD];

  // 부분 문자열 저장
  for (k = i = 0; i < len; i++) {
    for (j = i; j < len; j++, k++) {
      strncpy(str, source_str+i*2, (j-i+1)*2);
      str[(j-i+1)*2] = 0;
      ///**/fprintf(stderr, "[%d] %s\n", k, str);
      target_tabular.push_back(str);
    }
  }
  return 1;
}
