#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#ifdef _WIN32_WCE
#include <afxwin.h>
#endif
#ifdef WIN32
#include <afxwin.h>
#endif

#include "ANCommon.h"

#ifndef _DEBUG
#define _DEBUG
#endif

/*******************/
/* error functions */
/*******************/
static int ANErrorCode=ANERROR_NONE;
static const char *ANErrorString[]={
  "invalid filename",
  "invalid file handle",
  "no memory",
  "null pointer detected",
  "seek fail",
  "write fail to file",
  "undefined error"
};

void ANClearError(void)
{

  ANErrorCode=ANERROR_NONE;
}

int ANGetLastError(void)
{

  return ANErrorCode;
}

const char *ANGetErrorString(int ErrorCode)
{

  if (ErrorCode==ANERROR_NONE) 
    return "";
  else if ((ErrorCode>=ANERROR_BASE)&&(ErrorCode<ANERROR_MAX))
    return ANErrorString[ErrorCode-ANERROR_BASE];
  else return ANErrorString[ANERROR_UNDEFINED];
}

const char *ANGetLastErrorString(void)
{

  return ANGetErrorString(ANGetLastError());
}

#define IS_KSC0(c) ((((unsigned char)(c))>=0xb0)&&(((unsigned char)(c))<=0xc8))
#define IS_KSC1(c) ((((unsigned char)(c))>=0xa1)&&(((unsigned char)(c))<=0xfe))

void PrintHexa(const void *pD, int Size)
{
  int i, Base=0;
  char Line[80];
  const char *pData=(const char *)pD;

  while (Size>0) {
    memset(Line, 0x20, 80);

    sprintf(Line, "%08Xh: ", Base);
    Line[76]=0;
    for (i=0; i<16; i++) {
      if (i>=Size) break;
      sprintf(Line+3*i+11, "%02X ", (pData[Base+i]*1)&0xff);
      if ((pData[Base+i]>' ')||
          IS_KSC0(pData[Base+i])||
          ((i>0)&&IS_KSC0(pData[Base+i-1])&&IS_KSC1(pData[Base+i]))) {
        Line[60+i]=pData[Base+i];
      }
    }
    Line[11+i*3]=' ';
    Base+=i;
    Size-=i;
    Line[59]='|';
    LogCallback("%s", Line);
  }
}

/****************************/
/* message manage functions */
/****************************/

#define MaxErrorBuffer 256
#ifdef WIN32
#ifdef _WIN32_WCE
void DefaultErrorCallback(const char *Format, ...)
{
  char ErrorBuffer[MaxErrorBuffer];
  CString s;

  va_list args;
  va_start(args, Format);
  vsprintf(ErrorBuffer, Format, args);
  va_end(args);
  s=ErrorBuffer;
  ::MessageBox(NULL, s, NULL, MB_OK);
/*#ifdef _DEBUG
  __asm {int 3};
#endif*/
}
#else /* !_WIN32_WCE */
void DefaultErrorCallback(const char *Format, ...)
{
  char ErrorBuffer[MaxErrorBuffer];

  va_list args;
  va_start(args, Format);
  vsprintf(ErrorBuffer, Format, args);
  va_end(args);
  ::MessageBox(NULL, ErrorBuffer, NULL, MB_OK);
#ifdef _DEBUG
  __asm {int 3}; // break
#endif
}
#endif
#else
static char gLogFilename[256]={0,};
static char gErrorFilename[256]={0,};
static char gLogHeader[256];

void DefaultLogCallback(const char *Format, ...)
{
  FILE *f=NULL;
  va_list args;
  time_t ct;
  char tmstr[32];

  if (gLogFilename[0]) {
    if ((f=fopen(gLogFilename, "r+b"))!=NULL) 
      fseek(f, 0, 2);
  }
  
  // get current time
  ct=time(0);
  strcpy(tmstr,asctime(localtime(&ct))+11);
  tmstr[8]=0;

  if (f)
    fprintf(f, "%s :: ", tmstr);
  fprintf(stderr, "%s(%s) :: ", tmstr, gLogHeader);
  

  va_start(args, Format);
  if (f)
    vfprintf(f, Format, args);
  vfprintf(stderr, Format, args);
  va_end(args);

  if (f)
    fprintf(f, "\n");
  fprintf(stderr, "\n");
  
  if (f)
    fclose(f);
}

void DefaultErrorCallback(const char *Format, ...)
{
  FILE *f;
  va_list args;

  if (gErrorFilename[0]) {
    if ((f=fopen(gErrorFilename, "r+b"))!=NULL) 
      fseek(f, 0, 2);
  }

  if (f)
    fprintf(f, "ERROR :: ");
  fprintf(stderr, "ERROR(%s) :: ", gLogHeader);

  va_start(args, Format);
  if (f)
    vfprintf(f, Format, args);
  vfprintf(stderr, Format, args);
  va_end(args);

  if (f)
    fprintf(f, "\n");
  fprintf(stderr, "\n");
  
  if (f)
    fclose(f);
}

#endif

/*void PrepareLog(const char *LogFilename, const char *ErrorFilename, const char *LogHeader, int bRemove)
{
  FILE *f;
  
  if (bRemove) {
    remove(LogFilename);
    remove(ErrorFilename);
  }
  
  strcpy(gLogFilename, LogFilename);
  strcpy(gErrorFilename, ErrorFilename);
  strcpy(gLogHeader, LogHeader);
  
  if ((f=fopen(gLogFilename, "wb"))!=NULL)
    fclose(f);
  if ((f=fopen(gErrorFilename, "wb"))!=NULL)
    fclose(f);
}
*/
void (*ErrorCallback)(const char *Format, ...)=DefaultErrorCallback;
#ifdef WIN32
void (*LogCallback)(const char *Format, ...)=DefaultErrorCallback;
#else
void (*LogCallback)(const char *Format, ...)=DefaultLogCallback;
#endif

void RegisterErrorCallback(void (*Callback)(const char *Format, ...))
{
  if (Callback)
    ErrorCallback=Callback;
}

void RegisterLogCallback(void (*Callback)(const char *Format, ...))
{
  if (Callback)
    LogCallback=Callback;
}

/*************************/
/* file manage functions */
/*************************/

FILE *ANFopen(const char *Filename, const char *Mode)
{
  FILE *f;

  if ((f=fopen(Filename, Mode))==NULL) {
    ANErrorCode=ANERROR_FILENOTEXIST;
    ErrorCallback("cannot open file(%s)", Filename);
  }
  return f;
}

int ANIsFileExist(const char *Filename)
{
  FILE *f;

  if ((f=fopen(Filename, "rb"))==NULL)
    return 0;
  else {
    fclose(f);
    return 1;
  }
}

void ANFclose(FILE *f)
{
  if (f==NULL) {
    ANErrorCode=ANERROR_INVALIDFILE;
    ErrorCallback("cannot close NULL file");
  }
  else fclose(f);
}

int ANFseek(FILE *f, long offset, int origin)
{
  int ReturnValue=fseek(f, offset, origin);

  if (ReturnValue) {
    ANErrorCode=ANERROR_SEEKFAIL;
    ErrorCallback("cannot move file pointer");
  }

  return ReturnValue;
}

size_t ANFread(void *buffer, size_t size, FILE *f)
{
  size_t ReturnValue;

  if (size<=0) return size;
  
  ReturnValue=fread(buffer, 1, size, f);
  if (ReturnValue!=size) {
    ANErrorCode=ANERROR_WRITEFAIL;
    ErrorCallback("ERROR :: cannot read from file");
  }

  return ReturnValue;
}

size_t ANFwrite(const void *buffer, size_t size, FILE *f)
{
  size_t ReturnValue;

  if (size<=0) return size;

  ReturnValue=fwrite(buffer, 1, size, f);

  if (ReturnValue!=size) {
    ANErrorCode=ANERROR_WRITEFAIL;
    ErrorCallback("ERROR :: cannot write to file");
  }

  return ReturnValue;
}

size_t ANFwriteRepeat(char c, size_t size, FILE *f)
{
  size_t i;

  for (i=0; i<size; i++)
    if (fwrite(&c, 1, 1, f)!=1) {
      ANErrorCode=ANERROR_WRITEFAIL;
      ErrorCallback("ERROR :: cannot write to file");
      return i;
    }

  return i;
}

char *ANFgets(char *Line, int Size, FILE *f)
{

  if ((f==NULL)||(Line==NULL)) return NULL;
  return fgets(Line, Size, f);
}

static const char *WhiteSpace=" \t\r\n";

char *ANFgetsWithTrim(char *Line, int Size, FILE *f)
{
  int Length;

  if ((f==NULL)||(Line==NULL)) return NULL;
  while (fgets(Line, Size, f)!=NULL) {
    Length=strlen(Line);
    while (Length&&strchr(WhiteSpace, Line[Length-1])) {
      Length--;
      Line[Length]=0;
    }
    if (Line[0]) return Line;
  }
  return NULL;
}

long ANGetFileSize(FILE *f)
{
  if (f==NULL) return 0;
  long Index=ftell(f), Size;

  fseek(f, 0, SEEK_END);
  Size=ftell(f);
  fseek(f, Index, SEEK_SET);
  return Size;
}

#define MaxLineLength 40960

long ANGetNumberOfLine(FILE *f)
{
  if (f==NULL) return 0;
  long Index=ftell(f), nLine;
  char s[MaxLineLength];

  fseek(f, 0, SEEK_SET);
  for (nLine=0; ANFgets(s, MaxLineLength, f); nLine++);
  fseek(f, Index, SEEK_SET);
  return nLine;
}

void *ANReadAllFile(FILE *f)
{
  if (f==NULL) return NULL;
  long Size=ANGetFileSize(f);
  long Index=ftell(f);
  void *AllData=ANMalloc(Size+1);
  if (AllData) {
    fseek(f, 0, SEEK_SET);
    fread(AllData, Size, 1, f);
    ((char *)AllData)[Size]=0;
    fseek(f, Index, SEEK_SET);
  }
  return (void *)AllData;
}

/***************************/
/* memory manage functions */
/***************************/

void *ANMalloc(size_t Size)
{
  void *x;

  if ((int)Size<=0) {
    ANErrorCode=ANERROR_NOMEMORY;
    ErrorCallback("invalid size for malloc(%d)", Size);
    return NULL;
  }
  if ((x=malloc(Size))==NULL) {
    ANErrorCode=ANERROR_NOMEMORY;
    ErrorCallback("not enough memory during malloc(%d)", Size);
  }
  return x;
}

void *ANRealloc(void *x, size_t NewSize)
{

  if ((x=realloc(x, NewSize))==NULL) {
    ANErrorCode=ANERROR_NOMEMORY;
    ErrorCallback("not enough memory during realloc(%d)", NewSize);
  }
  return x;
}

void ANFree(void *x)
{
  if (x==NULL) {
    ANErrorCode=ANERROR_INVALIDPOINTER;
    ErrorCallback("cannot free NULL pointer");
  }
  else free(x);
}

/***************************/
/* string manage functions */
/***************************/

char *ANStrdup(const char *src)
{
  char *dest=NULL;
  if (src) {
    if ((dest=strdup(src))==NULL) {
      ANErrorCode=ANERROR_NOMEMORY;
      ErrorCallback("not enough memory during strdup(%s)", src);
    }
  }
  else {
    ANErrorCode=ANERROR_INVALIDPOINTER;
    ErrorCallback("null string is detected on strdup");
  }
  return dest;
}

void ANTrim(char *s)
{
  int Length;

  if (s) {
    Length=strlen(s);
    while (Length&&strchr(WhiteSpace, s[Length-1])) {
      Length--;
      s[Length]=0;
    }
  }
}

/***************************/
/* miscellaneous functions */
/***************************/
int ANGetAlignmentSize(void)
{
  typedef struct { char x; short y; } t1;
  typedef struct { short x; int y; } t2;
  typedef struct { int x; long y; } t3;
  typedef struct { long x; double y; } t4;

  if (sizeof(t1)==sizeof(char)+sizeof(short)) return sizeof(char);
  if (sizeof(short)<sizeof(int))
    if (sizeof(t2)==sizeof(short)+sizeof(int))  return sizeof(short);
  if (sizeof(int)<sizeof(long))
    if (sizeof(t3)==sizeof(int)+sizeof(long)) return sizeof(int);
  if (sizeof(long)<sizeof(double))
    if (sizeof(t4)==sizeof(long)+sizeof(double)) return sizeof(long);
  return sizeof(double);
}

void ReverseData(void *pData, int Size)
{
  char *s=(char *)pData, c;
  int i;

  for (i=0; i<Size/2; i++) {
    c=s[i];
    s[i]=s[Size-i-1];
    s[Size-i-1]=c;
  }
}

