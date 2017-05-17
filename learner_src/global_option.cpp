#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "getopt.h"
#include "report.h"
#include "definitions.h"

#include "global_option.h"
/******************************************************************************/

/******************************************************************************/
/* 전역 변수 */
char *Version = "("__DATE__")";
char *Description = "POS tagger learner with column format";

char *option_description[]={
  "-h : display help",
  "-v number[0-6] : verbosity [default = 2]",
  "-d character : delimiter [default = '/']",
  NULL,
};

globals_pt g;

extern int verbosity; /* default */ /* 0 errors only, 1 normal, ..., 6 really chatty */

/******************************************************************************/
void usage(void) {
  int i;

  report(0, "\n%s %s\n", Description, Version);
  report(0, "\n[Usage]");
  report(0, "\n%s OPTIONS input-FILE\n", g->cmd);
  report(0, "\n[OPTIONS]\n");

  for (i = 0; option_description[i]; i++) { 
    report(0, "  %s\n", option_description[i]); 
  }
  report(0, "\n");
}

/******************************************************************************/
/* 전역 변수를 위한 메모리 할당 및 값을 초기화 */
globals_pt init_globals(globals_pt old) {

  globals_pt g = (globals_pt)malloc(sizeof(globals_t));

  if (old) { memcpy(g, old, sizeof(globals_t)); return g; }

  g->cmd = NULL;
  g->delimiter = '/';
  
  return g;
}

/******************************************************************************/
/* 전역 변수에 대한 메모리 해제 및 화일 닫기 */
void free_globals(globals_pt old) {

  if (old) free(old); /* 메모리 해제 */
}

/******************************************************************************/
int get_options(globals_pt g, int argc, char **argv) {

  char c;

  while ( (c = getopt(argc, argv, "hv:d:")) != EOF) {
    switch (c) {
    case 'h':
      usage();
      exit(1);
      break;

    case 'v': // verbosity
      if (1 != sscanf(optarg, "%d", &verbosity)) {
        error("invalid verbosity \"%s\"\n", optarg);
      }
      break;
    case 'd': // delimiter
      //g->delimiter = '_';
      if (1 != sscanf(optarg, "%c", &g->delimiter)) {
        error("invalid delimiter \"%s\"\n", optarg);
      }
      break;
    } /* end of switch */
  }

//  /**/fprintf(stderr, "optind = %d, argc-1 = %d\n", optind, argc-1);

  if (optind > argc-1) { 
    usage(); 
    error("too few arguments\n"); 
    exit(1);
  }

  else if (optind < argc-1) { 
    usage(); 
    error("too many arguments\n"); 
    exit(1);
  }

  return (optind);
}


/******************************************************************************/
/* 아래부터는 수정할 필요없음 */
/******************************************************************************/
/*   successively returns tokens separated by one or more chars
   from sep string, not reentrant, temporarily modifies s
   typical use:
   for (t=tokenizer(b, " \t"); t; t=tokenizer(NULL, " \t"))
     { ... }  */
char *get_basename(char *name, char *s) {
#ifndef MAXPATHLEN
#define MAXPATHLEN 4096
#endif
  static char b[MAXPATHLEN];
  int tl, sl;
  char *t;
  
  t=strrchr(name, '/');

  if (!t) t=name; 
  else t++; 

  tl=strlen(t);

  if (tl+1 >= MAXPATHLEN) { 
    fprintf(stderr, "get_basename: \"%s\" too long\n", t); 
  }
  
  b[0] = '\0';
  strncat(b, t, MAXPATHLEN-1);

  if (s && tl>=(sl=strlen(s)) && !strcmp(t+tl-sl, s)) { 
    b[tl-sl]='\0'; 
  }
  return b;
}

/*****************************************************************************/
int print_progress (int percent, long filesize, FILE *fp) {
  long curpos;
  double newpercent;
  curpos = ftell (fp);
  newpercent = (double) curpos / filesize * 100;
  if ((int) newpercent > percent)
    fprintf (stderr, "\r%3d%% done..", (int) newpercent);
  return (int) newpercent;
}

/******************************************************************************/
