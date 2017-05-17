/* Filename	: cmn98.c */
/* Version	: 98 */
/* Discription	: library for common tools */
/* Programmer	: Sang-Zoo Lee (Dept. of Computer Science, Korea University) */
/* Date		: 26 August 1998 */
/* Specification: frequently used functions
   relating to error message handling, file handling, etc. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmn98.h"

/* error message */
char errmsg [MAXLEN_ERRMSG];

/* print error message into stderr */
void perrmsg(void)
{
  fprintf (stderr, "\n%s\n", errmsg);
}

/* get a line in file.
   if success, return SUCCESS.
   if error, return ERROR. */
short f_GetLine (char* line, FILE* file)
{
  short ch = 0;
  short i = 0;
  while ((ch = getc (file)) != EOF) 
    {
      line [i++] = (unsigned char) ch;
      if (i >= (MAXLEN_LINE-1)) 
	{
	  line[MAXLEN_LINE-1] = (char)NULL;
	  sprintf (errmsg, "\nError! Too long line(>%d)..!\n%s\n", MAXLEN_LINE, line);
	  return ERROR;
	}
      if (ch != 0x0a) continue;
      line [i] = 0;
      return SUCCESS;
    }
  line [i] = 0;
  sprintf (errmsg, "\nError! unexpected EOF..!\n%s\n", line);
  return ERROR;
}

/* get a line in file for more long line.
   if success, return SUCCESS.
   if error, return ERROR. */
short f_GetLine2 (char* line, FILE* file)
{
  short ch = 0;
  short i = 0;
  while ((ch = getc (file)) != EOF) 
    {
      line [i++] = (unsigned char) ch;
      if (i >= (MAXLEN_LINE2-1)) 
	{
	  line[MAXLEN_LINE2-1] = (char)NULL;
	  sprintf (errmsg, "\nError! Too long line(>%d)..!\n%s\n", MAXLEN_LINE2, line);
	  return ERROR;
	}
      if (ch != 0x0a) continue;
      line [i] = 0;
      return SUCCESS;
    }
  line [i] = 0;
  sprintf (errmsg, "\nError! unexpected EOF..!\n%s\n", line);
  return ERROR;
}

/* get a long line in file.
    initial state: *line == NULL, *len == 0
   if success, return SUCCESS.
   if error, return ERROR. */
char* f_GetLongLine (char* line, int* len, FILE* file)
{
  short ch = 0;
  short i = 0;
  char* temp;
  int tlen;
    
  if ( (*len) == 0 ) 
    {
      (*len) = MAXLEN_LINE;
      if (! (line = (char*)malloc((*len))) ) 
	{
	  sprintf (errmsg, "\nError! No available memory for line buffer(%d)..!\n", len);
	  return NULL;
	}
    }
  memset (line, 0, (*len));
  while ((ch = getc (file)) != EOF) 
    {
      line [i++] = (unsigned char) ch;
      if (i >= ((*len)-1)) 
	{
	  tlen = (*len) + MAXLEN_LINE;
	  if (! (temp = (char*)malloc(tlen)) ) 
	    {
	      line[(*len)-1] = (char)NULL;
	      sprintf (errmsg, "\nError! No available memory for line buffer(%d)..!\n%s\n", tlen, line);
	      return NULL;
	    }
	  memset (temp, 0, tlen);
	  memcpy (temp, line, (*len));
	  free(line);
	  (*len) = tlen;
	  line = temp;
	}
      if (ch != 0x0a) continue;
      return line;
    }
  return line;
}

/* return a beginning of a line ending in the given location of file. 
   if there is no line, return the given location. */
int f_BeginLine (int endloc, FILE* file)
{
  short ch = 0;
  int loc=endloc;

  while ( loc > 0 ) 
    {
      loc--;
      fseek (file, loc, pBOF);
      ch = getc (file);
      if ( ch != 0x0a && loc != 0 ) continue;
      if ( endloc == ftell(file) ) continue;
      return ftell(file);
    }
  return endloc;
}

/* if the file exists, remove it.
   if success, return SUCCESS.
   if error, return ERROR. */
short f_Remove(char* name, char* mode)
{
  FILE* file;

  /* if the file exists, remove it. */
  if ( file = fopen(name, mode) ) 
    {
      fclose(file);
      if ( remove(name) != 0 ) 
	{
	  sprintf (errmsg, "ERROR! cannot remove file(%s)..!!\n", name);
	  return ERROR;
	}
    }
  return SUCCESS;
}

/* open file by given mode.
   if success, return file handle.
   if error, return NULL. */
FILE* f_Open (char* name, char* mode)
{
  FILE* file;

  /* open file by given mode. */
  if (! (file = fopen(name, mode)) ) {
    sprintf (errmsg, "ERROR! cannot open file(%s) by %s-mode..!!\n", name, mode);
    return NULL;
  }
  return file;
}

/* open temporary file.
   if success, return file handle.
   if error, return NULL. */
FILE* f_OpenTemp(char* prefix, char* ext, char* mode, char* name)
{
  char suffix[4] = "aaa";
  FILE* tmpfile;
  char tmpname[MAXLEN_WORD];

  /* make name of the first temporary file. */
  sprintf (tmpname, "%s%s%s", prefix, suffix, ext);
  while (! (tmpfile = f_Open(tmpname, mode)) )
    {
      /* make name of the next temporary file. */
      if ( suffix[2] == 'z' ) 
	{
	  suffix[2] = 'a';
	  if ( suffix[1] == 'z' ) 
	    {
	      suffix[1] = 'a';
	      if ( suffix[0] == 'z' ) 
		{
		  sprintf(errmsg, "ERROR! cannot make temporary file..!!\n");
		  return NULL;
		} 
	      else suffix[0] += (char)1;
	    } 
	  else suffix[1] += (char)1;
	} 
      else suffix[2] += (char)1;
      sprintf (tmpname, "%s%s%s", prefix, suffix, ext);
    }
  strcpy (name, tmpname);
  return tmpfile;
}
