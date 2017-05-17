#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libcommon.h"

#define MAX_KEY_LENGTH 1024

int main(int argc, char *argv[])
{
  void *x;
  int i, n;
  char s[MAX_KEY_LENGTH];

  if (argc!=2) {
    fprintf(stderr, "DAFST : traverse(by Hash2String) program\n");
    fprintf(stderr, " USAGE : %s <FST filename>\n", argv[0]);
    return 0;
  }

  if ((x=LoadTransducer(argv[1], NULL))==NULL) return 0;

  n=GetNumberOfEntry(x);

  for (i=0; i<n; i++) {
    if (Hash2String(x, i, s))
      printf("%d : %s\n", i, s);
  }

  FreeTransducer(x);
  return 1;
}
