/* Filename     : env.cpp */
/* Discription  : Checking Environments for KomaTag */
/* Programmer   : Do-Gil Lee (Dept. of Computer Science & Engineering, Korea University) */

/****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/* rsc 화일명 ***************************************************************/
// env.h에 정의된 상수와 연결됨
char *rsc_file_names[]={
  "RMEJ.fst",         // 0
  "RMEJ.hsh",         // 1
  "RMEJ.info",        // 2
  "RMEJ.freq",        // 3
  "PHONETIC.prb",     // 4
  "PHONETIC.info",    // 5
  "LEXICAL.prb",      // 6
  "TRANSITION.prb",   // 7
  "TAG_S.fst",        // 8
  "TAG_S.hsh",        // 9
  "TAG_S.prob",       // 10
  "SYLLABLE_S.fst",   // 11
  "SYLLABLE_S.hsh",   // 12
  "SYLLABLE_S.prob",  // 13
  "syllable.dic",     // 14
  "sb.fst",           // 15
  "INTER_TRANS.prb",  // 16
  "INTRA_TRANS.prb",  // 17
  "fullmorpheme.def", // 18
  "head",             // 19
  "dummy_head",       // 20
  "tail",             // 21
  "INTER_TRANS_EJ.prb", // 22
  "S_TRANSITION.FST", // 23
  NULL,
};

/* 화일명을 포함한 경로 */
char rsc_file_with_full_path[40][100];

/****************************************************************************/
/* 화일들이 제대로 있는지 검사 */
/* 리턴값 : 성공 = 1, 실패 = 0 */
static int check_file(char *filename) {
  FILE *fp;
  
  if (!(fp = fopen(filename, "rb"))) {
    fprintf (stderr, "\n[ERROR] : Cannot open file (%s)\n", filename);
    return 0;
  }
  fclose (fp);
  return 1;
}

/****************************************************************************/
/* check environments for ProKoma */
short komatag_CheckEnv (char *RSC_Path) {

  for (int i = 0; rsc_file_names[i]; i++) { 
    if (RSC_Path[0]) {
      
      int len = (int) strlen(RSC_Path);

      #ifdef WIN32 // 윈도우즈
      if (RSC_Path[len-1] != '\\') {
        RSC_Path[len] = '\\';
        RSC_Path[len+1] = 0;
      }
      #else // 유닉스나 리눅스
      if (RSC_Path[len-1] != '/') {
        RSC_Path[len] = '/';
        RSC_Path[len+1] = 0;
      }
      #endif

      sprintf(rsc_file_with_full_path[i], "%s%s", RSC_Path, rsc_file_names[i]);
    }
    else {
      sprintf(rsc_file_with_full_path[i], "%s", rsc_file_names[i]);
    }

    // 화일들이 제대로 있는지 검사
    //if (!check_file(rsc_file_with_full_path[i])) return 0;
  }

  return 1;
}
