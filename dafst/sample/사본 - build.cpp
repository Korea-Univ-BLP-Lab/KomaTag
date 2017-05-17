#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libcommon.h"

#define MAX_KEY_LENGTH 1024

int main(int argc, char *argv[])
{
  void *x;
  FILE *f;
  char s[MAX_KEY_LENGTH];
  int n=0;

  if (argc!=4) {
    fprintf(stderr, "DAFST : dynamic acyclic FST builder\n");
    fprintf(stderr, " USAGE : %s <key filename> <FST filename> <supplement filename>\n", 
	    argv[0]);
    return 0;
  }

  if ((f=fopen(argv[1], "rb"))==NULL) {
    fprintf(stderr, "ERROR :: cannot open key file %s\n", argv[1]);
    return 0;
  }

  if ((x=NewTransducer())==NULL) {
    fprintf(stderr, "ERROR :: cannot make new FST\n");
    fclose(f);
    return 0;
  }

  while (ANFgetsWithTrim(s, MAX_KEY_LENGTH, f)) {
    if (s[0]==0) {
      fprintf(stderr, "WARNING :: empty key was detected on line %d\n", n);
      continue;
    }
    if (!RegisterKey(x, s)) {
      fprintf(stderr, "ERROR :: cannot register key %s\n", s);
      FreeTransducer(x);
      fclose(f);
      return 0;
    }
    n++;
    if (n%1000==0)
      fprintf(stderr, "\radded %d keys", n);
  }
  fprintf(stderr, "\radded total %d keys\n", n);
  fprintf(stderr, "saving files...\n");
  if (!SaveTransducer(x, argv[2], argv[3])) 
    fprintf(stderr, "ERROR :: cannot save FST to %s %s\n", argv[2], argv[3]);
  FreeTransducer(x);
  fclose(f);
  fprintf(stderr, "done\n");
  return 1;
}
