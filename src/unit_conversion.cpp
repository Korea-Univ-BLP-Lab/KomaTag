#include "definitions.h"
#include "phonetic_change.h"
#include <algorithm> /* count */
#include "hsplit.h" /* FIL */

#define B_STYLE 1
#define E_STYLE 2
#define STYLE_ERROR 3

#define BI  1
#define BIS 2
#define IE  3
#define IES 4

/******************************************************************************/
/* ��� ���� (���� ������ �±�� ���� ���¼� ���� ����� ��ȯ) */
/* ���ϰ� : 1 = ����, 0 = ���� */
int syllable2morpheme(char *result, int num_syl, char ej[][3], 
                      char tag_sequence[][MAX_TAG_LEN], char delimiter) {
  int i;

  char morph[MAX_WORD][MAX_WORD] = {0,}; /* ���¼� */
  char tags[MAX_WORD][MAX_TAG_LEN] = {0,};   /* ǰ�� */
  int num_morph;
  char tag_head;

  int start_time = 2;
  int end_time = num_syl+1;

  int tag_style = STYLE_ERROR;

  /* �ʱ�ȭ */
  i = start_time;
  while (i <= end_time) {
    if (tag_sequence[i][0] == 'B') {
      num_morph = -1;
      tag_style = B_STYLE;
      break;
    }
    else if (tag_sequence[i][0] == 'E') {
      num_morph = 0;
      tag_style = E_STYLE;
      break;
    }
    i++;
  }

  /* B�� E�߿� �ƹ��͵� ���� ��� */
  if (tag_style == STYLE_ERROR) {
    return 0;
  }

  for (i = start_time; i <= end_time; i++) {
    tag_head = tag_sequence[i][0];

    switch (tag_head) {

    case 'S' :

      if (tag_style == B_STYLE) num_morph++;

      /* ���� */
      if (ej[i][0] == FIL) strcat(morph[num_morph], &ej[i][1]);
      else strcat(morph[num_morph], ej[i]);

      /* �±� */
      strcpy(tags[num_morph], &tag_sequence[i][2]);

      if (tag_style == E_STYLE) num_morph++;
      break;

    case 'B' :

      num_morph++; // ������

      /* ���� */
      if (ej[i][0] == FIL) strcat(morph[num_morph], &ej[i][1]);
      else strcat(morph[num_morph], ej[i]);

      /* �±� */
      strcpy(tags[num_morph], &tag_sequence[i][2]);
      break;
    
    case 'I' :

      /* ���� üũ */
      if (tag_style == B_STYLE) {
        if (num_morph == -1) { /* �� ó���� I�±װ� ���� ��� */
          return 0;
        }
        /* �� �±׿� �ٸ� ��� ��) B-nc I-ef, I-nc I-ef */
        if (strcmp(tags[num_morph], &tag_sequence[i][2]) != 0) { 
          return 0;
        }
      }

      /* E_STYLE�� ���� ���� üũ�� �ؾ� �� */
      /* �� �������� I�±װ� ���� ��� */
      /* �� ó�� �±װ� �ƴϰ�, (�� �±װ� I�±��̰�, �� �±׿� �ٸ� ���) */

      /* ���¼� ���� */
      /* �±״� ������ �ʿ���� (���� �Ͱ� �����ϹǷ�) */
      if (ej[i][0] == FIL) strcat(morph[num_morph], &ej[i][1]); 
      else strcat(morph[num_morph], ej[i]);
      break;

    case 'E' :
      /* ���� */
      if (ej[i][0] == FIL) strcat(morph[num_morph], &ej[i][1]);
      else strcat(morph[num_morph], ej[i]);

      /* �±� */
      strcpy(tags[num_morph], &tag_sequence[i][2]);
      num_morph++; // ������
      break;
    }
  }

  /* ��� */
  if (tag_style == E_STYLE) num_morph--; // �ϳ� ���ҽ��Ѿ� ��

  {
    if (strlen(tags[0]) == 0) {
      ///**/fprintf(stderr, "���±׹߰�\n");
      return 0; /* �� �±��̸� ���� �߻� */
    }

    sprintf(result, "%s%c%s", morph[0], delimiter, tags[0]); /* ù��° ���¼ҿ� �±� */

    for (i = 1; i <= num_morph; i++) {
      
      if (strlen(tags[i]) == 0) {
        ///**/fprintf(stderr, "���±׹߰�\n");
        return 0; /* �� �±��̸� ���� �߻� */
      }
      sprintf(result+strlen(result), "+%s%c%s", morph[i], delimiter, tags[i]);
    }
  }
  return 1;
}

/******************************************************************************/
/* ������ �±׿��� ���缺 �˻� */
/* tag_sequence�� ���� �±׷� �Ǿ� �ִ�. ��) B-nc */
int check_tag_sequence(char tag_sequence[][MAX_TAG_LEN], SEQ_STAGS &syl_tag_seq) {

  int num = 0;

  if (!syl_tag_seq.size()) return 0;

  for (int i = 0; i < (int) syl_tag_seq.size(); i++) {
    ///**/fprintf(stderr, "tag = %s\n", tag_sequence[i]);
    
    /* ã�ƺ���. */
    num = (int) count(syl_tag_seq[i].begin(), syl_tag_seq[i].end(), tag_sequence[i]);
    if (!num) {
      ///**/fprintf(stderr, "�Ұ����� ����\n");
      return 0; /* ������ */
    }
  } 
  ///**/fprintf(stderr, "\n");
  return 1;
}

/*****************************************************************************/
/* ���� ���� ���¼ҿ� ǰ�� �±׸� �˾Ƴ���. */
/* str : ǰ�� ������ ������ �Է� ���� */
/* morphs : ���¼� �� */
/* tags : ǰ���±� �� */
/* spacing_tags : ���� �±� �� */
/* ���ϰ� : ���¼� �� */
int morpheme2syllable(char *str, char morphs[][MAX_WORD], char tags[][MAX_WORD], 
                      int spacing_tags[MAX_WORD], char delimiter) {  

  char cur_char;
  int len; // ���ڿ��� ����
  int cur = 0;

  char tag_sub[MAX_WORD];
  int tag_i = 0;
  
  char morph_sub[MAX_WORD];
  int morph_i = 0;

  int num_morph = 0;

  len = (int) strlen(str); /* ���ڿ��� ���� */

  for (cur = 0; cur < len; cur++) {

    cur_char = str[cur]; /* ���� ���� */
    morph_sub[morph_i++] = cur_char;

    if (cur_char == delimiter) { /* delimiter�̸� */
      morph_sub[--morph_i] = (char) NULL;
      cur_char = str[++cur];
      
      if (cur_char == delimiter) { /* delimiter�̸� */
        cur_char = str[++cur];
        morph_sub[morph_i++] = cur_char;
        morph_sub[morph_i] = (char) NULL;
      }
      
      /* �ʱ�ȭ */
      tag_i = 0;
      morph_i = 0;
      
      while(cur_char != (char) NULL && cur_char != '+' && cur_char != '\n' && cur_char != ' ' && cur_char != '\t' && cur_char != ';') {
        tag_sub[tag_i++] = cur_char;
        cur++;
        cur_char = str[cur];
      }
      tag_sub[tag_i] = (char) NULL;

      strcpy(tags[num_morph], tag_sub); /* ǰ�� �±� */
      strcpy(morphs[num_morph], morph_sub); /* ���¼� */
      spacing_tags[num_morph] = 0; /* ���� �±� */ /* �ϴ� 0�� ����� */
      num_morph++;
      
    } /* end of if */
  }
  
  spacing_tags[num_morph-1] = 1; /* ������ ���¼Ҵ� ���� �±׸� 1�� �����. */
  
  return num_morph;
}


/*****************************************************************************/
/* ������ ������ �� ������ �����Ǵ� �±׸� ���� */
/* ����� syllable_tags�� ���� */
void get_syllable_tagging_result(int print_mode, int lexical_ej_len, 
                                 char *tags, char syllable_tags[][MAX_TAG_LEN]) {
  
  for (int j = 0; j < lexical_ej_len; j++) {  
    if ((print_mode == BIS || print_mode == IES) && lexical_ej_len == 1) {
      sprintf(syllable_tags[j], "S-%s", tags); /* �ܵ� */
      return;
    }
    
    /* ���� */
    if ((print_mode == BI || print_mode == BIS) && !j) {
      sprintf(syllable_tags[j], "B-%s", tags);
    }
    
    /* �� */
    else if ((print_mode == IE || print_mode == IES) && j == lexical_ej_len-1) {
      sprintf(syllable_tags[j], "E-%s", tags); 
    }

    else {
      sprintf(syllable_tags[j], "I-%s", tags); /* �߰� */
    }
  } /* end of for */
}

/*****************************************************************************/
/* str : ���¼� �м� ��� */
/* ���ϰ� : ������ ���¼��� �� */
int check_morpheme_result(char *str, RESTORED_STAGS &str_syl_tag_seq, char delimiter) {

  int morph_num = 0; /* ���� ���� ���¼� �� */
  char morphs[MAX_WORD][MAX_WORD]; /* ���¼� */
  char tags[MAX_WORD][MAX_WORD]; /* ǰ�� �±� */

  int lexical_ej_len = 0; /* ������ ���� ���� */
  
  char syllable_tags[MAX_WORD][MAX_TAG_LEN]; /* ǥ�� ������ ���� ���� ������ ǰ�� �±� */
  int spacing_tags[MAX_WORD]; /* ���� �±� �� */

  /*****************************************************************************/
  /* ���� ���� ���¼ҿ� ǰ�� �±׸� �˾Ƴ���. */
  morph_num = morpheme2syllable(str, morphs, tags, spacing_tags, delimiter);
  if (!morph_num) return 0;
  /*****************************************************************************/

  /* ������ ���� �� ���� �±� ���ϱ� */
  char lexical_ej_str[MAX_WORD] = {0,};
  {
    int num_char = 0;
    
    for (int i = 0; i < morph_num; i++) { /* �� ���¼ҿ� ���� */
      num_char = (int) strlen(morphs[i])/2;
      strcat(lexical_ej_str, morphs[i]); /* ������ ���� ���ϱ� */

      /* ���� ���� �±� */
      get_syllable_tagging_result(BI, num_char, tags[i], &syllable_tags[lexical_ej_len]);

      lexical_ej_len += num_char; /* ������ ���� ���� */
    }
  }

  ///**/fprintf(stderr, "���������� = %s\n", lexical_ej_str);
  if (!check_tag_sequence(syllable_tags, str_syl_tag_seq[lexical_ej_str])) {
    ///**/fprintf(stderr, "�Ұ����� ���� str = %s\n", str);
    return 0;
  }

  return morph_num;
}
