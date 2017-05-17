#include <stdio.h>
#include <string.h>
#include "hsplit.h"
#include "definitions.h"
#include "tool_pos_tagged_corpus.h"

/****************************************************************************/
/* ������ �м��ϴ� �Լ�  */
int ej_analysis_for_morpheme_tagging(char *str, FILE *outfp, char delimiter) {

  int i, j;
  int num_char = 0;

  char raw_ej[MAX_WORD]; /* ǥ�� ���� (����) */  
  int morph_num = 0; /* ���� ���� ���¼� �� */
  char morphs[MAX_WORD][MAX_WORD]; /* ���¼� �� */
  char tags[MAX_WORD][MAX_WORD]; /* ǰ�� �±� �� */
  int spacing_tags[MAX_WORD]; /* ���� �±� �� */
  
  char morphs_syllable_unit[MAX_WORD][3]; /* ���� ������ ���¼� */

  /*****************************************************************************/
  /* ���� ���� ���¼ҿ� ǰ�� �±׸� �˾Ƴ���. */
  /* �Է� (str) : ǰ�� ���� ����ġ ���� */
  if (!get_morphs_tags(str, raw_ej, &morph_num, morphs, tags, spacing_tags, delimiter)) return 1;

  ///**/fprintf(stderr, "morph_num = %d\n", morph_num);

  /* ��� ��� */
  /* ������ ���� ���ϱ� */
  for (i = 0; i < morph_num; i++) { /* �� ���¼ҿ� ���� */

    num_char = split_by_char(morphs[i], morphs_syllable_unit); /* ���� ������ �ɰ��� */
    
    for (j = 0; j < num_char; j++) { /* �� ������ ����Ѵ�. */
      fprintf(outfp, "%s", morphs_syllable_unit[j]);
      ///**/fprintf(stderr, "%s", morphs_syllable_unit[j]);
    }
    fprintf(outfp, "\t%s\n", tags[i]); /* ���¼� �±� */
    ///**/fprintf(stderr, "\t%s\n", tags[i]); /* ���¼� �±� */
  }
  
  fprintf(outfp, "\n"); /* ������ ���϶� */
  ///**/fprintf(stderr, "\n"); /* ������ ���϶� */

  return 1;
} 

/*****************************************************************************/
/* infile : �Է� ���ϸ� (ǰ�� ���� ����ġ) */
/* outfile : ��� ���ϸ� */
/* ��� ���� ���� :
  ���    nc

  ���    nc
  ��      co
  ��      etm
*/

int morpheme_tagging(char *infile, char *outfile, char delimiter) {

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
    if (!ej_analysis_for_morpheme_tagging(InputString, outfp, delimiter)) {
      fprintf(stderr, "Error: �ùٸ��� ���� ���� [%d] : %s\n", line_count, InputString);
    }
  }
  
  fclose(infp);
  fclose(outfp);

  return 1;
}
