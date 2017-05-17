#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libcommon.h"

#define MAX_KEY_LENGTH 1024

int main(int argc, char *argv[])
{
  void *x;
  char s[MAX_KEY_LENGTH];
  int i, Hash, nItem, ToBeDeleted;

  if (argc!=3) {
    fprintf(stderr, "DAFST : remove keys\n");
    fprintf(stderr, " USAGE : %s <FST filename> <supplement filename>\n", 
	    argv[0]);
    return 0;
  }

  if ((x=LoadTransducer(argv[1], argv[2]))==NULL) 
    return 0;

  printf("Enter key to remove : ");
  while (ANFgetsWithTrim(s, MAX_KEY_LENGTH, stdin)) {
    if (s[0]) {
      Hash=String2Hash(x, s, &nItem);
      if (Hash==NOT_EXIST)
	printf("%s : not exist\n", s);
      else {
	if (nItem==1) {
	  if (!DeleteKey(x, s, ToBeDeleted)) {
	    fprintf(stderr, "ERROR on delete %s %d\n", s, Hash);
	    return 0;
	  }
	  else printf("deleted %s %d\n", s, Hash);
	}
	else {
	  while (1) {
	    printf("Candidates :");
	    for (i=0; i<nItem; i++)
	      printf(" %d", Hash+i);
	    printf("\nEnter -1(delete all) or hash value of key to delete : ");
	    scanf("%d", &ToBeDeleted);
	    if ((ToBeDeleted==-1)||
		((ToBeDeleted>=Hash)&&(ToBeDeleted<Hash+nItem))) {
	      if (!DeleteKey(x, s, ToBeDeleted)) {
		fprintf(stderr, "ERROR on delete %s %d\n", s, ToBeDeleted);
		return 0;
	      }
	      else printf("deleted %s\n", s);
	      break;
	    }
	  }
	}
      }
    }
    printf("Enter key to remove : ");
  }
  printf("\n");
  SaveTransducer(x, argv[1], argv[2]);
  FreeTransducer(x);
  return 1;
}
