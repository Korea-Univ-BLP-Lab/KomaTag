#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "entry2fst.h"

/*****************************************************************************/
/* Global variables */
char *Version = "("__DATE__")";
char *Description = "Converting an entry file to a FST file";

/*****************************************************************************/
void Help (char *RunName) {
  fprintf (stderr, "\n%s %s : %s", RunName, Version, Description);
  fprintf (stderr, "\n");
  fprintf (stderr, "\n[Usage]");
  fprintf (stderr, "\n%s FILE number", RunName);
  fprintf (stderr, "\n\n[Description]");
  fprintf (stderr, "\nFILE refers to an entry file consisting of (key and value) pairs, which should be delimited by TABs.");
  fprintf (stderr, "\nnumber refers to a cutoff threshold.");
  fprintf (stderr, "\nFILE.fst, FILE.list, FILE.info, and FILE.freq will be created as the output.");

  fprintf (stderr, "\n");
}

/*****************************************************************************/
int main(int argc, char *argv[]) {

  char fst_filename[100];
  char list_filename[100];
  char info_filename[100];
  char freq_filename[100];
  char hash_filename[100];

  if (argc != 3) {
    Help ("entry2fst");
    return 0;
  }

  /* 프로그램 정보 출력 */
  fprintf (stderr, "%s %s : %s\n", argv[0], Version, Description);      


  char ext_excluded_filename[100];
  
  { /* 확장자 제거 */
    char *p;
    strcpy(ext_excluded_filename, argv[1]);
    p = strrchr(ext_excluded_filename, '.');
    if (p != NULL) *p = 0;
  }

  sprintf(list_filename, "%s.list", ext_excluded_filename); /* list */
  sprintf(fst_filename,  "%s.fst",  ext_excluded_filename);   /* fst */
  sprintf(info_filename, "%s.info", ext_excluded_filename); /* info */
  sprintf(freq_filename, "%s.freq", ext_excluded_filename); /* freq */
  sprintf(hash_filename, "%s.hsh",  ext_excluded_filename); /* hsh */
  
  entry2fst(argv[1], list_filename, fst_filename, hash_filename, info_filename, freq_filename,
            atoi(argv[2]), 1);

  return 1;
}
