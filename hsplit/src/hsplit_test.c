#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "hsplit.h"

/*****************************************************************************/
char *RunName = "hsplit";
char *Version = "(21/Aug/2003)";
char *Description = "hsplit library test program";

/*****************************************************************************/
void error(const char *fmt, ... ) {
  va_list args;

  va_start(args,fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
}

/*****************************************************************************/
void Help (void) {
    error("\nNAME");
    error("\n\t%s - %s %s", RunName, Description, Version);
    error("\n");
    
    error("\nSYNOPSIS");
    error("\n\t%s OPTION [<stdin] [>stdout]", RunName);
    error("\n");
    
    error("\nOPTION");
    error("\n\t-w\tsplit word by word types.");
    error("\n\t-c\tsplit word to characters.");

    error("\n\n");
}


/*****************************************************************************/
char *num2symbol(int d, char *str) {
  char *p;
  
  switch (d) {
    case T_ENG_UPPER: 
      strcpy(str, "영어(대)");
      break;
    case T_ENG_LOWER: 
      strcpy(str, "영어(소)");
      break;
    case T_HAN: 
      strcpy(str, "한글");
      break;
    case T_HJ:
      strcpy(str, "한자");
      break;
    case T_DIG:
      strcpy(str, "숫자");
      break;
    case T_SYM:
      strcpy(str, "기호");
      break;
    case T_2BSYM:
      strcpy(str, "2-byte 기호");
      break;
  }

    p = str;
    return p;
}     

/*****************************************************************************/
int split_to_words(void) {
  char line[MAX_WORD];
  word_type split_word[MAX_SPLIT];
  int num_split = 0;
  int i;
  char str[15];

  while (fscanf(stdin, "%s", line) != EOF) {
    num_split = split_by_word_type(split_word, line);
    for (i = 0; i < num_split; i++) {
      fprintf(stdout, "%s : %s\n", split_word[i].word, num2symbol(split_word[i].type, str));
    }
  }
  
  return 1;
}
/*****************************************************************************/
int split_to_characters(void) {
  char line[MAX_WORD];
  int num_splitchar;
  int i;
  char splitchar[MAX_WORD][3];
  
  while (fgets(line, MAX_WORD, stdin) != NULL) {
    
    line[strlen(line)-1] = 0;

    /* input_word를 각 문자별로 나눈다. */
    /* 나누어진 문자의 수는 num_splitchar에 저장된다. */
    num_splitchar = split_by_char_origin(line, splitchar);

    for (i = 0; i < num_splitchar; i++) {
      fprintf(stdout, "%s\n", splitchar[i]);
    }
    fprintf(stdout, "\n");
  }
    
  return 1;
}

/*****************************************************************************/
int main(int argc, char *argv[]) {

  if ( argc != 2) {Help (); return 0; }  
  if (strcmp (argv[1], "-w") == 0) split_to_words();
  else if (strcmp (argv[1], "-c") == 0) split_to_characters();
  
  return 1;
}
