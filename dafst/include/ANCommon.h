#ifndef __ANCOMMON__
#define __ANCOMMON__

#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#define strcasecmp stricmp
#define strdup _strdup
#endif

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

// error definition
#define ANERROR_NONE            0
#define ANERROR_BASE            0x4000000
#define ANERROR_FILENOTEXIST    (ANERROR_BASE+0)
#define ANERROR_INVALIDFILE     (ANERROR_BASE+1)
#define ANERROR_NOMEMORY        (ANERROR_BASE+2)
#define ANERROR_INVALIDPOINTER  (ANERROR_BASE+3)
#define ANERROR_SEEKFAIL        (ANERROR_BASE+4)
#define ANERROR_WRITEFAIL       (ANERROR_BASE+5)
#define ANERROR_UNDEFINED       (ANERROR_BASE+6)
#define ANERROR_MAX             ANERROR_UNDEFINED       

/*****************/
/* dump function */
/*****************/
void PrintHexa(const void *pData, int Size);

void PrepareLog(const char *LogFilename, const char *ErrorFilename, const char *LogHeader, int bRemove);

/*******************/
/* error functions */
/*******************/

// get last error code
int ANGetLastError(void);
// get last error string
const char *ANGetLastErrorString(void);
// get matched error string
const char *ANGetErrorString(int ErrorCode);
// clear last error
void ANClearError(void);

// setting error callback function
// The usage of Callback is the same as that of printf()
// The Default error callback function is set to be 
//    MessageBox(); under WIN32 and
//    fprintf(stderr, Format, ...); otherwise
// If you don't do anything on occurring error,
//    just call RegisterErrorCallback(NULL);
void RegisterErrorCallback(void (*Callback)(const char *Format, ...));
void RegisterLogCallback(void (*Callback)(const char *Format, ...));

extern void (*ErrorCallback)(const char *Format, ...);
extern void (*LogCallback)(const char *Format, ...);

/*************************/
/* file manage functions */
/*************************/

// fopen with exception handling
FILE *ANFopen(const char *Filename, const char *Mode);

// check if the file exists   0 : not exist, 1 : exist
int ANIsFileExist(const char *Filename);

// fclose with exception handling
void ANFclose(FILE *f);

// fseek with exception handling
int ANFseek(FILE *f, long offset, int origin);

// fread(the third parameter is 1) with exception handling
size_t ANFread(void *buffer, size_t size, FILE *f);

// fwrite(the third parameter is 1) with exception handling
size_t ANFwrite(const void *buffer, size_t size, FILE *f);

// fwrite a specific character repeatedly
size_t ANFwriteRepeat(char c, size_t size, FILE *f);

// fgets with exception handling
char *ANFgets(char *Line, int Size, FILE *f);

// fgets with exception handling and Trimming
char *ANFgetsWithTrim(char *Line, int Size, FILE *f);

// return file size
long ANGetFileSize(FILE *f);

// return the number of line of a text file
long ANGetNumberOfLine(FILE *f);

// allocate memory and read the whole content of a text file
void *ANReadAllFile(FILE *f);

/***************************/
/* memory manage functions */
/***************************/

// malloc with exception handling
void *ANMalloc(size_t Size);

// realloc with exception handling
void *ANRealloc(void *x, size_t NewSize);

// free with exception handling
void ANFree(void *x);

/***************************/
/* string manage functions */
/***************************/

// strdup with exception handling
char *ANStrdup(const char *src);

// trim white space and line break characters from the end of string
void ANTrim(char *s);

/***************************/
/* miscellaneous functions */
/***************************/
int ANGetAlignmentSize(void);

void ReverseData(void *pData, int Size);

#endif
