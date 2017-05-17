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
/* ���� ���� ���¼� �м� */
/* return value: �м� ����� �� */
int prokoma_e(void *fst, char **rmej_info, int *rmej_freq, 
              const char *ej, ANALYZED_RESULT &result) {

  int result_num = 0;
  int i, n;
  int Index;
  int results[100];

  ANALYZED_RESULT_MAP result_map;

  if ((n = String2Hash (fst, (char *) ej, &Index)) == (-1)) { /* ����Ʈ�� ������ */
    return 0;
  }
  else {
    for (i = 0; i < Index; i++) { /* �������� �м� ����� ���� ��� */
      results[result_num++] = n++;
    }
    
  }

  int total_freq = 0;
  
  /* ���� �� (Ȯ������ �и�) */
  for (i = 0; i < result_num; i++) {
    total_freq += rmej_freq[results[i]];
  }
  
  double prob;

  /* �м� ����� ����ŭ */
  for (i = 0; i < result_num; i++) {
    
    prob = (double)rmej_freq[results[i]] / total_freq; /* Ȯ�� */

    /* ��� ���� (���¼�/ǰ�� �� + Ȯ��) */
    result_map.insert(make_pair(prob, (char *) rmej_info[results[i]]));
  }

  for (ANALYZED_RESULT_MAP::iterator itr = result_map.begin(); itr != result_map.end(); ++itr) {
    
    /* Ȯ�� + �м���� */
    result.push_back(make_pair(itr->first, itr->second));
  }

  return result_num;
}

/******************************************************************************/
/* ��� ��� */
/* ���ϰ� : �м� ����� �� */
int print_result_e(FILE *fp, ANALYZED_RESULT_MAP &result) {

  ANALYZED_RESULT_MAP::iterator res_itr;

  /* �ݺ� */
  for (res_itr = result.begin(); res_itr != result.end(); ++res_itr) {

    /* �м� ���, Ȯ�� */
    fprintf(fp, "\t%s\t%12.11e\n", (char *) res_itr->second.c_str(), res_itr->first);
  }

  return (int) result.size(); /* �м��� ����� �� */
}
