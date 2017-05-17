#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "getopt.h"

#include "definitions.h" // EOJEOL_ANALYSIS, ...
#include "report.h"
#include "global_option-nbest.h"

/******************************************************************************/
/* 전역 변수 */
char *Version = "("__DATE__")";
char *Description = "N-Best Korean Part-of-Speech tagger";

char *option_description[]={
  "-h : display help",
  "-v verbosity[0-6] [default = 2]",
  "-d path : resource directory [default = current directory]",
  "-i input_file_style [default = m]",
  "     m : morphological analyzed text without sentence breaking (KU style)",
  "     ms : morphological analyzed text with sentence breaking (KU style)",
  "          (blank line in each sentence boundary)",
  "-s : print the results with sentence breaking (by blank line)",
  "     otherwise, print the results without sentence breaking [default]",
  "-k : print the results with the KU style",
  "       ex) \"morph1/pos1+morph2/pos2+...\"",
  "     otherwise, print the results with the Sejong corpus style [default]",
  "       ex) \"morph1/pos1 + morph2/pos2 + ...\"",
//  "-l delimiter : set the delimiter between morphemes and tags [default = '/']",
  "-c constraint type for N-best tagging [default = c]",
  "     c : consistency constraint between morphological analysis and tagging",
  "     a : c + absolute threshold constraint",
  "     r : c + relative threshold constraint",
  "     ar : c + a + r",
  "-r [1-20]: relative threshold value [default = 5]",
  "-a [1-1000]: absolute threshold value [default = 10]",
  NULL,
};

globals_pt g; /* 옵션 자료구조 */

/******************************************************************************/
void usage(void) {
  int i;

  //fprintf(stderr, "\n%s %s\n", Description, Version);
  fprintf(stderr, "\n[Usage]");
  fprintf(stderr, "\n%s [OPTIONS] Input-File(s)\n", g->cmd);
  fprintf(stderr, "%s [OPTIONS] [< stdin] [> stdout]\n", g->cmd);
  fprintf(stderr, "\n[OPTIONS]\n");

  for (i = 0; option_description[i]; i++) { 
    fprintf(stderr, "  %s\n", option_description[i]); 
  }
  fprintf(stderr, "\n");
}

/******************************************************************************/
int get_options(globals_pt g, int argc, char **argv) {

  char c;

  while ( (c = getopt(argc, argv, "hv:d:i:skl:c:r:a:")) != EOF) {
    switch (c) {
    case 'h':
      usage();
      exit(1);
      break;

    case 'v': // verbosity
      if (1 != sscanf(optarg, "%d", &verbosity)) {
        fprintf(stderr, "ERROR: invalid verbosity \"%s\"\n", optarg);
        exit(1);
      }
      break;

    case 'd': // resource directory
      if (1 != sscanf(optarg, "%s", g->rsc_path)) {
        fprintf(stderr, "ERROR: invalid path \"%s\"\n", optarg);
        exit(1);
      }
      break;

    case 'i':
      if (strcmp(optarg, "m") == 0) {
        g->run_sentence_breaking = 1;
      }
      else if (strcmp(optarg, "ms") == 0) {
        g->run_sentence_breaking = 0;
      }
      else {
        fprintf(stderr, "ERROR: invalid input file style \"%s\"\n", optarg);
        exit(1);
      }
      
      break;

    case 's':
      g->print_sentence_breaking = 1;
      break;

    case 'k':
      g->ku_style = 1;
      break;

    case 'l': // delimiter
      if (1 != sscanf(optarg, "%c", &g->delimiter)) {
        fprintf(stderr, "ERROR: invalid delimiter \"%s\"\n", optarg);
        exit(1);
      }
      break;

    case 'c':
      if (strcmp(optarg, "c") == 0) {
        g->constraint = CONSISTENCY_CONSTRAINT;
      }
      else if (strcmp(optarg, "a") == 0) {
        g->constraint = ABSOLUTE_MORPH_CONSTRAINT | CONSISTENCY_CONSTRAINT;
      }
      else if (strcmp(optarg, "r") == 0) {
        g->constraint = RELATIVE_MORPH_CONSTRAINT | CONSISTENCY_CONSTRAINT;
      }
      else if (strcmp(optarg, "ar") == 0) {
        g->constraint = ABSOLUTE_MORPH_CONSTRAINT | CONSISTENCY_CONSTRAINT | RELATIVE_MORPH_CONSTRAINT;
      }
      else {
        fprintf(stderr, "ERROR: invalid processing unit type \"%s\"\n", optarg);
        exit(1);
      }
      
      break;
    
    case 'r':
      if (1 != sscanf(optarg, "%d", &g->relative_threshold) || g->relative_threshold < 0) {
        fprintf(stderr, "ERROR: invalid value \"%s\"\n", optarg);
        exit(1);
      }
      break;
    
    case 'a':
      if (1 != sscanf(optarg, "%d", &g->absolute_threshold) || g->absolute_threshold < 0) {
        fprintf(stderr, "ERROR: invalid value \"%s\"\n", optarg);
        exit(1);
      }
      break;
    
    default: exit(1);
    } /* end of switch */
  }

  /**///fprintf(stderr, "argc = %d, optind = %d\n", argc, optind);
  if (argc < optind) {
    usage(); 
    fprintf(stderr, "ERROR: too few arguments\n"); 
    exit(1);
  }

  return optind;
}

/******************************************************************************/
/* 전역 변수를 위한 메모리 할당 및 값을 초기화 */
globals_pt init_globals(globals_pt old) {

  globals_pt g = (globals_pt)malloc(sizeof(globals_t));

  if (old) { memcpy(g, old, sizeof(globals_t)); return g; }

  g->cmd = NULL;
  g->rsc_path[0] = 0;
  
  g->run_sentence_breaking = 1;
  g->print_sentence_breaking = 0;
  
  g->ku_style = 0;
  
  g->delimiter = '/';
  
  g->constraint = CONSISTENCY_CONSTRAINT;
  g->relative_threshold = 5;
  g->absolute_threshold = 10;
  
  return g;
}

/******************************************************************************/
void print_globals(globals_pt g) {

  fprintf(stderr, "\n[Options]\n");
  fprintf(stderr, "command name = \"%s\"\n", g->cmd);
  fprintf(stderr, "resource path = \"%s\"\n", g->rsc_path);
  fprintf(stderr, "run sentence breaking = %s\n", g->run_sentence_breaking ? "yes" : "no");
  fprintf(stderr, "print sentence breaking = %s\n", g->print_sentence_breaking ? "yes" : "no");
  fprintf(stderr, "print with the Korea University style = %s\n", g->ku_style ? "yes" : "no");
  fprintf(stderr, "delimiter = '%c'\n", g->delimiter);
  fprintf(stderr, "consistency constraint = %s\n", g->constraint & CONSISTENCY_CONSTRAINT ? "on" : "off");
  fprintf(stderr, "absolute constraint = %s\n", g->constraint & ABSOLUTE_MORPH_CONSTRAINT ? "on" : "off");
  fprintf(stderr, "relative constraint = %s\n", g->constraint & RELATIVE_MORPH_CONSTRAINT ? "on" : "off");
  fprintf(stderr, "relative threshold = %d\n", g->relative_threshold);
  fprintf(stderr, "absolute threshold = %d\n", g->absolute_threshold);
  fprintf(stderr, "\n");
}

/******************************************************************************/
/* 전역 변수에 대한 메모리 해제 및 화일 닫기 */
void free_globals(globals_pt old) {
  if (old) free(old); /* 메모리 해제 */
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

  tl = (int)strlen(t);

  if (tl+1 >= MAXPATHLEN) { 
    fprintf(stderr, "get_basename: \"%s\" too long\n", t); 
  }
  
  b[0] = '\0';
  strncat(b, t, MAXPATHLEN-1);

  if (s && tl>=(sl=(int)strlen(s)) && !strcmp(t+tl-sl, s)) { 
    b[tl-sl]='\0'; 
  }
  return b;
}

