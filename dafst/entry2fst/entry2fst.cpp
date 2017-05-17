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

typedef map<string, int>             TAG_FREQ; /* �±� �� */
typedef map<string, TAG_FREQ>        WORD_TAG_FREQ; /* �ܾ� �±� �� */


/* ���� ��) entry2fst dic.txt 5 */

/*****************************************************************************/
int store_entry(WORD_TAG_FREQ &store, char *str) {
  char *ptr1 = str;
  char *ptr2;

  // �����ڸ� ã�´�.
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

  // ����
  store[ptr1][ptr2]++;

  return 1;
}


/*****************************************************************************/
int entry2fst(char *filename, char *list_filename, char *fst_filename, char *hash_filename,
              char *info_filename, char *freq_filename, 
              int cutoff_threshold, int cutoff_threshold2) {

  char **EntryTable = NULL;    /* �Է� ���ε��� ������ */
  char *Entry = NULL;
  int linenum;

  WORD_TAG_FREQ store;

  /* ȭ���� ���κ��� �Է� */
  if ( (linenum = file2lines(filename, &EntryTable, &Entry)) == 0) return 0;

  for (int j = 0; j < linenum; j++) { /* ���κ��� */
    if (!store_entry(store, EntryTable[j])) return 0; /* ��Ʈ���� ���� */
  }

  FILE *list_fp, *info_fp, *freq_fp;

  /* ȭ�� ���� */
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

  /* ����� �����κ��� �� ���Ͽ� ��� */
  for (itr1 = store.begin(); itr1 != store.end(); ++itr1) {
    int sum = 0;

    for (itr2 = itr1->second.begin(); itr2 != itr1->second.end(); ++itr2) {
      sum += itr2->second;
    }

    /* ���� ���� threshold�� �Ѵ� ��츸 ���� */
    if (sum >= cutoff_threshold) {

      for (itr2 = itr1->second.begin(); itr2 != itr1->second.end(); ++itr2) {

        // ���� �󵵰� threshold�� �Ѵ� ��츸 ����
        if (itr2->second >= cutoff_threshold2) {
     
          fprintf(list_fp, "%s\n", itr1->first.c_str());
          fprintf(info_fp, "%s\n", itr2->first.c_str());
          //fprintf(freq_fp, "%d\n", itr2->second); /* freq */
          fwrite(&itr2->second, sizeof(int), 1, freq_fp);
        }
      }
    }
  }

  /* ���� �ݱ� */
  fclose(list_fp);
  fclose(info_fp);
  fclose(freq_fp);

  /* fst ����� */
  build_fst(list_filename, fst_filename, hash_filename);

  /* �޸� ���� */
  if (Entry) free(Entry); 
  if (EntryTable) free(EntryTable);

  return 1;
}

