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

  build_fst(argv[1], argv[2], argv[3]);

  return 1;
}
