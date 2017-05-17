#include <stdio.h>
#include <string.h>
#include "hsplit.h"
#include "tool_pos_tagged_corpus.h"

/* ��� ��� */
#define BI    1 /* ����, �߰� */
#define BIS   2 /* ����, �߰�, �ܵ� */
#define IE    3 /* ����, �� */    
#define IES   4 /* ����, ��, �ܵ� */

#define SYLLABLE_TAGGING 1  /* ǰ�� ���� ����ġ -> ���� ���� �±� */
#define ALIGNMENT        2  /* ǰ�� ���� ����ġ -> ǥ�� ������ ������ ���� ���� (�������� ����) */


/*****************************************************************************/
/* ������ �м��ϴ� �Լ�  */
int ej_analysis_for_syllable_tagging(char *str, FILE *outfp, int print_mode, char delimiter) {
  int i;
  
  int morph_num = 0; /* ���� ���� ���¼� �� */
  char morphs[MAX_WORD][MAX_WORD]; /* ���¼� */
  char tags[MAX_WORD][MAX_WORD]; /* ǰ�� �±� */

  int lexical_ej_len = 0; /* ������ ���� ���� */
  char lexical_ej[MAX_WORD][3]; /* ������ ���� */
  
  char raw_ej[MAX_WORD]; /* ǥ�� ���� (����) */
  int surface_ej_len = 0; /* ǥ�� ���� ���� */
  char surface_ej[MAX_WORD][3]; /* ǥ�� ���� */
  
  char syllable_tag[MAX_WORD][30]; /* ǥ�� ������ ���� ���� ������ ǰ�� �±� */
  int spacing_tags[MAX_WORD]; /* ���� �±� �� */

  /*****************************************************************************/
  /* ���� ���� ���¼ҿ� ǰ�� �±׸� �˾Ƴ���. */
  if (!get_morphs_tags(str, raw_ej, &morph_num, morphs, tags, spacing_tags, delimiter)) return 1;
  /*****************************************************************************/

  /* ��� ��� */
  
  /* ������ ���� ���ϱ� */
  {
    int num_char = 0;
    for (i = 0; i < morph_num; i++) { /* �� ���¼ҿ� ���� */

      num_char = split_by_char(morphs[i], &lexical_ej[lexical_ej_len]); /* ���� ������ �ɰ��� */

      get_syllable_tagging_result(print_mode, &lexical_ej[lexical_ej_len], num_char, tags[i], &syllable_tag[lexical_ej_len]);
    
      lexical_ej_len += num_char;
    }
  }

  /*****************************************************************************/
  /* ǥ�� ���� */
  surface_ej_len = split_by_char(raw_ej, surface_ej); /* ���� ������ �ɰ��� */
    
  /* ������ ���� */
  for (i = 0; i < lexical_ej_len; i++) {
    fprintf(outfp, "%s\t%s\n", lexical_ej[i], syllable_tag[i]);
  }
  
  fprintf(outfp, "\n"); /* ������ ���϶� */
    
  return 1;
} 

/*****************************************************************************/
/* infile : ǰ�� ���� ����ġ */
/* outfile�� ����:
��      B-nc
��      I-nc

��      B-nc
��      I-nc
��      B-co
��      B-etm
*/
int syllable_tagging(char *infile, char *outfile, char delimiter) {
  int line_count = 0;
  char InputString[MAX_WORD];

  FILE *infp;
  FILE *outfp;

  if ((infp = fopen(infile, "rt")) == NULL) {
    fprintf(stderr, "File open error : %s\n", infile);
    return 0;
  }

  if ((outfp = fopen(outfile, "wt")) == NULL) {
    fprintf(stderr, "File open error : %s\n", outfile);
    return 0;
  }

  while (fgets(InputString, MAX_WORD, infp) != NULL) {
    ++line_count;

    /* �� ������ ���� ó�� */
    if (!ej_analysis_for_syllable_tagging(InputString, outfp, BI, delimiter)) {
      fprintf(stderr, "Error: �ùٸ��� ���� ���� [%d] : %s\n", line_count, InputString);
    }
  }

  fclose(infp);
  fclose(outfp);


  return 1;
}

