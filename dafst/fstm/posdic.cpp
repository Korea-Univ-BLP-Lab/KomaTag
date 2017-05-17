#include <stdio.h>
#include <stdlib.h>
#include "posdic.h"

/*****************************************************************************/
void Close_Info(FST_INFO *fst_info) {
  if (fst_info->fileptr) fclose(fst_info->fileptr);
  else fprintf(stderr, "info file 없음\n");
  
  if (fst_info->finfo) free(fst_info->finfo);
  if (fst_info) free(fst_info);
}

/*****************************************************************************/
/* 품사 사전의 information */
FST_INFO *Open_Info(char* infofile, short infolength) { /* FST file을 open */

  FST_INFO *HandleINFO;
  long FileSize;

  if (!(HandleINFO = (FST_INFO *) malloc(sizeof(FST_INFO)))) {
    fprintf(stderr, "\nFST(%s) Error! No Memory..!!\n", infofile);
    return NULL; /* memory allocation */
  }

  if (!(HandleINFO->fileptr = fopen (infofile, "rb"))) {
    if (!(HandleINFO->fileptr = fopen (infofile, "w+b"))) {
      fprintf(stderr, "Can't open information file [%s]!!\n", infofile);
      return NULL;
    }
  }
  
  HandleINFO->infolen = infolength;

  fseek(HandleINFO->fileptr, 0, SEEK_END);		/* SEEK_END */
  FileSize = ftell(HandleINFO->fileptr);
  rewind(HandleINFO->fileptr);

  /* memory allocation */
  if (!(HandleINFO->finfo = (long unsigned int*) malloc(FileSize))) {
    fprintf(stderr, "\nError! Not enough memory for information file..!!\n");
    return NULL; /* memory allocation */
  }

  /* information을 읽어와서 저장 */
  fread(HandleINFO->finfo, FileSize, 1, HandleINFO->fileptr);
	
  return HandleINFO;  
}

/*****************************************************************************/
/* Get pos of key. 
   If key does not exist, set information zero.
   If error, return ERROR.
   Otherwise, return SUCCESS. */
short FST_GetPos (void *fst, FST_INFO *fst_info, char *key, unsigned long *info) {
  int index;
  int i;
  int nItem;

#ifdef _FST_DEBUG_
  char kkey[XL_WORD];
#endif

  /* 색인어가 사전에 있는지 */
  index = String2Hash(fst, key, &nItem);

#ifdef _FST_DEBUG_
  tg2ks(key, kkey);
  fprintf(stdout, "key = %s, index = %d\n", kkey, index);
#endif

  if ( index != -1 ) { /* 발견된 경우 */
    if (!FST_GetInfo(fst_info, index, info)) return 0;
  } 
  else { /* 발견되지 않은 경우 모든 비트를 0으로 setting */
    for (i=0; i < fst_info->infolen; i++)
      info[i] = 0l;
  }
  return 1;
}

/*****************************************************************************/
/* INFO library */
/* 사전의 index 위치에 있는 품사 및 자질 정보를 info에 저장 */
short FST_GetInfo (FST_INFO *fst_info, long index, unsigned long *info) {
  /* start of 메모리에 저장하는 경우 (memory-based)*/
  /* FST.h의 finfo를 unsigned long형 이차원 배열로 고침 */
  /*info[0] = fst_info->finfo[index][0];
  info[1] = fst_info->finfo[index][1];
  info[2] = fst_info->finfo[index][2];
  */

  info[0] = fst_info->finfo[index*(fst_info->infolen)];
  info[1] = fst_info->finfo[index*(fst_info->infolen) + 1];
  info[2] = fst_info->finfo[index*(fst_info->infolen) + 2];
  /* end of 메모리에 저장하는 경우 (memory-based)*/
  
  return 1;
}
