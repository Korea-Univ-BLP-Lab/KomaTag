#ifndef TRIANGULAR_MATRIX_H
#define TRIANGULAR_MATRIX_H

#include "hsplit.h"
#include "definitions.h" //SUB_STRING
/* �ﰢ����� ���Ҹ� �Է¹޾� �������迭�� �����Ǵ� ��ġ�� ���� */
/* see also TabPos1to2 */
#define TabPos2to1(x, y, n) ((x)*(2*n-(x)-1)/2+(y)-1) /* dglee */

/* ���ڿ��� ũ�⸦ �Է¹޾� �ﰢ����� ������ ���� */
#define TabNum(x) ((x)*((x)+1)/2) /* added by dglee */

/****************************************************************************/
typedef struct { /* �ﰢǥ ��ġ ����ü */
    int x;
    int y;
} t_TAB;

extern void setpos(t_TAB *pos, int x, int y);
extern short is_empty(t_TAB pos);
extern char *TabPos2String(char *src, char *dst, t_TAB *pos);
extern short TabPos1to2(int x, int n, t_TAB *pos);
extern int save_partial_str(int len, char *source_str, SUB_STRING &target_tabular);

#endif
