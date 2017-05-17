/* ǰ�� �°� (tagger)�� learner�� ������ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "entry2fst.h"

#define RMEJ_LIST "RMEJ.list" /* ��м� ���� ��Ʈ�� -> RMEJ.FST */
#define RMEJ_INFO "RMEJ.info" /* ��м� ������ �м� ��� */
#define RMEJ_FREQ "RMEJ.freq" /* �� (binary) */
#define RMEJ_FST  "RMEJ.fst"
#define RMEJ_HASH "RMEJ.hsh"

/*****************************************************************************/
/* freq_boundary : �� �Ӱ谪 */
/* input_filename : �Է� ���ϸ� (ǰ�� ���� ����ġ) */
int prokoma_learn_e (int freq_boundary, char *input_filename) {

  /* �� */
  if (freq_boundary <= 0) {      /* ���̸� */
    return 0;
  }

  if (!entry2fst(input_filename, RMEJ_LIST, 
                 RMEJ_FST, RMEJ_HASH, RMEJ_INFO, RMEJ_FREQ, freq_boundary, 2)) 
    return 0;

  return 1;
}

