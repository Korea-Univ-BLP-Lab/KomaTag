#pragma warning(disable: 4786)
#pragma warning(disable: 4503)

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>

#include "FST.h"
#include "fileio.h"
#include "entry2fst.h"

#define DELIM '\t'

using namespace std;

typedef map<string, int>             TAG_FREQ; /* 태그 빈도 */
typedef map<string, TAG_FREQ>        WORD_TAG_FREQ; /* 단어 태그 빈도 */


/* 사용법 예) entry2fst dic.txt 5 */

/*****************************************************************************/
int store_entry(WORD_TAG_FREQ &store, char *str) {
  char *ptr1 = str;
  char *ptr2;

  // 구분자를 찾는다.
  if ((ptr2 = strchr(str, DELIM)) == NULL) {
    fprintf(stderr, "Error: there is no TAB in line [%s]\n", str);
    return 0;
  }
  *ptr2 = 0;
  ptr2++;

  if (strlen(ptr2) == 0) {
    fprintf(stderr, "Error: there is no category in line [%s]\n", str);
    return 0;
  }

  // 저장
  store[ptr1][ptr2]++;

  return 1;
}


/*****************************************************************************/
int entry2fst(char *filename, char *list_filename, char *fst_filename, char *hash_filename,
              char *info_filename, char *freq_filename, 
              int cutoff_threshold, int cutoff_threshold2) {

  char **EntryTable = NULL;    /* 입력 라인들의 포인터 */
  char *Entry = NULL;
  int linenum;

  WORD_TAG_FREQ store;

  /* 화일을 라인별로 입력 */
  if ( (linenum = file2lines(filename, &EntryTable, &Entry)) == 0) return 0;

  for (int j = 0; j < linenum; j++) { /* 라인별로 */
    if (!store_entry(store, EntryTable[j])) return 0; /* 엔트리를 저장 */
  }

  FILE *list_fp, *info_fp, *freq_fp;

  /* 화일 열기 */
  if ((list_fp = fopen(list_filename, "wt")) == NULL) {
    fprintf(stderr, "Error: cannot open file [%s]\n", list_filename);
    return 0;
  }

  if ((info_fp = fopen(info_filename, "wt")) == NULL) {
    fprintf(stderr, "Error: cannot open file [%s]\n", info_filename);
    return 0;
  }

  if ((freq_fp = fopen(freq_filename, "wb")) == NULL) {
    fprintf(stderr, "Error: cannot open file [%s]\n", freq_filename);
    return 0;
  }

  WORD_TAG_FREQ::iterator itr1;
  TAG_FREQ::iterator itr2;

  /* 저장된 정보로부터 각 파일에 출력 */
  for (itr1 = store.begin(); itr1 != store.end(); ++itr1) {
    int sum = 0;

    for (itr2 = itr1->second.begin(); itr2 != itr1->second.end(); ++itr2) {
      sum += itr2->second;
    }

    /* 빈도의 합이 threshold를 넘는 경우만 저장 */
    if (sum >= cutoff_threshold) {

      for (itr2 = itr1->second.begin(); itr2 != itr1->second.end(); ++itr2) {

        // 개별 빈도가 threshold를 넘는 경우만 저장
        if (itr2->second >= cutoff_threshold2) {
     
          fprintf(list_fp, "%s\n", itr1->first.c_str());
          fprintf(info_fp, "%s\n", itr2->first.c_str());
          //fprintf(freq_fp, "%d\n", itr2->second); /* freq */
          fwrite(&itr2->second, sizeof(int), 1, freq_fp);
        }
      }
    }
  }

  /* 파일 닫기 */
  fclose(list_fp);
  fclose(info_fp);
  fclose(freq_fp);

  /* fst 만들기 */
  build_fst(list_filename, fst_filename, hash_filename);

  /* 메모리 해제 */
  if (Entry) free(Entry); 
  if (EntryTable) free(EntryTable);

  return 1;
}

