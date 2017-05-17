#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cmn98.h"
#include "postype.h"

/****************************************************************************/
/* ��� ���� */
#define ENV_FILE        "NE.ENV"
#define POS_NAME        "POS.NAM"
#define ENV_NAME        "NE2001"
#define RSC_NAME        "RSC"
#define RSC_VALUE       "../../rsc/ne2001/"

#define FST_FILE        "POSDIC.FST"
#define FST_INFO_FILE   "POSDIC.NFO"

/****************************************************************************/
/* environments for ne2001 */
char* PosnameFile=POS_NAME; /* ǰ���̸� ȭ�� */

/* ȯ�漳�� ȭ�� */
char* Env_Name = ENV_NAME;
char Env_Path[80] = ENV_FILE; /* ex) set NE=x:\bin\NE.ENV  --> Env_Path = "x:\bin\NE.ENV" */

/* ���ҽ� */
char* Rsc_Name = RSC_NAME;
char Rsc_Value[80] =  RSC_VALUE; /* �� ���� ���� ���� ��� (�ʱⰪ) */

/* FST ���� */
char* FSTFile = FST_FILE;
char* FSTinfoFile = FST_INFO_FILE;
char FSTPath[80];
char FSTinfoPath[80];

/* ǰ�� �̸� */
char Posname_Path[80];
char PosName[NUM_POS][40];


/****************************************************************************/
/* ���ڿ� src�κ��� option�� �ش��ϴ� value�� ���� */
/* ex) src = "HC=KS", option = "HC" --> return value = "KS" */
char* mygetenv (char *src, const char *option) {
  char **search = &src;
  unsigned int length;

  if (search && option) {

    length = strlen(option);

    while (*search) {
      if (strlen(*search) > length && (*(*search + length) == '=') 
	  && (strncmp(*search, option, length) == 0)) {
        return(*search + length + 1);
      }

      search++;
    }
  }

  return(NULL);
}

/****************************************************************************/
void KOMA_ReadPosName (void) {
  short i;
  FILE *fileptr;
  char name[40];

  /* ȭ�� ���� */
  if (!(fileptr = fopen (Posname_Path, "rt"))) {
    fprintf (stderr, "\nNE Error! I cannot load pos_name_file(%s)..!!\n", Posname_Path);
    exit(0);
  }

  /* ǰ�� �б� */
  i = 0;
  while (i < NUM_POS && fscanf (fileptr, "%s%*s", name) != EOF) {
    strcpy (PosName[i++], name);
  }

  /* ȭ�� �ݱ� */
  fclose (fileptr);
}
/****************************************************************************/
void KOMA_ViewPosName (void) {
  short i;
 
  KOMA_ReadPosName();
  
  /* ǰ�� ��� */
  for (i=0; i < NUM_POS; i++)
    fprintf (stdout, "\n%s \t:%03d", PosName[i], i+1);
  fprintf (stdout, "\n");

}

/****************************************************************************/
void KOMA_ViewEnv (void) {
  fprintf (stdout, "\n[Environment] ");
  fprintf (stdout, "\n   %s=%s", Env_Name, Env_Path);
  
  fprintf (stdout, "\n\nIn Environment file (%s),", Env_Path);
  fprintf (stdout, "\n   %s=%s", Rsc_Name, Rsc_Value);
  
  fprintf (stdout, "\n\nIn directory (%s),", Rsc_Value);
  fprintf (stdout, "\n   %s : POS name file", PosnameFile);
  fprintf (stdout, "\n   %s : FST-dictionary file", FSTFile);
  fprintf (stdout, "\n   %s : FST-dictionary information file", FSTinfoFile);
  
  fprintf (stdout, "\n\nPOS number: %d", NUM_POS);
  fprintf (stdout, "\nPOS length: %d", LEN_POS);
  fprintf (stdout, "\n\n");
}

/****************************************************************************/
short KOMA_SetEnvHelp (void) {
  fprintf (stdout, "\n[Environment] ");
  fprintf (stdout, "\nYou must set environment such as..");
  fprintf (stdout, "\n   set %s=<absolute path & file name>", Env_Name);
  
  fprintf (stdout, "\nFor example, ");
  fprintf (stdout, "\nIn case of Unix, ");
  fprintf (stdout, "\n   set %s=/home/bin/NE.ENV", Env_Name);

  fprintf (stdout, "\nIn case of Windows, ");
  fprintf (stdout, "\n   SET %s=C:\\Program Files\\NE2000\\NE.ENV", Env_Name);
  
  fprintf (stdout, "\n\n");
  return ERROR;
}

/****************************************************************************/
/* set environments for NE */
short KOMA_CheckEnv (void) {
  FILE *fileptr;
  FILE *envptr;
  char readstr[200];
  char *temp;

  /* ȯ�溯�� �о���� */
  if (temp = getenv(Env_Name)) {
    strcpy(Env_Path, temp);
  }

  if (!(envptr = fopen (Env_Path, "rt"))) { /* ȯ�漳��ȭ�� ���� */
    fprintf (stderr, "\nNE Error! I cannot load environment file (%s)\n", Env_Path);
    return KOMA_SetEnvHelp();
  }

  /* ���ҽ��� ��ġ */
  if (fscanf(envptr, "%s", readstr) == EOF) return KOMA_SetEnvHelp();
  if (!(temp = mygetenv(readstr, Rsc_Name))) return KOMA_SetEnvHelp();
  strcpy(Rsc_Value, temp);
  sprintf (Posname_Path, "%s%s", Rsc_Value, PosnameFile);
  sprintf (FSTPath, "%s%s", Rsc_Value, FSTFile);
  sprintf (FSTinfoPath, "%s%s", Rsc_Value, FSTinfoFile);
  
  fclose (envptr); /* ȯ�漳��ȭ�� �ݱ� */

  /* ȭ�ϵ��� ����� �ִ��� �˻� */
  /* load posname */
  KOMA_ReadPosName();

  /* load dictionary file */
  if (!(fileptr = fopen(FSTPath, "rb"))) {
    fprintf (stderr, "\nNE Error : Cannot Open Dictionary file (%s)\n", FSTPath);
    return ERROR;
  }
  fclose (fileptr);

  return SUCCESS;
}
