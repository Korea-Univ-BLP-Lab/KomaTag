/* ǰ������� ���õ� ��� */

#include "FST.h"

typedef struct {
  FILE* fileptr;	    /* information file pointer */
  short infolen;	    /* length of information field */
  unsigned long *finfo;  /* information�� ����Ǵ� ���� ����Ŵ */
} FST_INFO;


extern short FST_GetInfo (FST_INFO *fst_info, long index, unsigned long *info);
extern short FST_GetPos (void *fst, FST_INFO *fst_info, char *key, unsigned long *info);

extern FST_INFO *Open_Info(char* infofile, short infolength); /* FST file�� open */
extern void Close_Info(FST_INFO *fst_info);
