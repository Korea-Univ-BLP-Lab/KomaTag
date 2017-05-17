#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libcommon.h"

#define MAX_KEY_LENGTH 1024

int main(int argc, char *argv[])
{
  void *x;
  char s[MAX_KEY_LENGTH];

  if (argc!=3) {
    fprintf(stderr, "DAFST : appending program\n");
    fprintf(stderr, " USAGE : %s <FST filename> <supplement filename>\n", 
	    argv[0]);
    return 0;
  }

  if ((x=LoadTransducer(argv[1], argv[2]))==NULL) 
    return 0;

  printf("Enter key to append : ");
  while (ANFgetsWithTrim(s, MAX_KEY_LENGTH, stdin)) {
    if (s[0])
      if (!RegisterKey(x, s)) {
	printf("ERROR :: cannot register key %s\n", s);
	return 0;
      }
    printf("Enter key to append : ");
  }
  printf("\nsaving files...\n");
  SaveTransducer(x, argv[1], argv[2]);
  FreeTransducer(x);
  printf("done\n");
  return 1;
}
