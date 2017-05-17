#include <stdio.h>
#include <stdlib.h>
#include "posdic.h"

/*****************************************************************************/
void Close_Info(FST_INFO *fst_info) {
  if (fst_info->fileptr) fclose(fst_info->fileptr);
  else fprintf(stderr, "info file ����\n");
  
  if (fst_info->finfo) free(fst_info->finfo);
  if (fst_info) free(fst_info);
}

/*****************************************************************************/
/* ǰ�� ������ information */
FST_INFO *Open_Info(char* infofile, short infolength) { /* FST file�� open */

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

  /* information�� �о�ͼ� ���� */
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

  /* ���ξ ������ �ִ��� */
  index = String2Hash(fst, key, &nItem);

#ifdef _FST_DEBUG_
  tg2ks(key, kkey);
  fprintf(stdout, "key = %s, index = %d\n", kkey, index);
#endif

  if ( index != -1 ) { /* �߰ߵ� ��� */
    if (!FST_GetInfo(fst_info, index, info)) return 0;
  } 
  else { /* �߰ߵ��� ���� ��� ��� ��Ʈ�� 0���� setting */
    for (i=0; i < fst_info->infolen; i++)
      info[i] = 0l;
  }
  return 1;
}

/*****************************************************************************/
/* INFO library */
/* ������ index ��ġ�� �ִ� ǰ�� �� ���� ������ info�� ���� */
short FST_GetInfo (FST_INFO *fst_info, long index, unsigned long *info) {
  /* start of �޸𸮿� �����ϴ� ��� (memory-based)*/
  /* FST.h�� finfo�� unsigned long�� ������ �迭�� ��ħ */
  /*info[0] = fst_info->finfo[index][0];
  info[1] = fst_info->finfo[index][1];
  info[2] = fst_info->finfo[index][2];
  */

  info[0] = fst_info->finfo[index*(fst_info->infolen)];
  info[1] = fst_info->finfo[index*(fst_info->infolen) + 1];
  info[2] = fst_info->finfo[index*(fst_info->infolen) + 2];
  /* end of �޸𸮿� �����ϴ� ��� (memory-based)*/
  
  return 1;
}
