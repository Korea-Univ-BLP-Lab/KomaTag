#include <stdio.h>
#include <string.h>
#include "hsplit.h"
#include "definitions.h"
#include "probability_tool.h"
#include "tool_pos_tagged_corpus.h"
#include "entry2fst.h"

#define S_TRANSITION_LIST "S_TRANSITION.list" /* ��Ʈ�� */
#define S_TRANSITION_INFO "S_TRANSITION.info" /* ���� */
#define S_TRANSITION_FREQ "S_TRANSITION.freq" /* �� (binary) */
#define S_TRANSITION_FST  "S_TRANSITION.FST"

/*****************************************************************************/
static int is_same(char surface_ej[][3], int surface_ej_len, 
            char lexical_ej[][3], int lexical_ej_len) {

  int i = 0, j = 0;

  if (surface_ej_len != lexical_ej_len) return 0; // ���̰� �ٸ���
  
  for (; i < surface_ej_len && j < lexical_ej_len; i++, j++) {
    if (strcmp(surface_ej[i], lexical_ej[j]) != 0) // �ٸ���
      return 0;
  }

  if (i == surface_ej_len && j == lexical_ej_len) return 1; // ��� ���ڿ��� ���� ���������� �񱳰� �Ǿ�����
  else return 0;

  return 0;
}

/*****************************************************************************/
/* ǥ�� ������ ������ ���� �Ҵ� */
/* ���� ���� ó�� */
int align_surface_lexical(SURFACE_FREQ &surface_freq, LEXICAL_FREQ &lexical_freq,
                          SURFACE_LEXCIAL_FREQ &surface_lexical_freq,
                          SURFACE_LEXCIAL_FREQ &surface_lexical_tag_freq,
                          char surface_ej[][3], int surface_ej_len, 
                          char lexical_ej[][3], int lexical_ej_len, char syllable_tag[][30]) {
  
  int i, j;
  
  int front_surface = 0; /* ����ġ�� ���۵Ǵ� �κ� */
  int front_lexical = 0;
  
  int end_surface = surface_ej_len-1; /* ����ġ�� ������ �κ� */
  int end_lexical = lexical_ej_len-1;

  char surface_temp[MAX_WORD]; /* ǥ�� */
  char lexical_temp[MAX_WORD];/* ������ */
  char surface_lexical_temp[MAX_WORD];/* ǥ�� + ������ */
  char tag_temp[MAX_WORD]; /* �±� */

  static int num = 0;

  num++;

  // ǥ������ ���������� ������
  if (is_same(surface_ej, surface_ej_len, lexical_ej, lexical_ej_len)) {
    for (i = 0; i < surface_ej_len; i++) {

      surface_freq[surface_ej[i]]++; // ǥ��
      lexical_freq[lexical_ej[i]]++; /* ������ */
      surface_lexical_freq[surface_ej[i]][lexical_ej[i]]++; /* ǥ�� ������ */

      //fprintf(stdout, "%s\t%s\t%s\n", surface_ej[i], lexical_ej[i], syllable_tag[i]);
      sprintf(surface_lexical_temp, "%s%s%s", surface_ej[i], DELIM, lexical_ej[i]); // ǥ��_|������
      
      surface_lexical_tag_freq[surface_lexical_temp][syllable_tag[i]]++; // ǥ��_|������ �±�

    }
    return 1;
  }

  // ���������� ��Ÿ������
  // ���� Ž��
  while(front_surface < surface_ej_len && front_lexical < lexical_ej_len) {
    if (strcmp(surface_ej[front_surface], lexical_ej[front_lexical]) == 0) {
      //fprintf(stdout, "%s\t%s\n", surface_ej[front_surface], lexical_ej[front_lexical]);
      front_surface++; front_lexical++;
    }
    else break;
  }
  
  if (front_surface == surface_ej_len) { // ������ ������ �� ���� ������
    front_surface--; front_lexical--;
  }

  else {
    // �Ĺ� Ž��
    while(end_surface > front_surface && end_lexical > front_lexical) {
      if (strcmp(surface_ej[end_surface], lexical_ej[end_lexical]) == 0) {
        //fprintf(stdout, "%s\t%s\n", surface_ej[end_surface], lexical_ej[end_lexical]);
        end_surface--; end_lexical--;
      }
      else break;
    }
  }

  /* ��º� */
  
  /* �պκ� **************************************************/
  for (i = 0; i < front_surface; i++) {
    ///**/fprintf(stdout, "��]%s\t%s\n", surface_ej[i], lexical_ej[i]);

    surface_freq[surface_ej[i]]++; /* ǥ�� */
    lexical_freq[lexical_ej[i]]++; /* ������ */
    surface_lexical_freq[surface_ej[i]][lexical_ej[i]]++; /* ǥ�� ������ */

    //fprintf(stdout, "%s\t%s\t%s\n", surface_ej[i], lexical_ej[i], syllable_tag[i]);
    sprintf(surface_lexical_temp, "%s%s%s", surface_ej[i], DELIM, lexical_ej[i]);
    surface_lexical_tag_freq[surface_lexical_temp][syllable_tag[i]]++;
  }

  //**/fprintf(stdout, "front_surface = %d, end_surface = %d\n", front_surface, end_surface);
  //**/fprintf(stdout, "front_lexical = %d, end_lexical = %d\n", front_lexical, end_lexical);

  // NULL ����
  if (front_surface > end_surface || front_lexical > end_lexical) {
    fprintf(stderr, "\nIn the [%d]th Ej., an unexpected morphological variation has been occurred\n", num);
    for (i = 0; i < surface_ej_len; i++) {
      fprintf(stderr, "%s", surface_ej[i]);
    }
    fprintf(stderr, "\n");
    return 1;
  }

  /* �߰��κ� (���� ����) **************************************************/
  surface_temp[0] = 0; /* �ʱ�ȭ */
  for (i = front_surface; i <= end_surface; i++) {
    strcat(surface_temp, surface_ej[i]);
  }
  surface_freq[surface_temp]++; /* ǥ�� */
  
  lexical_temp[0] = 0; /* �ʱ�ȭ */
  tag_temp[0] = 0; /* �ʱ�ȭ */
  for (i = front_lexical; i <= end_lexical; i++) {
    if (i > front_lexical) strcat(tag_temp, "|"); /* �±װ� �и��� */
    strcat(lexical_temp, lexical_ej[i]);
    strcat(tag_temp, syllable_tag[i]); 
  }
  lexical_freq[lexical_temp]++; /* ������ */
  
  ///**/fprintf(stdout, "�߰�]%s\t%s\n", surface_temp, lexical_temp);
  surface_lexical_freq[surface_temp][lexical_temp]++; /* ǥ�� ������ */
  
  //fprintf(stdout, "%s\t%s\t%s\n", surface_temp, lexical_temp, tag_temp);
  sprintf(surface_lexical_temp, "%s%s%s", surface_temp, DELIM, lexical_temp);
  surface_lexical_tag_freq[surface_lexical_temp][tag_temp]++;

  /* ���κ� **************************************************/
  for (i = end_surface+1, j = end_lexical+1; i < surface_ej_len, j < lexical_ej_len; i++, j++) {
    ///**/fprintf(stdout, "��]%s\t%s\n", surface_ej[i], lexical_ej[j]);

    surface_freq[surface_ej[i]]++; /* ǥ�� */
    lexical_freq[lexical_ej[j]]++; /* ������ */
    surface_lexical_freq[surface_ej[i]][lexical_ej[j]]++; /* ǥ�� ������ */

    //fprintf(stdout, "%s\t%s\t%s\n", surface_ej[i], lexical_ej[j], syllable_tag[j]);
    sprintf(surface_lexical_temp, "%s%s%s", surface_ej[i], DELIM, lexical_ej[j]);
    surface_lexical_tag_freq[surface_lexical_temp][syllable_tag[j]]++;
  }
  
  return 1;  
}

/*****************************************************************************/
/* ������ ���� ���ϱ� */
int get_lexical_ej(char lexical_ej[][3], char morphs[][MAX_WORD], int morph_num) {

  int num_char = 0;
  int len = 0;
  
  for (int i = 0; i < morph_num; i++) { /* �� ���¼ҿ� ���� */

    num_char = split_by_char(morphs[i], &lexical_ej[len]); /* ���� ������ �ɰ��� */

    len += num_char;
  }
  
  return len; /* ������ ������ ���� �� */
}
    
/*****************************************************************************/
/* ������ �м��ϴ� �Լ�  */
int extract_phonetic_info(FILE *fp, FILE *s_transition_FP, 
                          SURFACE_FREQ &surface_freq, LEXICAL_FREQ &lexical_freq, 
                          SURFACE_LEXCIAL_FREQ &surface_lexical_freq,
                          SURFACE_LEXCIAL_FREQ &surface_lexical_tag_freq, char delimiter) {

  int morph_num = 0; /* ���� ���� ���¼� �� */
  char morphs[MAX_WORD][MAX_WORD]; /* ���¼� */
  char tags[MAX_WORD][MAX_WORD]; /* ǰ�� �±� */

  int lexical_ej_len = 0; /* ������ ���� ���� */
  char lexical_ej[MAX_WORD][3]; /* ������ ���� */
  
  char raw_ej[MAX_WORD]; /* ǥ�� ���� (����) */
  int surface_ej_len = 0; /* ǥ�� ���� ���� */
  char surface_ej[MAX_WORD][3]; /* ǥ�� ���� */

  int line_count = 0;
  char InputString[MAX_WORD];
  char syllable_tag[MAX_WORD][30]; /* ������ ������ ���� ���� ������ ǰ�� �±� */
  int spacing_tags[MAX_WORD]; /* ���� �±� �� */

  /* ������ ���� �ݺ� */
  while (fgets(InputString, MAX_WORD, fp) != NULL) {
    ++line_count;
    
    /* �ʱ�ȭ */
    morph_num = 0;
    lexical_ej_len = 0;
    
    /*****************************************************************************/
    /* ���� ���� ���¼ҿ� ǰ�� �±׸� �˾Ƴ���. */
    if (!get_morphs_tags(InputString, raw_ej, &morph_num, morphs, tags, spacing_tags, delimiter)) continue;
    /*****************************************************************************/
  
    /* ������ ������ ���� �±� ���ϱ� */
    {
      int num_char = 0;
      for (int i = 0; i < morph_num; i++) { /* �� ���¼ҿ� ���� */

        num_char = split_by_char(morphs[i], &lexical_ej[lexical_ej_len]); /* ���� ������ �ɰ��� */

        get_syllable_tagging_result(BI, &lexical_ej[lexical_ej_len], num_char, tags[i], &syllable_tag[lexical_ej_len]);
  
        lexical_ej_len += num_char;
      }
    }

    /**/
    // �±� ���� ���� (������� ��Ģ ȹ��)
    {
      // �������� �±�, ù��° �±�
      fprintf(s_transition_FP, "%s%s\t1\n", BOW_TAG_1, syllable_tag[0]); 

      // ���� ���� �±�, ���� ���� �±�
      // �ι�°���� ������ �±ױ���
      for (int i = 1; i < lexical_ej_len; i++) {
        fprintf(s_transition_FP, "%s%s\t1\n", syllable_tag[i-1], syllable_tag[i]); 
      }
      
      // ������ �±�, ������ �±�
      fprintf(s_transition_FP, "%s%s\t1\n", syllable_tag[lexical_ej_len-1], EOW_TAG); 
    }

    /* ǥ�� ���� */
    surface_ej_len = split_by_char(raw_ej, surface_ej); /* ���� ������ �ɰ��� */

    /* ��� ��� */  
    align_surface_lexical(surface_freq, lexical_freq, surface_lexical_freq, surface_lexical_tag_freq,
                          surface_ej, surface_ej_len, lexical_ej, lexical_ej_len, syllable_tag);
  }
  return 1;
} 

/******************************************************************************/
int get_MLE_probability_phonetic(SURFACE_FREQ &surface_freq, LEXICAL_FREQ &lexical_freq,
                        SURFACE_LEXCIAL_FREQ &surface_lexical_freq,
                        SURFACE_LEXCIAL_FREQ &surface_lexical_tag_freq,
                        PROB_MAP &phonetic_change_prob,
                        PROB_MAP &phonetic_change_info) {

  /* ǥ������ ���� loop */
  for (SURFACE_FREQ::iterator word_itr = surface_freq.begin(); 
       word_itr != surface_freq.end(); ++word_itr)  {
    
    /* �ܾ�-�±׿� ���� loop */
    for (LEXICAL_FREQ::iterator tag_itr = surface_lexical_freq[word_itr->first].begin(); 
      tag_itr != surface_lexical_freq[word_itr->first].end(); ++tag_itr) {
      
      /* ǥ�� ������ */
      ///**/fprintf(stdout, "%s %s -> %e\n", word_itr->first.c_str(), tag_itr->first.c_str(), (double)tag_itr->second/surface_freq[word_itr->first]);
      
//      if (tag_itr->second > 1) { /* �� 2�̻��� ��쿡�� ���� */
      if (tag_itr->second > 2) { /* �� 3�̻��� ��쿡�� ���� */ 

        /* �������̰� ǥ���� �������� ���� Ȯ���� 1�� ��쿡�� ���� */
        if (word_itr->first.size() == 2 && word_itr->first.compare(tag_itr->first) == 0 && 
            tag_itr->second == surface_freq[word_itr->first]) {
        }
        else phonetic_change_prob[word_itr->first][tag_itr->first] = (double)tag_itr->second/surface_freq[word_itr->first]; /* HMM */


        /* ���� ���� */
        for (LEXICAL_FREQ::iterator itr = surface_lexical_tag_freq[word_itr->first+DELIM+tag_itr->first].begin();
          itr != surface_lexical_tag_freq[word_itr->first+DELIM+tag_itr->first].end(); ++itr) {

            //if (itr->second > 1) // �� 2 �̻��� ��츸 ����

          //fprintf(stdout, "%s%s%s\t%s\t%d\n", word_itr->first.c_str(), DELIM, tag_itr->first.c_str(), itr->first.c_str(), itr->second);
          phonetic_change_info[word_itr->first+DELIM+tag_itr->first][itr->first] = (double) itr->second; /* ǥ��/������ + �±� + ��(Ȯ���� �ƴ� -> ���߿� ���氡��) */
        }
      }
    }
  }
  
  /* �� token�� ��(N)�� ��� */
 /* int N = 0;
  for (LEXICAL_FREQ::iterator itr = lexical_freq.begin(); itr != lexical_freq.end(); ++itr) {
    N += itr->second;
  }
  */
  ///**/fprintf(stdout, "N = %d\n", N);
/*  for (LEXICAL_FREQ::iterator itr = lexical_freq.begin(); itr != lexical_freq.end(); ++itr) {
    phonetic_change_prob[DELIM][itr->first] = (double)itr->second / N;
  }
*/

  return 1;
}

/*****************************************************************************/
#define TEMP_OUTFILENAME "$$$$$.$$"

/* infile : ǰ�� ���� ����ġ */
/* outfile1 : ���� ���� Ȯ�� */
/* outfile2 : ���� ���� */
int phonetic_info(char *infile, char *outfile1, char *outfile2, char delimiter) {

  SURFACE_FREQ          surface_freq; /* ǥ�� ����(��) + �� */
  LEXICAL_FREQ          lexical_freq; /* ������ ����(��) + �� */
  SURFACE_LEXCIAL_FREQ  surface_lexical_freq; /* ǥ�� ����(��) + ������ ����(��) + �� */
  SURFACE_LEXCIAL_FREQ  surface_lexical_tag_freq; /* ǥ��/������ ����(��) + �±�(��) + �� */

  FILE *infp;

  /* ���� ���� */
  if ((infp = fopen(infile, "rt")) == NULL) {
    fprintf(stderr, "File open error : %s\n", infile);
    return 0;
  }

  FILE *s_transition_FP;

  if ((s_transition_FP = fopen(TEMP_OUTFILENAME, "wt")) == NULL) {
    fprintf(stderr, "File open error!\n");
  }

  fprintf(stderr, "\tExtracting information from [%s]...\n", infile);
  
  /* ���� ���� */
  extract_phonetic_info(infp, s_transition_FP,
                        surface_freq, lexical_freq, surface_lexical_freq, 
                        surface_lexical_tag_freq, delimiter);

  /* ���� �ݱ� */
  fclose(infp);
  fclose(s_transition_FP);

  // FST �����
  if (!entry2fst(TEMP_OUTFILENAME, S_TRANSITION_LIST, 
                  S_TRANSITION_FST, NULL, S_TRANSITION_INFO, S_TRANSITION_FREQ, 2, 1))  return 0;

  // ���� ����
  remove(TEMP_OUTFILENAME);

  /* Ȯ�� ���� */  
  PROB_MAP phonetic_change_prob;
  PROB_MAP phonetic_change_info;

  fprintf(stderr, "Estimating the phonetic probabilities.\n");
  get_MLE_probability_phonetic(surface_freq, lexical_freq, surface_lexical_freq, 
                               surface_lexical_tag_freq,
                               phonetic_change_prob,
                               phonetic_change_info);

  /* Ȯ�� ��� */
  fprintf(stderr, "Printing the phonetic probabilities.\n");
  map_print_probability(outfile1, phonetic_change_prob, "t");

  fprintf(stderr, "Printing the phonetic information.\n");
  map_print_probability(outfile2, phonetic_change_info, "t");
  
  return 1;
}
