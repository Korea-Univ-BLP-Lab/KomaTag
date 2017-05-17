/* Filename : fstm.cpp */
/* Version  : 2005 */
/* Discription  : POS Dictionary (FST based) Manager */
/* Programmer : Do-gil Lee (Dept. of Computer Science, Korea University) */
/* Date   : 3/Mar/2005 */

char *RunName = "fstm2001";
char *Version = "()";
char *Description = "POS Dictionary (FST based) Manager";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmn98.h"
#include "env.h"
#include "FST.h"
#include "posdic.h"
#include "dafst.h"

#define MAX_KEY_LENGTH 1024

/* FST-based dictionary */
void *fst;
FST_INFO *fst_info;

void SetUnset (short argc, char *argv []);
void View(void);
void PutOwnPos(void);
/*void PutSamePos(void);*/
void GetAllKey(void);
void GetGivenKey(void);
void GetSamePos(void);
void GetOrPos(void);
void GetMaskPos(void);

/* UsageHelp */
void UsageHelp (void) {
  fprintf (stdout, "\n%s %s", RunName, Version);
  fprintf (stdout, "\n  %s", Description);
  fprintf (stdout, "\n[Usage]");
  fprintf (stdout, "\n: view posname list.");
  fprintf (stdout, "\n  %s -p  [> stdout]", RunName);

  fprintf (stdout, "\n: view environemt.");
  fprintf (stdout, "\n  %s -e  [> stdout]", RunName);

  /*fprintf (stdout, "\n: view and edit key and pos.");
    fprintf (stdout, "\n  %s !  [< stdin] [> stdout]", RunName);
  */

  fprintf (stdout, "\n: put key, with its own pos list.");
  fprintf (stdout, "\n  %s +  [< stdin]", RunName);

  /*    fprintf (stdout, "\n: put key, with the same pos list."); 
	fprintf (stdout, "\n  %s += <[+|-]POSNAME>... [< stdin]", RunName); */

  fprintf (stdout, "\n: get key and its pos, for all entry.");
  fprintf (stdout, "\n  %s -  [> stdout]", RunName);

  fprintf (stdout, "\n: get key and its pos, for given entry.");
  fprintf (stdout, "\n  %s -: [< stdin] [> stdout]", RunName);

  fprintf (stdout, "\n: get key and its pos, with the same as given pos list.");
  fprintf (stdout, "\n  %s -= <[+|-]POSNAME>... [> stdout]", RunName);

  fprintf (stdout, "\n: get key and its pos, if disjointed by given pos list.");
  fprintf (stdout, "\n  %s -^ <[+|-]POSNAME>... [> stdout]", RunName);

  fprintf (stdout, "\n: get key and masked pos, if masked by given pos list.");
  fprintf (stdout, "\n  %s -$ <[+|-]POSNAME>... [> stdout]", RunName);

  fprintf (stdout, "\n\n");
}

/*****************************************************************************/
/* main function. */
int main(short argc, char *argv []) {
  if (!KOMA_CheckEnv()) { perrmsg(); return SUCCESS; }
  if (argc < 2) { UsageHelp (); return SUCCESS; }
  if (!strcmp (argv [1], "-p") || !strcmp (argv [1], "-P")) {
    KOMA_ViewPosName ();
    return SUCCESS;
  }
  if (!strcmp (argv [1], "-e") || !strcmp (argv [1], "-E")) {
    KOMA_ViewEnv ();
    return SUCCESS;
  }
  
  /* 사전 열기 */
  {
    /* FST */
    if ((fst = LoadTransducer(FSTPath, NULL)) == NULL) {
      fprintf(stderr, "Can't open FST file [%s]\n", FSTPath); 
      return SUCCESS; 
    }
  
    /* information */
    if (! (fst_info = Open_Info(FSTinfoPath, LEN_POS))) { 
      fprintf(stderr, "Can't open information file [%s]\n", FSTinfoPath); 
      return SUCCESS; 
    }
  }

  if (argc==2) {
    /*if (!strcmp (argv[1], "!")) View ();
      else */
    if (!strcmp (argv[1], "+")) PutOwnPos ();
    else if (!strcmp (argv[1], "-")) GetAllKey ();
    else if (!strcmp (argv[1], "-:")) GetGivenKey ();
    else UsageHelp ();
  }
  else if (argc>2) {
    SetUnset (argc, argv);
    /*if (!strcmp (argv[1], "+=")) PutSamePos ();
      else */
    if (!strcmp (argv[1], "-=")) GetSamePos ();
    else if (!strcmp (argv[1], "-^")) GetOrPos ();
    else if (!strcmp (argv[1], "-$")) GetMaskPos ();
    else UsageHelp ();
  }
  else UsageHelp ();

  FreeTransducer(fst);
  if (fst_info) Close_Info(fst_info);
  return SUCCESS;
}

#define LEN_KEY 200

char tgkey [LEN_KEY*2];
char kskey [LEN_KEY*2];
unsigned long info [LEN_POS];
unsigned long setinfo [LEN_POS];
unsigned long unsetinfo [LEN_POS];

/*****************************************************************************/
/*void SimulateSambul (void) {
  tPH ph;
  ConvertToPhoneme (tgkey, &ph);
  if (ph.m != PHM_FIL || ph.f != PHF_FIL) return;
  switch (ph.i) {
  case PHI_N: 
    CopySyllable (tgkey, PHI_FIL, PHM_FIL, PHF_N);
    break;
  case PHI_R: 
    CopySyllable (tgkey, PHI_FIL, PHM_FIL, PHF_R);
    break;
  case PHI_M: 
    CopySyllable (tgkey, PHI_FIL, PHM_FIL, PHF_M);
    break;
  case PHI_B: 
    CopySyllable (tgkey, PHI_FIL, PHM_FIL, PHF_B);
    break;
  case PHI_NG: 
    CopySyllable (tgkey, PHI_FIL, PHM_FIL, PHF_NG);
    break;
  }
}
*/
/*****************************************************************************/
/*void ReverseSambul (void) {
  tPH ph;
  ConvertToPhoneme (tgkey, &ph);
  if (ph.i != PHI_FIL || ph.m != PHM_FIL) return;
  switch (ph.f) {
  case PHF_N: 
    CopySyllable (tgkey, PHI_N, PHM_FIL, PHF_FIL);
    break;
  case PHF_R: 
    CopySyllable (tgkey, PHI_R, PHM_FIL, PHF_FIL);
    break;
  case PHF_M: 
    CopySyllable (tgkey, PHI_M, PHM_FIL, PHF_FIL);
    break;
  case PHF_B: 
    CopySyllable (tgkey, PHI_B, PHM_FIL, PHF_FIL);
    break;
  case PHF_NG: 
    CopySyllable (tgkey, PHI_NG, PHM_FIL, PHF_FIL);
    break;
  }
}
*/
/*****************************************************************************/
short ScanKey (short sambul) {
  if (fscanf (stdin, "%s", kskey) == EOF) return (short)0;

  strcpy (tgkey, kskey);
  return (short)1;
}

/*****************************************************************************/
void PrintKey (short sambul) {
  strcpy (kskey, tgkey);
  fprintf (stdout, "%s ", kskey);
}

/*****************************************************************************/
short INFO_InfoCompSame (unsigned long *info, unsigned long *setinfo, unsigned long *unsetinfo);
short INFO_InfoCompOr (unsigned long *info, unsigned long *setinfo, unsigned long *unsetinfo);
void INFO_MaskInfo (unsigned long *info, unsigned long *setinfo, unsigned long *unsetinfo);
void INFO_SetInfo (unsigned long *info, unsigned long *setinfo, unsigned long *unsetinfo);
void INFO_SetBitInfo (unsigned long *info, short bit);
short INFO_IsSetBitInfo (unsigned long *info, short bit);
short INFO_GetBit (char *InfoName);
short INFO_ParseInfo(unsigned long *setinfo, unsigned long *unsetinfo);
void INFO_PrintInfo (unsigned long *info);


/*****************************************************************************/
/* get set and unset information from arguments. */
void SetUnset (short argc, char *argv [])
{
  short i;
  short bit;
  for (i=2; i<argc; i++) 
    {
      bit = INFO_GetBit (argv[i]);
      if (bit>0) INFO_SetBitInfo (setinfo, (short)(bit-1));
      else if (bit<0) INFO_SetBitInfo (unsetinfo, (short)(-bit-1));
    }
}

/*****************************************************************************/
/* show informatin field name list. */
void ViewName (void)
{
  short i;
  for (i=0; i<NUM_POS; i++)
    fprintf (stdout, "\n%s \t:%03d", PosName[i], i+1);
  fprintf (stdout, "\n");
}

/*****************************************************************************/
short FST_PutInfo (FST_INFO *fst_info, long index, unsigned long *info)
{
  int i;
  fseek (fst_info->fileptr, (index*(fst_info->infolen)*sizeof(long)), pBOF);
  for (i=0; i < fst_info->infolen; i++)
    fwrite (info+i, sizeof(long), 1, fst_info->fileptr);
  return SUCCESS;
}

/*****************************************************************************/
/* put key, with its own pos. */
void PutOwnPos (void){
  int index;
  int count = 0;
  int i;
  int nItem;

  /* infomation file을 쓰기위해 연다. */
  if (!(fst_info->fileptr = fopen (FSTinfoPath, "wb"))) return;
  
  while (ScanKey(0)) 
    {
      if ( (index = String2Hash(fst, tgkey, &nItem)) == (-1)) { 
        fprintf(stderr, "No entry in FST (line(%d): %s)\n", count+1, kskey);
        return; 
      }

      for (i=0; i<fst_info->infolen; i++) /* 초기화 */
        info[i] = 0l;

      if (!INFO_ParseInfo (setinfo, unsetinfo)) return;
      INFO_SetInfo (info, setinfo, unsetinfo);
      if (!FST_PutInfo (fst_info, index, info)) { perrmsg(); return; }
    }
}

/*****************************************************************************/
/* put key, with the same pos. */
/*void PutSamePos (void)
  {
  while (ScanKey(0)) 
  {
  if (!PDB_GetPos (pdb, tgkey, info)) { perrmsg(); return; }
  INFO_SetInfo (info, setinfo, unsetinfo);
  if (!PDB_PutPos (pdb, tgkey, info)) { perrmsg(); return; }
  }
  }
*/
/*****************************************************************************/
long notnull (unsigned long* info)
{
  short i;
  for (i=0; i<LEN_POS; i++)
    if (info[i]) return 1;
  return 0;
}

/*****************************************************************************/
/* get key and its pos, for all entrys. */
void GetAllKey (void) {
  int i;
  int quit = 0;
  char *ptr;
  char s[MAX_KEY_LENGTH];


  strcpy (tgkey, "\x1");

  for (i = 0; !quit; i++) {

    if ( (ptr = Hash2String(fst, i, s)) == NULL ) {
      fprintf(stderr, "number of index = %d\n", i);
      return;
    }

    strcpy(tgkey, ptr);
    if (!FST_GetPos (fst, fst_info, tgkey, info)) { perrmsg(); return; }
    if (notnull(info)) {
      PrintKey(0);
      INFO_PrintInfo (info);
    }
  }
}

/*****************************************************************************/
/* get key and its pos, for given entrys. */
void GetGivenKey (void)
{
  while (ScanKey(0)) 
    {
      if (!FST_GetPos (fst, fst_info, tgkey, info)) { perrmsg(); return; }
      PrintKey(0);
      INFO_PrintInfo (info);
    }
}

/*****************************************************************************/
/* get key, with the same as given information list. */
void GetSamePos (void) {
  int i;
  int quit = 0;
  char *ptr;
  char s[MAX_KEY_LENGTH];
  
  strcpy(tgkey, "\x1");

  for (i = 0; !quit; i++) {
    /* 엔트리를 얻어낸다 */
    if ( (ptr = Hash2String(fst, i, s)) == NULL) { /* 마지막까지 검색한 경우(더 이상의 엔트리가 없을 때) */
      return;
    }
    strcpy(tgkey, ptr);
    /* 엔트리의 사전정보를 얻는다 */
    if (!FST_GetPos (fst, fst_info, tgkey, info)) { perrmsg(); return; }
    if (INFO_InfoCompSame (info, setinfo, unsetinfo)) {
      PrintKey(0);
      INFO_PrintInfo (info);
    }
  }
}

/*****************************************************************************/
/* get key, if disjointed by given information list. */
void GetOrPos (void) {
  int i;
  int quit = 0;
  char *ptr;
  char s[MAX_KEY_LENGTH];
  
  strcpy(tgkey, "\x1");

  for (i = 0; !quit; i++) {
    /* 엔트리를 얻어낸다 */
    if ( (ptr = Hash2String(fst, i, s)) == NULL) { /* 마지막까지 검색한 경우(더 이상의 엔트리가 없을 때) */
      return;
    }
    strcpy(tgkey, ptr);
    /* 엔트리의 사전정보를 얻는다 */
    if (!FST_GetPos (fst, fst_info, tgkey, info)) { perrmsg(); return; }
    if (INFO_InfoCompOr (info, setinfo, unsetinfo)) {
      PrintKey(0);
      INFO_PrintInfo (info);
    }
  }
}
/*****************************************************************************/
/* get key, if masked by given information list. */
void GetMaskPos (void) {
  int i;
  int quit = 0;
  char *ptr;
  char s[MAX_KEY_LENGTH];
  
  strcpy(tgkey, "\x1");
  
  for (i = 0; !quit; i++) {
    /* 엔트리를 얻어낸다 */
    if ( (ptr = Hash2String(fst, i, s)) == NULL) { /* 마지막까지 검색한 경우(더 이상의 엔트리가 없을 때) */
      return;
    }
    strcpy(tgkey, ptr);
    /* 엔트리의 사전정보를 얻는다 */
    if (!FST_GetPos (fst, fst_info, tgkey, info)) { perrmsg(); return; }
    if (INFO_InfoCompOr (info, setinfo, unsetinfo)) {
      INFO_MaskInfo (info, setinfo, unsetinfo);      
      PrintKey(0);
      INFO_PrintInfo (info);
    }
  }
}


enum eOP 
{
  OP_QUIT = 1,
  OP_GET,
  /*  OP_PUT, */
  OP_INFOUsageHelp,
  OP_UsageHelp,
};

void UsageHelpView (void);
short GetCommand (void);

/*****************************************************************************/
/* view and edit b-tree dictionary. */
void View (void)
{
  UsageHelpView ();
  while (1) 
    {
      switch (GetCommand()) 
	{
	case OP_GET:
	  if (!ScanKey(1)) return;
	  if (!FST_GetPos (fst, fst_info, tgkey, info)) { perrmsg(); return; }
	  PrintKey(1);
	  INFO_PrintInfo (info);
	  break;
	  /*  case OP_PUT:
	      if (!ScanKey(1)) return;
	      if (!FST_GetPos (fst, fst_info, tgkey, info)) { perrmsg(); return; }
	      if (!INFO_ParseInfo (setinfo, unsetinfo)) return;
	      INFO_SetInfo (info, setinfo, unsetinfo);
	      if (!FST_PutPos (fst, tgkey, info)) { perrmsg(); return; }
	      break;*/
	case OP_INFOUsageHelp:
	  ViewName ();
	  break;
	case OP_UsageHelp:
	  UsageHelpView ();
	  break;
	default:    /* OP_QUIT */
	  return;
	}
    }
}

/*****************************************************************************/
/* UsageHelp for view. */
void UsageHelpView (void)
{
  fprintf (stderr, "\nUsage: <operator> [<operand list>]\n");
  fprintf (stderr, "\nGet info:        - <key>");    
  fprintf (stderr, "\nQuit:            . [or ^Z]");
  fprintf (stderr, "\ninfoname list:   #");
  fprintf (stderr, "\nUsageHelp:            ? ...\n");
}

/*****************************************************************************/
/* Get command from console. */
short GetCommand (void)
{
  char TempStr [LEN_KEY] = { (char)NULL, };
  fprintf (stderr, "\n> ");

  if ( fscanf (stdin, "%s", TempStr) == EOF ) return OP_QUIT;
  if ( !strcmp ("-", TempStr)) return OP_GET;
  /*else if ( !strcmp ("+", TempStr)) return OP_PUT;*/
  else if ( !strcmp (".", TempStr)) return OP_QUIT;
  else if ( !strcmp ("#", TempStr)) return OP_INFOUsageHelp;
  return OP_UsageHelp;
}

/*****************************************************************************/
/* Check if the information is the same as the given one.
   if so, return 1. otherwise, return 0. */
short INFO_InfoCompSame (unsigned long *info, unsigned long *setinfo, unsigned long *unsetinfo)
{
  short i;
  for (i=0; i<LEN_POS; i++)
    if (!((setinfo[i] == (info[i] & setinfo[i])) &&
	  (unsetinfo[i] == ((~info[i]) & unsetinfo[i]))))
      return 0;
  return 1;
}

/*****************************************************************************/
/* Check if the information is disjointed by the given one.
   if so, return 1. otherwise, return 0. */
short INFO_InfoCompOr (unsigned long *info, unsigned long *setinfo, unsigned long *unsetinfo)
{
  short i;
  for (i=0; i<LEN_POS; i++)
    if ( (setinfo[i] && !(info[i] & setinfo[i])) || 
	 (unsetinfo[i] && !(~info[i] & unsetinfo[i])) )
      return 0;
  return 1;
}

/*****************************************************************************/
/* set information by using set and unset information. */
void INFO_MaskInfo (unsigned long *info, unsigned long *setinfo, unsigned long *unsetinfo)
{
  short i;
  for (i=0; i<LEN_POS; i++)
    info[i] &= (setinfo[i] | unsetinfo[i]);
}

/*****************************************************************************/
/* set information by using set and unset information. */
void INFO_SetInfo (unsigned long *info, unsigned long *setinfo, unsigned long *unsetinfo)
{
  short i;
  for (i=0; i<LEN_POS; i++)
    info[i] = (info[i] | setinfo[i]) & (~unsetinfo[i]);
}

/*****************************************************************************/
/* set bit index of information of entry. */
void INFO_SetBitInfo (unsigned long *info, short bit)
{
  short d;      /* d-th double word */
  short b;      /* b-th bit index in double word */
  if (bit>=NUM_POS) return;
  d = (short) bit / 32;
  b = (short) bit % 32;
  info [d] |= ((unsigned long)1 << b);
}

/*****************************************************************************/
/* if bit index of information of entry is set. */
short INFO_IsSetBitInfo (unsigned long *info, short bit)
{
  short d;      /* d-th double word */
  short b;      /* b-th bit index in double word */
  if (bit>=NUM_POS) return 0;
  d = (short) bit / 32;
  b = (short) bit % 32;
  if ( info [d] & ((unsigned long)1 << b) ) return 1;
  return 0;
}

/*****************************************************************************/
/* get information bit number from information name. */
short INFO_GetBit (char *infoname)
{
  short bit;
  if (infoname[0] == '-' || infoname[0] == '+') 
    {
      for (bit=0; bit<NUM_POS; bit++)
	if (!strcmp (PosName[bit], infoname+1)) break;
      if (bit==NUM_POS) 
	{
	  fprintf (stderr, "(%s) is not information name..!!\n", infoname);
	  return 0;
	}
      if (infoname[0] == '-') return -(bit+1);
    } 
  else 
    {
      for (bit=0; bit<NUM_POS; bit++)
	if (!strcmp (PosName[bit], infoname)) break;
      if (bit==NUM_POS) 
	{
	  fprintf (stderr, "(%s) is not information name..!!\n", infoname);
	  return 0;
	}
    }
  return bit+1;
}

/*****************************************************************************/
/* parse set and unset information.
   if error, return 0. otherwise, return 1. */
short INFO_ParseInfo(unsigned long *setinfo, unsigned long *unsetinfo)
{
  char temps[80] = {0,};
  short bit = 0;
  short loopcnt = 0;
  short i;
  for (i = 0; i<LEN_POS; i++) 
    {
      setinfo[i] = 0lu;
      unsetinfo[i] = 0lu;
    }
  /* read information block begin marker, { */
  if (fscanf (stdin, "%s", temps) == EOF) 
    {
      fprintf (stderr, "illegal EOF..!!\n");
      return 0;
    }
  if (strcmp("{", temps)) 
    {
      fprintf (stderr, "(%s) is not block begin marker {..!!\n", temps);
      return 1;
    }
  if (fscanf (stdin, "%s", temps) == EOF) 
    {
      fprintf (stderr, "illegal EOF..!!\n");
      return 0;
    }
  /* until read information block end marker, } */
  while (strcmp("}", temps)) 
    {
      if (++loopcnt>NUM_POS) 
	{
	  fprintf (stderr, "\nPerhaps you got in infinite loop..!!\n");
	  return 0;
	}
      bit = INFO_GetBit (temps);
      if (bit>0) INFO_SetBitInfo (setinfo, (short)(bit-1));
      else if (bit<0) INFO_SetBitInfo (unsetinfo, (short)(-bit-1));
      if (fscanf (stdin, "%s", temps) == EOF) 
	{
	  fprintf (stderr, "illegal EOF..!!\n");
	  return 0;
	}
    }
  return 1;
}

/*****************************************************************************/
/* view information */
void INFO_PrintInfo (unsigned long *info)
{
  short d;      /* d-th double word */
  short b;      /* b-th bit index in double word */
  short bit;
  fprintf (stdout, "{");
  for (bit=0; bit<NUM_POS; bit++) 
    {
      d = (short) bit / 32;
      b = (short) bit % 32;
      if (info [d] & ((unsigned long)1 << b))
	fprintf (stdout, " %s", PosName[bit]);
    }
  fprintf (stdout, " }\n");
}
