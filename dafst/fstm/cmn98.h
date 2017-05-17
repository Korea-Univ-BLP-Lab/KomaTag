/* Filename	: cmn98.h */
/* Version	: 98 */
/* Discription	: external header of cmn98.c */
/* Programmer	: Do-Gil Lee (Dept. of Computer Science, Korea University) */
/* Date		: 15 March 2001 */
/* Modefied by Do-gil Lee */

#ifndef __CMN98_H__
#define __CMN98_H__

#include <stdio.h>

/* compile system information */
#define _UNIX_ /* unix system */

/* related to return value */
#ifndef SUCCESS
#define SUCCESS		((short)1) /* success */
#endif
#ifndef ERROR
#define ERROR		((short)0) /* error */
#endif

#ifndef YES
#define YES 1
#endif
#ifndef NO
#define NO  0
#endif
#ifndef NONE
#define NONE  -1
#endif
#ifndef ALL
#define ALL  2
#endif

/* related to link or pointer */
#ifndef NOTEXIST
#define NOTEXIST        ((int)-1)  /* something does not exist */
#endif
#ifndef NILPTR
#define NILPTR          ((int)-2)  /* pointer to nothing */
#endif

/* maximal length of error message */
#ifndef MAXLEN_ERRMSG
#define MAXLEN_ERRMSG   (200)
#endif

/* maximal length of a line delimited by carriage return */
#ifndef MAXLEN_LINE
#define MAXLEN_LINE     (1000)
#endif

/* maximal length of a word */
#ifndef MAXLEN_WORD
#define MAXLEN_WORD	(500)
#endif

/* maximal length of a line delimited by carriage return for more long line */
#ifndef MAXLEN_LINE2
#define MAXLEN_LINE2     (10000)
#endif

/* maximal length of a word for more long word */
#ifndef MAXLEN_WORD2
#define MAXLEN_WORD2	(9000)
#endif

/* maximal length of a key in a dictionary */
#ifndef MAXLEN_KEY
#define MAXLEN_KEY      (100)
#endif

/* Korean character area of Trigem-code(combination-type code) */
#ifndef isKOR
#define isKOR(x1,x2) ((x1 >= 0x84 && x1 <= 0xD3) && ((x2 >= 0x41 && x2 <= 0x7E) || (x2 >= 0x81 && x2 <= 0xFE)))
#endif
/* Chinese character area of Trigem-code(combination-type code) */
#ifndef isHJA
#define isHJA(x1,x2) ((x1 >= 0xE0 && x1 <= 0xF9) && ((x2 >= 0x31 && x2 <= 0x7E) || (x2 >= 0x91 && x2 <= 0xFE)))
#endif

/* 완성형 */
#ifndef isHanja /* 한자 */
#define isHanja(str0, str1) ( (str0 >= 0xCA) && (str0 <= 0xFD) && (str1 >= 0xA1) && (str1 <= 0xFE) )
#endif

#ifndef isHangul /* 한글 */
#define isHangul(str0, str1) ( (str0 >= 0xB0) && (str0 <= 0xC8) && (str1 >= 0xA1) && (str1 <= 0xFE) )
#endif

#ifndef is2Byte /* 2byte Code */
#define is2Byte(str0, str1) ( (str0 >= 0xA1) && (str0 <= 0xAC) && (str1 >= 0xA1) && (str1 <= 0xFE) )
#endif

/* related to location or position in file, for fseek */
#ifndef pBOF
#define pBOF		(0)		/* beginning of file */
#endif
#ifndef pCOF
#define pCOF		(1)		/* current position of file */
#endif
#ifndef pEOF
#define pEOF		(2)		/* end of file */
#endif

/* error message */
extern char errmsg[];

/* print error message into stderr */
extern void perrmsg(void);

/* get a line in file.
   if success, return SUCCESS.
   if error, return ERROR. */
extern short f_GetLine (char* line, FILE* file);

/* get a line in file for more long line.
   if success, return SUCCESS.
   if error, return ERROR. */
extern short f_GetLine2 (char* line, FILE* file);

/* get a long line in file.
    initial state: *line == NULL, *len == 0.
   if success, return SUCCESS.
   if error, return ERROR. */
extern char* f_GetLongLine (char* line, int* len, FILE* file);

/* return a beginning of a line ending in the given location of file. 
   if there is no line, return the given location. */
extern int f_BeginLine (int endloc, FILE* file);

/* if the file exists, remove it.
   if success, return file handle.
   if error, return NULL. */
extern short f_Remove(char* name, char* mode);

/* open file by given mode.
   if success, return file handle.
   if error, return NULL. */
extern FILE* f_Open (char* name, char* mode);

/* open temporary file.
   if success, return file handle.
   if error, return NULL. */
extern FILE* f_OpenTemp(char* prefix, char* ext, char* mode, char* name);

#endif
