#include <string.h>
#include "hsplit.h"
#include "definitions.h"
#include "tool_pos_tagged_corpus.h"

/*****************************************************************************/
/* ���� ���� ���¼ҿ� ǰ�� �±׸� �˾Ƴ���. */
/* str : ǰ�� ������ ������ �Է� ���� */
/* raw_ej : ǥ�� ���� */
/* morph_num : ���¼� �� */
/* morphs : ���¼� �� */
/* tags : ǰ���±� �� */
/* spacing_tags : ���� �±� �� */
int get_morphs_tags(char *str, char *raw_ej, int *morph_num, 
                    char morphs[][MAX_WORD], char tags[][MAX_WORD], int spacing_tags[MAX_WORD], char delimiter) {  

  char cur_char;
  int len; // ���ڿ��� ����
  int cur;

  char tag_sub[MAX_WORD];
  int tag_i;
  
  char morph_sub[MAX_WORD];
  int morph_i;

  len = strlen(str); /* ���ڿ��� ���� */
  if (len == 1) return 0; /* �ٿ� �ƹ� ���ڵ� ���� ���('\n'�� ���� ��) */

  ///**/fprintf(stderr, "str = %s\n", str);

  /* tab�� ���������� �ݺ� */ 
  /* ǥ������ ���� */
  for (cur = 0; str[cur] != '\t' && cur < len; cur++) {
    raw_ej[cur] = str[cur];
  } 
  raw_ej[cur] = 0;

  ///**/fprintf(stderr, "raw_ej = %s\n", raw_ej);
  
  if (cur == len) {
    return 0; 
  }
  else  cur++;

  /* �ʱ�ȭ */
  tag_i = 0;
  morph_i = 0;
  
  for (; cur < len; cur++) {
    cur_char = str[cur]; /* ���� ���� */
    morph_sub[morph_i++] = cur_char;

    if (cur_char == delimiter) { /* delimiter�̸� */
      morph_sub[--morph_i] = (char) NULL;
      cur_char = str[++cur];
      
      if (cur_char == delimiter) { /* delimiter�̸� */
        morph_sub[morph_i++] = cur_char;
        morph_sub[morph_i] = (char) NULL;
        cur_char = str[++cur];
      }
      
      /* �ʱ�ȭ */
      tag_i = 0;
      morph_i = 0;
      
      while(cur_char != '+' && cur_char != '\n' && cur_char != ' ' && cur_char != '\t' && cur_char != ';') {
        tag_sub[tag_i++] = cur_char;
        cur++;
        cur_char = str[cur];
      }
      tag_sub[tag_i] = (char) NULL;

      strcpy(tags[*morph_num], tag_sub); /* ǰ�� �±� */
      strcpy(morphs[*morph_num], morph_sub); /* ���¼� */
      spacing_tags[*morph_num] = 0; /* ���� �±� */ /* �ϴ� 0�� ����� */
      (*morph_num)++;
      
    } /* end of if */
  }
  
  spacing_tags[(*morph_num)-1] = 1; /* ������ ���¼Ҵ� ���� �±׸� 1�� �����. */
  
  return 1;
}


/*****************************************************************************/
/* ������ ������ �� ������ �����Ǵ� �±׸� ���� */
/* ����� syllable_tag�� ���� */
void get_syllable_tagging_result(int print_mode, char lexical_ej[][3], int lexical_ej_len, char *tags, char syllable_tag[][30]) {
  int j;
  
  for (j = 0; j < lexical_ej_len; j++) {  
    if ((print_mode == BIS || print_mode == IES) && lexical_ej_len == 1) {
      //fprintf(stdout, "%s\tS-%s\n", lexical_ej[j], tags); /* �ܵ� */
      sprintf(syllable_tag[j], "S-%s", tags); /* �ܵ� */
      return;
    }
    
    /* ���� */
    if ((print_mode == BI || print_mode == BIS) && !j) {
      //fprintf(stdout, "%s\tB-%s\n", lexical_ej[j], tags);
      sprintf(syllable_tag[j], "B-%s", tags);
    }
    
    /* �� */
    else if ((print_mode == IE || print_mode == IES) && j == lexical_ej_len-1) {
      //fprintf(stdout, "%s\tE-%s\n", lexical_ej[j], tags); 
      sprintf(syllable_tag[j], "E-%s", tags); 
    }

    else
      //fprintf(stdout, "%s\tI-%s\n", lexical_ej[j], tags); /* �߰� */
      sprintf(syllable_tag[j], "I-%s", tags); /* �߰� */
  }
}


/*****************************************************************************/
/* ���� ���� ���¼ҿ� ǰ�� �±׸� �˾Ƴ���. */
/* str : ǰ�� ������ ������ �Է� ���� */
/* morph_num : ���¼� �� */
/* morphs : ���¼� �� */
/* tags : ǰ���±� �� */
/* spacing_tags : ���� �±� �� */
int get_morphs_tags(char *str, int *morph_num, 
                    char morphs[][MAX_WORD], char tags[][MAX_WORD], int spacing_tags[MAX_WORD],
                    char delimiter) {

  char cur_char;
  int len; // ���ڿ��� ����
  int cur;

  char tag_sub[MAX_WORD];
  int tag_i = 0;
  
  char morph_sub[MAX_WORD];
  int morph_i = 0;

  len = strlen(str); /* ���ڿ��� ���� */
//  if (len == 1) return 0; /* �ٿ� �ƹ� ���ڵ� ���� ���('\n'�� ���� ��) */

  *morph_num = 0;

  for (cur = 0; cur < len; cur++) {
    cur_char = str[cur]; /* ���� ���� */
    morph_sub[morph_i++] = cur_char;

    if (cur_char == delimiter) { /* delimiter�̸� */
      morph_sub[--morph_i] = (char) NULL;
      cur_char = str[++cur];
      
      if (cur_char == delimiter) { /* delimiter�̸� */
        morph_sub[morph_i++] = cur_char;
        morph_sub[morph_i] = (char) NULL;
        cur_char = str[++cur];
      }
      
      /* �ʱ�ȭ */
      tag_i = 0;
      morph_i = 0;
      
      while(cur_char != 0 && cur_char != '+' && cur_char != '\n' && cur_char != ' ' && cur_char != '\t' && cur_char != ';') {
        tag_sub[tag_i++] = cur_char;
        cur++;
        cur_char = str[cur];
      }
      tag_sub[tag_i] = (char) NULL;

      strcpy(tags[*morph_num], tag_sub); /* ǰ�� �±� */
      strcpy(morphs[*morph_num], morph_sub); /* ���¼� */
      spacing_tags[*morph_num] = 0; /* ���� �±� */ /* �ϴ� 0�� ����� */
      (*morph_num)++;
      
    } /* end of if */
  } /* end of for */
  
  spacing_tags[(*morph_num)-1] = 1; /* ������ ���¼Ҵ� ���� �±׸� 1�� �����. */
  
  return 1;
}

