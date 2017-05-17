#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "FST.h"
#include "env.h"
#include "common.h"
#include "definitions.h"
#include "entry2fst.h"

/*****************************************************************************/
void prokoma_e_close(void *rmej_fst, char **rmej_info, int *rmej_freq) {

  fst_close(rmej_fst, rmej_info, rmej_freq);

}

/*****************************************************************************/
int prokoma_e_open(char *RMEJ_FST_Path, char *RMEJ_hash_Path, char *RMEJ_FST_INFO_Path, char *RMEJ_FST_FREQ_Path, 
                   void **rmej_fst, char ***rmej_info, int **rmej_freq) {

  if (!fst_open(RMEJ_FST_Path, RMEJ_hash_Path, RMEJ_FST_INFO_Path, RMEJ_FST_FREQ_Path, 
                rmej_fst, rmej_info, rmej_freq)) {

    fprintf(stderr, "Error: FST_open\n");
    return 0;
  }

  return 1;
}

/*****************************************************************************/
/* 어절 단위 형태소 분석 */
/* return value: 분석 결과의 수 */
int prokoma_e(void *fst, char **rmej_info, int *rmej_freq, 
              const char *ej, ANALYZED_RESULT &result) {

  int result_num = 0;
  int i, n;
  int Index;
  int results[100];

  ANALYZED_RESULT_MAP result_map;

  if ((n = String2Hash (fst, (char *) ej, &Index)) == (-1)) { /* 리스트에 없으면 */
    return 0;
  }
  else {
    for (i = 0; i < Index; i++) { /* 복수개의 분석 결과가 있을 경우 */
      results[result_num++] = n++;
    }
    
  }

  int total_freq = 0;
  
  /* 빈도의 합 (확률값의 분모) */
  for (i = 0; i < result_num; i++) {
    total_freq += rmej_freq[results[i]];
  }
  
  double prob;

  /* 분석 결과의 수만큼 */
  for (i = 0; i < result_num; i++) {
    
    prob = (double)rmej_freq[results[i]] / total_freq; /* 확률 */

    /* 결과 저장 (형태소/품사 열 + 확률) */
    result_map.insert(make_pair(prob, (char *) rmej_info[results[i]]));
  }

  for (ANALYZED_RESULT_MAP::iterator itr = result_map.begin(); itr != result_map.end(); ++itr) {
    
    /* 확률 + 분석결과 */
    result.push_back(make_pair(itr->first, itr->second));
  }

  return result_num;
}

/******************************************************************************/
/* 결과 출력 */
/* 리턴값 : 분석 결과의 수 */
int print_result_e(FILE *fp, ANALYZED_RESULT_MAP &result) {

  ANALYZED_RESULT_MAP::iterator res_itr;

  /* 반복 */
  for (res_itr = result.begin(); res_itr != result.end(); ++res_itr) {

    /* 분석 결과, 확률 */
    fprintf(fp, "\t%s\t%12.11e\n", (char *) res_itr->second.c_str(), res_itr->first);
  }

  return (int) result.size(); /* 분석된 결과의 수 */
}
