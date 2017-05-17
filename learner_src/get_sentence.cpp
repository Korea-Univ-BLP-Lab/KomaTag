#include "definitions.h"
#include "report.h"

#ifndef MAX_LINE_LENGTH
#define MAX_LINE_LENGTH 500
#endif

/******************************************************************************/
/* return value : ������ �ܾ�� */
int get_sentence(FILE *fp, char words[][MAX_WORD_LEN], char results[][MAX_RESULT_LEN]) {
  static int line_num = 0;

  int num = 0;
  char line[MAX_LINE_LENGTH];
  char *tag;

  /* ���� ���� */
  strcpy(results[0], BOSTAG_2);
  strcpy(results[1], BOSTAG_1);
  
  while (fgets(line, MAX_LINE_LENGTH, fp) != NULL) {
    if (strcmp (line, "\n") == 0) { // ������ ��
      return num;
    }
    else {
      line_num++;
      /* ���� ��Ȳ ǥ�� */
//      if ((line_num+1) % 2000 == 0) report(3, ".");
      if ((line_num+1) % 100000 == 0) 
        report(3, "\r%d of lines are processed..\n", line_num+1);

      line[strlen(line)-1] = 0;
      if ((tag = strchr(line, '\t')) == NULL) {
        error("Can't find TAB in %s\n", line);
        return -1;
      }
      *tag = 0;
      tag++;
      
      if (num+2 >= MAX_LINE) {
        error("Too many words in a sentence. The rest parts of the sentence will be ignored.\n");
        return num;
      }
      strcpy(words[num+2], line);
      strcpy(results[num+2], tag);
      num++;
    }
  }
  
  return num;
}

/*****************************************************************************/
/* ������ �о�� */
/* sentence[1]���� sentence[num_word]�� ����� */
int read_sentence (FILE *fp, char sentence[][MAX_WORD_LEN]) {
  char line[MAX_WORD_LEN];
  int num_word = 0;
  
  while (fgets (line, MAX_WORD_LEN, fp) != NULL) {
    if (strcmp (line, "\n") == 0) {     /* ������ �� */
      return (num_word);
    }
    else {
      line[strlen(line)-1] = 0;
      sentence[++num_word][0] = 0;
      strcpy(sentence[num_word], line);
      //sentence[num_word+1][0] = 0;
    }
  }
  return (num_word);
}

/******************************************************************************/
/* return value : ������ �ܾ�� */
// �Է� ���� ����
// ��      B-VA
// ��      I-VA
// ��      I-VA
// ��      B-ETM
//
// ��      B-MM
//
// ��      B-NNG
// ��      I-NNG

int get_sentence_by_syllable(FILE *fp, char syllables[][MAX_WORD_LEN], char tags[][MAX_TAG_LEN]) {
  static int line_num = 0;

  int num = 0;
  char line[MAX_LINE_LENGTH];
  char *tag;

  /* ���� ���� */
  strcpy(tags[0], BOW_TAG_2);
  strcpy(tags[1], BOW_TAG_1);
  strcpy(syllables[0], BOW_SYL_2);
  strcpy(syllables[1], BOW_SYL_1);
  
  while (fgets(line, MAX_LINE_LENGTH, fp) != NULL) {
    if (strcmp (line, "\n") == 0) { // ������ ��

      /* �ؿ��� num++�� �����Ƿ� num+2�� �´�. */
      strcpy(syllables[num+2], EOW_SYL); /* ���峡 ���� */
      strcpy(tags[num+2], EOW_TAG); /* ���峡 �±� */
      return num;
    }
    else {
      line_num++;
      /* ���� ��Ȳ ǥ�� */
//      if ((line_num+1) % 2000 == 0) fprintf(stderr, ".");
      if ((line_num+1) % 100000 == 0) 
        fprintf(stderr, "\r%d of lines are processed..\n", line_num+1);

      line[strlen(line)-1] = 0;
      if ((tag = strchr(line, '\t')) == NULL) {
        fprintf(stderr, "Can't find TAB in %s\n", line);
        return -1;
      }
      *tag = 0;
      tag++;
      
      if (num+2 >= MAX_LINE) {
        fprintf(stderr, "Too many syllables in a sentence. The rest parts of the sentence will be ignored.\n");
        return num;
      }
      strcpy(syllables[num+2], line);
      strcpy(tags[num+2], tag);
      num++; /* ���� */
    }
  }
  
  return num;
}
