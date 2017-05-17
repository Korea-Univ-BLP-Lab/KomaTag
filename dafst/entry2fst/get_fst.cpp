#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FST.h"

/*****************************************************************************/
char **entry_table=NULL;    /* 입력 단어들의 포인터 */
char *entry_string=NULL;    /* 입력 단어의 문자열 */

static int make_entry_table(char *DicFile) {
  FILE *f;
  char *p;
  int i;
  long FileSize;
  int nEntry;

  if ((f=fopen(DicFile, "rb"))==NULL) {
    fprintf(stderr, "Cannot open entry file");
    return 0;
  }
  
  fseek(f, 0, SEEK_END);
  FileSize=ftell(f);
  fseek(f, 0, SEEK_SET);

  entry_string=(char *)malloc(FileSize+1);
  fread(entry_string, FileSize, 1, f);
  entry_string[FileSize]=0;
  fclose(f);

  nEntry=0;
  if (strtok((char *)entry_string, "\r\n") != NULL)

  do {
    nEntry++; 
  } while (strtok(NULL, "\r\n") != NULL);

  ///**/fprintf(stderr, "nEntry = %d\n", nEntry);
  entry_table=(char **)malloc(nEntry * sizeof(char *));

  p = entry_string;

  for (i = 0; i < nEntry; i++) {
    entry_table[i] = p;
    if (i<nEntry-1)
      for (p += strlen((char *)p)+1; (*p==0)||(*p=='\r')||(*p=='\n'); p++);
  } 
  
  return (nEntry);
}

/*****************************************************************************/
static int *LoadINFO (char *path) {
  FILE *infofp;
  long FileSize = 0;
  int *fst_info;

  if ((infofp = fopen (path, "rb")) == NULL) {
    fprintf (stderr, "ERROR: Can't open information file [%s]!\n", path);
    return NULL;
  }

  fseek (infofp, 0, SEEK_END);
  FileSize = ftell (infofp);
  fseek (infofp, 0, SEEK_SET);
  fst_info = (int *) malloc (FileSize + 1);

  if (!fst_info) {
    fprintf (stderr, "ERROR: Not enough memory!\n");
    return NULL;
  }
  fread (fst_info, sizeof (int), FileSize / sizeof (int), infofp);
  fclose (infofp);
  return fst_info;
}


/*****************************************************************************/
int fst_open(char *fst_Path, char *hash_Path, char *fst_INFO_Path, char *fst_FREQ_Path, 
                   void **fst_fst, char ***fst_info, int **fst_freq) {

  int num;
  
  /* fst 열기 */
  fprintf(stderr, "\tReading FST file.. [%s]", fst_Path);
  if ((*fst_fst = LoadTransducer (fst_Path, hash_Path)) == NULL) {
    fprintf (stderr, "[ERROR] Cannot open fst! [%s][%s]\n", fst_Path, hash_Path);
    return 0;
  }
  fprintf(stderr, "\t[done]\n");
  
  /* frequency 화일 열기 */
  fprintf(stderr, "\tReading Frequency file.. [%s]", fst_FREQ_Path);
  if ((*fst_freq = LoadINFO (fst_FREQ_Path)) == NULL) {
    fprintf (stderr, "[ERROR] Cannot open fst! [%s]\n", fst_FREQ_Path);
    return 0;
  }
  fprintf(stderr, "\t[done]\n");

  /* info 화일 열기 */
  fprintf(stderr, "\tReading Infomation file.. [%s]", fst_INFO_Path);
  num = make_entry_table(fst_INFO_Path);
  
  *fst_info = entry_table;
  fprintf(stderr, "\t[done]\n");

  return 1;
}

/*****************************************************************************/
/* 이 함수의 호출은 : fst_close(fst_fst, fst_info, fst_freq); 와 같이 한다. */
void fst_close(void *fst_fst, char **fst_info, int *fst_freq) {

  if (fst_fst) FreeTransducer(fst_fst);

  /* 메모리 해제 */
  if (fst_freq) free (fst_freq);
  fst_freq = NULL;

  if (entry_string) free(entry_string);
  entry_string = NULL;

  if (entry_table) free(entry_table);
  entry_table = NULL;
}
