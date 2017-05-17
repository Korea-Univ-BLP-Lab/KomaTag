#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libcommon.h"

#define MAX_KEY_LENGTH 1024

int main(int argc, char *argv[])
{
  void *x;
  int n;
  char s[MAX_KEY_LENGTH];

  if (argc!=2) {
    fprintf(stderr, "DAFST : hash -> key conversion program\n");
    fprintf(stderr, " USAGE : %s <FST filename>\n", argv[0]);
    return 0;
  }

  if ((x=LoadTransducer(argv[1], NULL))==NULL) return 0;

  printf("Enter hash value : ");
  while (scanf("%d", &n)!=EOF) {
    if (Hash2String(x, n, s))
      printf("%d : %s\n", n, s);
    else printf("%d : invalid hash\n", n);
    printf("Enter hash value : ");
  }
  printf("\n");

  FreeTransducer(x);
  return 1;
}
