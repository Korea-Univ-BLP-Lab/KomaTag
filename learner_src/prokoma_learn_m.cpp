/* ǰ�� �°� (tagger)�� learner�� ������ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "definitions.h"
#include "probability_tool.h"

#define TRANSITION "TRANSITION.prb"
#define LEXICAL "LEXICAL.prb"

/*****************************************************************************/
/* return value : ������ ���¼� �� */
// ������ �Է�
static int get_eojeols(FILE *fp, char words[][200], char tags[][200]) {
  int num = 0;
  char line[500];
  char *tag;
  
  strcpy(tags[0], BOW_TAG_1); /* ���� ���� �±� */
  
  while (fgets(line, 500, fp) != NULL) {
    if (strcmp (line, "\n") == 0) { /* ������ �� */
      strcpy(tags[num+1], EOW_TAG);   /* ���� �� �±� */
      return num;
    }
    else {
      line[strlen(line)-1] = 0;
      if ((tag = strchr(line, '\t')) == NULL) {
        fprintf(stderr, "Can't find TAB in %s\n", line);
        return -1;
      }
      *tag = 0;
      tag++;
      
      num++;
      strcpy(words[num], line); /* �ܾ� */
      strcpy(tags[num], tag);   /* �±� */
    }
  }
  
  strcpy(tags[num+1], EOW_TAG);   /* ���� �� �±� */
  return num;
}

/*****************************************************************************/
int extract_frequency_m(FILE *fp,
					  C_FREQ &tag_unigram_freq, 
					  CC_FREQ &tag_bigram_freq,
					  C_FREQ &morph_freq, 
					  CC_FREQ &morph_tag_freq) {

  char words[200][200]; /* ���¼� */
  char tags[200][200];  /* �±� */
  
  int num_word;
  int i;
  
  while (1) {
    num_word = get_eojeols(fp, words, tags); /* ������ �Է� */
    
    if (num_word <= 0) return 1;
    
  
    for (i = 1; i <= num_word; i++) {
    
      /* for ���� Ȯ�� */
      morph_freq[words[i]]++; /* ���¼� */

      morph_tag_freq[words[i]][tags[i]]++; /* ���¼� �±� */

    }
      
    for (i = 1; i <= num_word+1; i++) { /* num_word+1 -> ���� �� �±� ���� */

      /* for ���� Ȯ�� */
      tag_bigram_freq[tags[i-1]][tags[i]]++; /* �±� bigram */
    }
    
    for (i = 0; i <= num_word; i++) {
      tag_unigram_freq[tags[i]]++; /* �±� unigram */
    }
  }
  
  return 1;
}


/******************************************************************************/
static int get_MLE_probability(C_FREQ &tag_unigram_freq, 
                        CC_FREQ &tag_bigram_freq,
                        C_FREQ &morph_freq, 
                        CC_FREQ &morph_tag_freq,
                        PROB_MAP &transition_prob, PROB_MAP &lexical_prob) {

  int N = 0;
  
  /* ���� Ȯ�� P(���¼� | ǰ��) */
  /* �ܾ ���� loop */
  for (C_FREQ::iterator word_itr = morph_freq.begin(); 
       word_itr != morph_freq.end(); ++word_itr)  {
    
    N += word_itr->second;
    
    /* �ܾ�-�±׿� ���� loop */
    for (C_FREQ::iterator tag_itr = morph_tag_freq[word_itr->first].begin(); 
	     tag_itr != morph_tag_freq[word_itr->first].end(); ++tag_itr) {
      
      /* �ܾ� �±� */
      ///**/fprintf(stdout, "%s %s -> %e\n", word_itr->first.c_str(), tag_itr->first.c_str(), (double)tag_itr->second/tag_unigram_freq[tag_itr->first]);
//      if (tag_itr->second > 1) { /* �� 2�̻��� ��쿡�� ���� */
        lexical_prob[word_itr->first][tag_itr->first] = (double)tag_itr->second/tag_unigram_freq[tag_itr->first]; /* HMM */
//      }
    }
  }
  
  /* ���� Ȯ�� */
  /* �±׿� ���� loop */
  for (CC_FREQ::iterator itr2 = tag_bigram_freq.begin(); itr2 != tag_bigram_freq.end(); ++itr2) {

    for (C_FREQ::iterator itr3 = tag_bigram_freq[itr2->first].begin(); 
      itr3 != tag_bigram_freq[itr2->first].end(); ++itr3) {

      /* t1t2 t3 */
      ///**/fprintf(stdout, "%s %s -> %e\n", itr2->first.c_str(), itr3->first.c_str(), (double)itr3->second/tag_unigram_freq[itr2->first]);
      if (itr3->second > 1) { /* �� 2�̻��� ��쿡�� ���� */
        transition_prob[itr2->first][itr3->first] = (double)itr3->second/tag_unigram_freq[itr2->first];
      }
    }
  }
  return 1;
}

/*****************************************************************************/
int prokoma_learn_m (char *filename) {
  FILE *fp;

  C_FREQ         tag_unigram_freq;
  CC_FREQ  tag_bigram_freq;

  C_FREQ        morph_freq;
  CC_FREQ    morph_tag_freq; 

  /* ȭ�� ���� */
  if ((fp = fopen(filename, "rt")) == NULL) {
    fprintf(stderr, "File open error : %s\n", filename);
    return 0;
  }

  fprintf(stderr, "Extracting information from [%s]..\n", filename);
  /************************************************************************/
  extract_frequency_m(fp, tag_unigram_freq, tag_bigram_freq, morph_freq, morph_tag_freq);
  /************************************************************************/

  /* ȭ�� �ݱ� */    
  fclose(fp);


  /* Ȯ�� ���� */  
  PROB_MAP transition_prob, lexical_prob;
  
  fprintf(stderr, "Estimating the probabilities.\n");
  get_MLE_probability(tag_unigram_freq, tag_bigram_freq, morph_freq, morph_tag_freq, transition_prob, lexical_prob);

  /* Ȯ�� ��� */
  fprintf(stderr, "Printing transition probabilities.\n");
  map_print_probability(TRANSITION, transition_prob, "t"); /* ���� Ȯ��*/

  fprintf(stderr, "Printing lexical probabilities.\n");
  map_print_probability(LEXICAL, lexical_prob, "t"); /* ���� Ȯ�� */

  return 1;
}
