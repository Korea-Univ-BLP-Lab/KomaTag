#include <ctype.h>
#include "definitions.h"
#include "report.h"
#include "get_sentence.h"
#include "hsplit.h"
#include "tool_pos_tagged_corpus.h"
#include "probability_tool.h"

/******************************************************************************/
// for ǰ�� �°�
// return value : ������ ��
int extract_frequency_bigram_ej_by_ej(FILE *fp, 
                      C_FREQ &inter_tag_unigram_freq, 
                      CC_FREQ &inter_tag_bigram_freq,
                      char delimiter) {
	
  static char words[MAX_LINE][MAX_WORD_LEN];
  static char result[MAX_LINE][MAX_RESULT_LEN];
  
  int num_word; /* ���峻�� �ܾ��� �� */
  int i;
  int num_sentence = 0; /* ������ �� */
  int start = 2;
  int end;

  /* ������ ���� ��� �����ϱ� ���ؼ��� �ƴ϶�  */
  /* �Լ� ȣ��� �Ź� ���� ������ �ɸ��� �ð������� static���� ������ */
  static int morph_num = 0; /* ���� ���� ���¼� �� */
  static char morphs[MAX_WORD][MAX_WORD] = {0}; /* ���¼� �� */
  static char tags[MAX_WORD][MAX_WORD] = {0}; /* ǰ�� �±� �� */
  static int spacing_tags[MAX_WORD] = {0}; /* ���� �±� �� */

  while (1) {
	/* ������ �Է� */
    num_word = get_sentence(fp, words, result);

    num_sentence++;

    if (num_word <= 0) {
      report(3, "\n");
      return num_sentence; /* ������ �� */
    }

    start = 2;
    end = num_word + 1;

//    string prev_tag = BOSTAG_1; // ���� ���� �±�
//    string cur_tag; // ���� �±�

    // ��� ������ ����
    for (i = start; i <= end; i++) {

      //cur_tag.clear(); // �ʱ�ȭ

      /* ���� ���� ���¼ҿ��� �±׿��� �˾Ƴ���. */
      // 0 ~ morph_num-1
//      get_morphs_tags(result[i], &morph_num, morphs, tags, spacing_tags, delimiter);

      //{
      //  // ���� ������ �±� (��� ǰ�縦 ����)
      //  for (int j = 0; j < morph_num; j++) {
      //    
      //    if (j) cur_tag += "+";

      //    cur_tag += morphs[j];
      //    cur_tag += "/";
      //    cur_tag += tags[j];
      //  }
      //}

      ///**/fprintf(stderr, "cur_tag = %s\n", cur_tag.c_str());

      // ���� ���� �±� -> �� ���� �±�
      inter_tag_bigram_freq[result[i-1]][result[i]]++; // �±� bigram
      inter_tag_unigram_freq[result[i-1]]++; // �±� unigram

      // dummy ���� ���� �±� -> �� ���� �±�
      inter_tag_bigram_freq[BOW_TAG_1][result[i]]++; // �±� bigram
      inter_tag_unigram_freq[BOW_TAG_1]++; // �±� unigram


      ///**/fprintf(stdout, "%s -> %s\n", prev_tag.c_str(), cur_tag.c_str());
      ///**/fprintf(stdout, "%s -> %s\n", BOW_TAG_1, cur_tag.c_str());

      //prev_tag = cur_tag; // ���� ���� �±� ����

    } /* end of for */
  }
}


/******************************************************************************/
// for ǰ�� �°�
// return value : ������ ��
int extract_frequency_bigram_ej(FILE *fp, 
                      C_FREQ &inter_tag_unigram_freq, 
                      CC_FREQ &inter_tag_bigram_freq,
                      char delimiter) {
	
  static char words[MAX_LINE][MAX_WORD_LEN];
  static char result[MAX_LINE][MAX_RESULT_LEN];
  
  int num_word; /* ���峻�� �ܾ��� �� */
  int i;
  int num_sentence = 0; /* ������ �� */
  int start = 2;
  int end;

  /* ������ ���� ��� �����ϱ� ���ؼ��� �ƴ϶�  */
  /* �Լ� ȣ��� �Ź� ���� ������ �ɸ��� �ð������� static���� ������ */
  static int morph_num = 0; /* ���� ���� ���¼� �� */
  static char morphs[MAX_WORD][MAX_WORD] = {0}; /* ���¼� �� */
  static char tags[MAX_WORD][MAX_WORD] = {0}; /* ǰ�� �±� �� */
  static int spacing_tags[MAX_WORD] = {0}; /* ���� �±� �� */

  while (1) {
	/* ������ �Է� */
    num_word = get_sentence(fp, words, result);

    num_sentence++;

    if (num_word <= 0) {
      report(3, "\n");
      return num_sentence; /* ������ �� */
    }

    start = 2;
    end = num_word + 1;

    string prev_tag = BOSTAG_1; // ���� ���� �±�
    string cur_tag; // ���� �±�

    // ��� ������ ����
    for (i = start; i <= end; i++) {

      cur_tag.clear(); // �ʱ�ȭ

      /* ���� ���� ���¼ҿ��� �±׿��� �˾Ƴ���. */
      // 0 ~ morph_num-1
      get_morphs_tags(result[i], &morph_num, morphs, tags, spacing_tags, delimiter);

      {
#ifdef USING_ALL_TAG
        // ���� ������ �±� (��� ǰ�縦 ����)
        for (int j = 0; j < morph_num; j++) {
          cur_tag += tags[j];
          cur_tag += "|";
        }
#endif
#ifdef USING_HEAD_TAIL_TAG
        // ó���� �� �±��� ����
        cur_tag += tags[0];
        cur_tag += "|";
        cur_tag += tags[morph_num-1];
#endif
      }

#ifdef USING_FIRST_TAG
      // ���� ���� �±� -> �� ���� ù �±�
      inter_tag_bigram_freq[prev_tag][tags[0]]++; // �±� bigram
      inter_tag_unigram_freq[prev_tag]++; // �±� unigram

      // dummy ���� ���� �±� -> �� ���� ù �±�
      inter_tag_bigram_freq[BOW_TAG_1][tags[0]]++; // �±� bigram
      inter_tag_unigram_freq[BOW_TAG_1]++; // �±� unigram
#else
      // ���� ���� �±� -> �� ���� �±�
      inter_tag_bigram_freq[prev_tag][cur_tag]++; // �±� bigram
      inter_tag_unigram_freq[prev_tag]++; // �±� unigram

      // dummy ���� ���� �±� -> �� ���� �±�
      inter_tag_bigram_freq[BOW_TAG_1][cur_tag]++; // �±� bigram
      inter_tag_unigram_freq[BOW_TAG_1]++; // �±� unigram

#endif

      ///**/fprintf(stdout, "%s -> %s\n", prev_tag.c_str(), cur_tag.c_str());
      ///**/fprintf(stdout, "%s -> %s\n", BOW_TAG_1, cur_tag.c_str());

      prev_tag = cur_tag; // ���� ���� �±� ����

    } /* end of for */
  }
}

/******************************************************************************/
// for ǰ�� �°�
// return value : ������ ��
int extract_frequency_bigram(FILE *fp, 
                      C_FREQ &tag_unigram_freq, 
                      CC_FREQ &tag_bigram_freq,
                      C_FREQ &inter_tag_unigram_freq, 
                      CC_FREQ &inter_tag_bigram_freq,
                      char delimiter) {
	
  static char words[MAX_LINE][MAX_WORD_LEN];
  static char result[MAX_LINE][MAX_RESULT_LEN];
  
  int num_word; /* ���峻�� �ܾ��� �� */
  int i;
  int num_sentence = 0; /* ������ �� */
  int start = 2;
  int end;

  /* ������ ���� ��� �����ϱ� ���ؼ��� �ƴ϶�  */
  /* �Լ� ȣ��� �Ź� ���� ������ �ɸ��� �ð������� static���� ������ */
  static int morph_num = 0; /* ���� ���� ���¼� �� */
  static char morphs[MAX_WORD][MAX_WORD] = {0}; /* ���¼� �� */
  static char tags[MAX_WORD][MAX_WORD] = {0}; /* ǰ�� �±� �� */
  static int spacing_tags[MAX_WORD] = {0}; /* ���� �±� �� */

  while (1) {
	/* ������ �Է� */
    num_word = get_sentence(fp, words, result);

    num_sentence++;

    if (num_word <= 0) {
      report(3, "\n");
      return num_sentence; /* ������ �� */
    }

    start = 2;
    end = num_word + 1;

    string prev_tag = BOSTAG_1; // ���� ������ ������ ǰ�� (���� ���� �±�)

    for (i = start; i <= end; i++) {

      /* ���� ���� ���¼ҿ��� �±׿��� �˾Ƴ���. */
      // 0 ~ morph_num-1
      get_morphs_tags(result[i], &morph_num, morphs, tags, spacing_tags, delimiter);

      /* ���� ���� �±� -> ���� ó�� */
      // ������(intra-transition)
      tag_bigram_freq[BOW_TAG_1][tags[0]]++; // �±� bigram
      tag_unigram_freq[BOW_TAG_1]++; // �±� unigram

      /* ���� ���� ������ -> �� ���� ó�� */
      // ������(inter-transition)
      inter_tag_bigram_freq[prev_tag][tags[0]]++; // �±� bigram
      inter_tag_unigram_freq[prev_tag]++; // �±� unigram

      prev_tag = tags[morph_num-1];

    } /* end of for */
  }
}


/******************************************************************************/
// for ǰ�� �°�
// return value : ������ ��
int extract_frequency_trigram(FILE *fp, 
                      C_FREQ &tag_bigram_freq, 
                      CC_FREQ &tag_trigram_freq,
                      C_FREQ &inter_tag_bigram_freq, 
                      CC_FREQ &inter_tag_trigram_freq,
                      char delimiter) {
	
  static char words[MAX_LINE][MAX_WORD_LEN];
  static char result[MAX_LINE][MAX_RESULT_LEN];
  
  int num_word; /* ���峻�� �ܾ��� �� */
  int i;
  int num_sentence = 0; /* ������ �� */
  int start = 2;
  int end;

  /* ������ ���� ��� �����ϱ� ���ؼ��� �ƴ϶�  */
  /* �Լ� ȣ��� �Ź� ���� ������ �ɸ��� �ð������� static���� ������ */
  static int morph_num = 0; /* ���� ���� ���¼� �� */
  static char morphs[MAX_WORD][MAX_WORD] = {0}; /* ���¼� �� */
  static char tags[MAX_WORD][MAX_WORD] = {0}; /* ǰ�� �±� �� */
  static int spacing_tags[MAX_WORD] = {0}; /* ���� �±� �� */

  while (1) {
	/* ������ �Է� */
    num_word = get_sentence(fp, words, result);

    num_sentence++;

    if (num_word <= 0) {
      report(3, "\n");
      return num_sentence; /* ������ �� */
    }

    start = 2;
    end = num_word + 1;

    string prev_tag = BOSTAG_1; // ���� ���� �±�
    string pprev_tag = BOSTAG_2;

    for (i = start; i <= end; i++) {

      /* ���� ���� ���¼ҿ��� �±׿��� �˾Ƴ���. */
      // 0 ~ morph_num-1
      get_morphs_tags(result[i], &morph_num, morphs, tags, spacing_tags, delimiter);

      /* ���� ���� �±� -> ���� ó�� */
      // ������(intra-transition)
      string denom = BOW_TAG_2;
      denom += BOW_TAG_1;
      tag_trigram_freq[denom][tags[0]]++; // �±� trigram // p-2 p-1 c0
      tag_bigram_freq[denom]++; // �±� bigram

      ///**/fprintf(stderr, "intra[0] %s -> %s\n", denom.c_str(), tags[0]);

      denom = BOW_TAG_1;
      denom += tags[0];

      if (morph_num > 1) {
        tag_trigram_freq[denom][tags[1]]++; // �±� trigram // p-1 c0 c1
      }
      else {
        tag_trigram_freq[denom][EOW_TAG]++; // �±� trigram // p-1 c0 c1
      }
      tag_bigram_freq[denom]++; // �±� bigram
     
      ///**/fprintf(stderr, "intra[1] %s -> %s\n", denom.c_str(), tags[1]);

      /* ���� ���� ������ -> �� ���� ó�� */
      // ������(inter-transition)
      denom = pprev_tag + prev_tag; // p-2 p-1
      denom += "->"; // ���� ���� ��ġ�� ǥ���ϱ� ����
      inter_tag_trigram_freq[denom][tags[0]]++; // �±� trigram // p-2 p-1 c0
      inter_tag_bigram_freq[denom]++; // �±� bigram

      if (strstr(denom.c_str(), "ǳ��") != NULL) 
        fprintf(stderr, "\nǳ�� found\n%s\n\n", result[i]);
      ///**/fprintf(stderr, "inter[0] %s -> %s\n", denom.c_str(), tags[0]);

      denom = prev_tag;
      denom += "->"; // ���� ���� ��ġ�� ǥ���ϱ� ����
      denom += tags[0];

      if (morph_num > 1) {
        inter_tag_trigram_freq[denom][tags[1]]++; // �±� trigram // p-1 c0 c1
      } 
      else {
        inter_tag_trigram_freq[denom][EOW_TAG]++; // �±� trigram // p-1 c0 c1
      }
      inter_tag_bigram_freq[denom]++; // �±� bigram

      ///**/fprintf(stderr, "inter[1] %s -> %s\n", denom.c_str(), tags[1]);

      if (morph_num > 1) pprev_tag = tags[morph_num-2];
      else pprev_tag = BOW_TAG_1;
      prev_tag = tags[morph_num-1];

    } /* end of for */
  }
}

/******************************************************************************/
/* syll : ���� */
/* syll_type : ���� Ÿ�� */
static int get_syllable_type(unsigned char *syll, char *syll_type) {

  if (syll[0] == FIL) {
    if (isalpha(syll[1])) { // ���ĺ�
      strcpy(syll_type, "alpha");
    }
    else if (isdigit(syll[1])) { // ����
      strcpy(syll_type, "digit");
    }
    else if (isascii(syll[1])) { // ��ȣ
    strcpy(syll_type, "1symb");
    }
    else { // etc
      strcpy(syll_type, "1etc");
    }
  }
  
  else {
    if (isHanja(syll[0], syll[1])) { // ����
      strcpy(syll_type, "hanja");
    }
    else if (isHangul(syll[0], syll[1])) { // �ѱ�
      strcpy(syll_type, "hangul");
    }
    else if (is2Byte(syll[0], syll[1])) { // 2byte ��ȣ
      strcpy(syll_type, "2symb");
    }
    else { // etc
      strcpy(syll_type, "2etc");
    }
  }
  return 1;
}

/******************************************************************************/
// for ���� ���� ���¼� �м�
int extract_frequency_s(FILE *fp, C_FREQ &u_freq, C_FREQ &c_freq, 
                      CC_FREQ &uc_freq, CC_FREQ &cu_freq, 
                      CCC_FREQ &ucu_freq, CCC_FREQ &cuc_freq,
                      CCCC_FREQ &ucuc_freq, CCCC_FREQ &cucu_freq, 
                      CCCCC_FREQ &ucucu_freq, CCCCC_FREQ &cucuc_freq,
                      C_FREQ &c_type_freq, CC_FREQ &c_type_u_freq) {
	
  char syllables[MAX_LINE][MAX_WORD_LEN];
  char tags[MAX_LINE][MAX_TAG_LEN];
  
  int num_word; /* ���峻�� ������ �� */
  int i;
  int num_sentence = 0; /* ������ �� */
  int start = 2;
  int end;

  while (1) {
	  /* ������ �Է� (�����δ� ����) */
    num_word = get_sentence_by_syllable(fp, syllables, tags);

    num_sentence++;

    if (num_word <= 0) {
      fprintf(stderr, "\n");
      return num_sentence; /* ������ �� */
    }

    start = 2;
    end = num_word + 1;
    
    //for (i = start-1; i <= end; i++) {
    for (i = start; i <= end; i++) {
      u_freq[tags[i]]++; /* �±� */
    }

    for (i = start; i <= end; i++) { /* end+1 -> ���� �� ���� ���� */
      c_freq[syllables[i]]++; /* ���� */
    }

    for (i = start; i <= end+1; i++) {
      uc_freq[tags[i-1]][syllables[i]]++; /* �±� ���� */
    }

    for (i = start; i <= end+1; i++) {
      cu_freq[syllables[i]][tags[i]]++; /* ���� �±� */
    }

    for (i = start; i <= end+1; i++) {
      ucu_freq[tags[i-1]][syllables[i]][tags[i]]++; /* �±� ���� �±� */
    }

    for (i = start; i <= end+1; i++) {
      cuc_freq[syllables[i-1]][tags[i-1]][syllables[i]]++; /* ���� �±� ����*/
    }

    for (i = start; i <= end+1; i++) {
      ucuc_freq[tags[i-2]][syllables[i-1]][tags[i-1]][syllables[i]]++; /* �±� ���� �±� ���� */
    }

    for (i = start; i <= end+1; i++) {
      cucu_freq[syllables[i-1]][tags[i-1]][syllables[i]][tags[i]]++; /* ���� �±� ���� �±� */
    }

    for (i = start; i <= end+1; i++) {
      ucucu_freq[tags[i-2]][syllables[i-1]][tags[i-1]][syllables[i]][tags[i]]++; /* �±� ���� �±� ���� �±� */
    }

    for (i = start; i <= end+1; i++) {
      cucuc_freq[syllables[i-2]][tags[i-2]][syllables[i-1]][tags[i-1]][syllables[i]]++; /* ���� �±� ���� �±� ���� */
    }

    char syll_type[MAX_WORD];
    for (i = start; i <= end; i++) {
      get_syllable_type((unsigned char *)syllables[i], syll_type);
      c_type_freq[syll_type]++;
      c_type_u_freq[syll_type][tags[i]]++;
    }

  } // end of while
}
