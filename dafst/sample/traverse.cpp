#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libcommon.h"

#define MAX_KEY_LENGTH 1024

void TraverseCallback(void *pParam, const char *s, int Hash, int nItem)
{
  printf("%d(%d) : %s\n", Hash, nItem, s);
}

int main(int argc, char *argv[])
{
  void *x;
  char s[MAX_KEY_LENGTH];
  int i, Hash, nItem;

  if (argc!=2) {
    fprintf(stderr, "DAFST : key -> hash conversion program\n");
    fprintf(stderr, " USAGE : %s <FST filename>\n", argv[0]);
    return 0;
  }

  if ((x=LoadTransducer(argv[1], NULL))==NULL) {
    fprintf(stderr, "Load failure\n");
    return 0;
  }

  TraverseTransducer(x, NULL, TraverseCallback);

  FreeTransducer(x);
  return 1;
}
