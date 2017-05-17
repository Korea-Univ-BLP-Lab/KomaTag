/* 품사 태거 (tagger)의 learner를 수정함 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "entry2fst.h"

#define RMEJ_LIST "RMEJ.list" /* 기분석 어절 엔트리 -> RMEJ.FST */
#define RMEJ_INFO "RMEJ.info" /* 기분석 어절의 분석 결과 */
#define RMEJ_FREQ "RMEJ.freq" /* 빈도 (binary) */
#define RMEJ_FST  "RMEJ.fst"
#define RMEJ_HASH "RMEJ.hsh"

/*****************************************************************************/
/* freq_boundary : 빈도 임계값 */
/* input_filename : 입력 파일명 (품사 부착 말뭉치) */
int prokoma_learn_e (int freq_boundary, char *input_filename) {

  /* 빈도 */
  if (freq_boundary <= 0) {      /* 음이면 */
    return 0;
  }

  if (!entry2fst(input_filename, RMEJ_LIST, 
                 RMEJ_FST, RMEJ_HASH, RMEJ_INFO, RMEJ_FREQ, freq_boundary, 2)) 
    return 0;

  return 1;
}

