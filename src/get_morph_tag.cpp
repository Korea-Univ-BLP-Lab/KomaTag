#include <string.h>
#include "hsplit.h"
#include "definitions.h"
#include "get_morph_tag.h"

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

  len = (int) strlen(str); /* ���ڿ��� ���� */
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

