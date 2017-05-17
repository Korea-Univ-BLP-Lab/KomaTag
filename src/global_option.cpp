#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "getopt.h"

#include "definitions.h" // EOJEOL_ANALYSIS, ...
#include "report.h"
#include "global_option.h"

/******************************************************************************/
/* 전역 변수 */
char *Version = "("__DATE__")";
char *Description = "Korean morphological analyzer and POS tagger";

char *option_description[]={
  "-h : display help",
  "-v verbosity[0-6] [default = 2]",
  "-d path : resource directory [default = current directory]",
  "-m : run morphological analysis only",
  "     otherwise, run morphological analysis and POS tagging [default]",
  "-i input_file_style [default = t]",
  "     t : text without sentence breaking",
  "     rs : row format text with sentence breaking (one sentence per line)",
  "     cs : column format text with sentence breaking",
  "          (blank line in each sentence boundary)",
  "     m : morphological analyzed text without sentence breaking (KU style)",
  "     ms : morphological analyzed text with sentence breaking (KU style)",
  "          (blank line in each sentence boundary)",
  "-s : print the results with sentence breaking (by blank line)",
  "     otherwise, print the results without sentence breaking [default]",
  "-k : print the results with the KU style",
  "       ex) \"morph1/pos1+morph2/pos2+...\"",
  "     otherwise, print the results with the Sejong corpus style [default]",
  "       ex) \"morph1/pos1 + morph2/pos2 + ...\"",
  "-l delimiter : set the delimiter between morphemes and tags [default = '/']",
  "-u processing_unit_type [default = ems]",
  "     e : Eojeol-unit analysis",
  "     m : morpheme-unit analysis",
  "     s : syllable-unit analysis",
  "     em : Eojeol-unit + morpheme-unit analysis",
  "     es : Eojeol-unit + syllable-unit analysis",
  "     ms : morpheme-unit + syllable-unit analysis",
  "     ems : Eojeol-unit + morpheme-unit + syllable-unit analysis",
  "-x [1-20]: cutoff threshold for morpheme-unit analysis [default = 10]",
  "-y [1-20]: cutoff threshold for syllable-unit analysis [default = 10]",
  "-b [1-20] : beam size for syllable-unit analysis [default = 15]",
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

  while ( (c = getopt(argc, argv, "hv:d:mi:skl:u:x:y:b:")) != EOF) {
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
    case 'm': // run mode
      g->morphological_analysis_only = 1;
      break;

    case 'i':
      if (strcmp(optarg, "t") == 0) {
        g->run_sentence_breaking = 1;
      }
      else if (strcmp(optarg, "rs") == 0) {
        g->run_sentence_breaking = 0;
        g->column_format = 0;
      }
      else if (strcmp(optarg, "cs") == 0) {
        g->run_sentence_breaking = 0;
        g->column_format = 1;
      }
      else if (strcmp(optarg, "m") == 0) {
        g->tagging_only = 1;
        g->run_sentence_breaking = 1;
      }
      else if (strcmp(optarg, "ms") == 0) {
        g->tagging_only = 1;
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

    case 'l': // resource directory
      if (1 != sscanf(optarg, "%c", &g->delimiter)) {
        fprintf(stderr, "ERROR: invalid delimiter \"%s\"\n", optarg);
        exit(1);
      }
      break;

    case 'u':
      if (strcmp(optarg, "e") == 0) {
        g->processing_unit = EOJEOL_ANALYSIS;
      }
      else if (strcmp(optarg, "m") == 0) {
        g->processing_unit = MORPHEME_ANALYSIS;
      }
      else if (strcmp(optarg, "s") == 0) {
        g->processing_unit = SYLLABLE_ANALYSIS;
      }
      else if (strcmp(optarg, "em") == 0) {
        g->processing_unit = EOJEOL_ANALYSIS | MORPHEME_ANALYSIS;
      }
      else if (strcmp(optarg, "es") == 0) {
        g->processing_unit = EOJEOL_ANALYSIS | SYLLABLE_ANALYSIS;
      }
      else if (strcmp(optarg, "ms") == 0) {
        g->processing_unit = MORPHEME_ANALYSIS | SYLLABLE_ANALYSIS;
      }
      else if (strcmp(optarg, "ems") == 0) {
        g->processing_unit = EOJEOL_ANALYSIS | MORPHEME_ANALYSIS | SYLLABLE_ANALYSIS;
      }

      else {
        fprintf(stderr, "ERROR: invalid processing unit type \"%s\"\n", optarg);
        exit(1);
      }
      
      break;
      
    case 'x':
      if (1 != sscanf(optarg, "%d", &g->cutoff_threshold_m) || g->cutoff_threshold_m < 0) {
        fprintf(stderr, "ERROR: invalid value \"%s\"\n", optarg);
        exit(1);
      }
      break;

    case 'y':
      if (1 != sscanf(optarg, "%d", &g->cutoff_threshold_s) || g->cutoff_threshold_s < 0) {
        fprintf(stderr, "ERROR: invalid value \"%s\"\n", optarg);
        exit(1);
      }
      break;
    
    case 'b':
      if (1 != sscanf(optarg, "%d", &g->beam_size) || g->beam_size < 1) {
        fprintf(stderr, "ERROR: invalid value \"%s\"\n", optarg);
        exit(1);
      }
      break;
    default: exit(1);
    } /* end of switch */
  }

  if (g->morphological_analysis_only && g->tagging_only) {
    usage(); 
    fprintf(stderr, "ERROR: invalid arguments\n"); 
    exit(1);
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
  
  g->morphological_analysis_only = 0;
  g->tagging_only = 0;
  g->run_sentence_breaking = 1;
  g->column_format = 0;
  g->print_sentence_breaking = 0;
  g->ku_style = 0;
  g->delimiter = '/';
  g->processing_unit = EOJEOL_ANALYSIS | MORPHEME_ANALYSIS | SYLLABLE_ANALYSIS;
  g->cutoff_threshold_m = 10;
  g->cutoff_threshold_s = 10;
  g->beam_size = 15;
  return g;
}

/******************************************************************************/
void print_globals(globals_pt g) {

  fprintf(stderr, "\n[Options]\n");
  fprintf(stderr, "command name = \"%s\"\n", g->cmd);
  fprintf(stderr, "resource path = \"%s\"\n", g->rsc_path);
  fprintf(stderr, "morphological analysis only = %s\n", g->morphological_analysis_only ? "yes" : "no");
  fprintf(stderr, "run sentence breaking = %s\n", g->run_sentence_breaking ? "yes" : "no");
  fprintf(stderr, "column format text = %s\n", g->column_format ? "yes" : "no");
  fprintf(stderr, "print sentence breaking = %s\n", g->print_sentence_breaking ? "yes" : "no");
  fprintf(stderr, "print with the Korea University style = %s\n", g->ku_style ? "yes" : "no");
  fprintf(stderr, "delimiter = '%c'\n", g->delimiter);
  
  fprintf(stderr, "eojeol-unit processing = %s\n", g->processing_unit & EOJEOL_ANALYSIS ? "on" : "off");
  fprintf(stderr, "morpheme-unit processing = %s\n", g->processing_unit & MORPHEME_ANALYSIS ? "on" : "off");
  fprintf(stderr, "syllable-unit processing = %s\n", g->processing_unit & SYLLABLE_ANALYSIS ? "on" : "off");
  
  fprintf(stderr, "cutoff threshold for morpheme-unit analysis = %d\n", g->cutoff_threshold_m);
  fprintf(stderr, "cutoff threshold for syllable-unit analysis = %d\n", g->cutoff_threshold_s);
  fprintf(stderr, "beam size for syllable-unit analysis = %d\n", g->beam_size);
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

