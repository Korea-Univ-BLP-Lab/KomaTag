#include <ctype.h>
#include "definitions.h"
#include "report.h"
#include "get_sentence.h"
#include "hsplit.h"
#include "tool_pos_tagged_corpus.h"
#include "probability_tool.h"

/******************************************************************************/
// for 품사 태거
// return value : 문장의 수
int extract_frequency_bigram_ej_by_ej(FILE *fp, 
                      C_FREQ &inter_tag_unigram_freq, 
                      CC_FREQ &inter_tag_bigram_freq,
                      char delimiter) {
	
  static char words[MAX_LINE][MAX_WORD_LEN];
  static char result[MAX_LINE][MAX_RESULT_LEN];
  
  int num_word; /* 문장내의 단어의 수 */
  int i;
  int num_sentence = 0; /* 문장의 수 */
  int start = 2;
  int end;

  /* 변수의 값을 계속 유지하기 위해서가 아니라  */
  /* 함수 호출시 매번 변수 생성에 걸리는 시간때문에 static으로 선언함 */
  static int morph_num = 0; /* 어절 내의 형태소 수 */
  static char morphs[MAX_WORD][MAX_WORD] = {0}; /* 형태소 열 */
  static char tags[MAX_WORD][MAX_WORD] = {0}; /* 품사 태그 열 */
  static int spacing_tags[MAX_WORD] = {0}; /* 띄어쓰기 태그 열 */

  while (1) {
	/* 문장을 입력 */
    num_word = get_sentence(fp, words, result);

    num_sentence++;

    if (num_word <= 0) {
      report(3, "\n");
      return num_sentence; /* 문장의 수 */
    }

    start = 2;
    end = num_word + 1;

//    string prev_tag = BOSTAG_1; // 문장 시작 태그
//    string cur_tag; // 현재 태그

    // 모든 어절에 대해
    for (i = start; i <= end; i++) {

      //cur_tag.clear(); // 초기화

      /* 어절 내의 형태소열과 태그열을 알아낸다. */
      // 0 ~ morph_num-1
//      get_morphs_tags(result[i], &morph_num, morphs, tags, spacing_tags, delimiter);

      //{
      //  // 현재 어절의 태그 (모든 품사를 결합)
      //  for (int j = 0; j < morph_num; j++) {
      //    
      //    if (j) cur_tag += "+";

      //    cur_tag += morphs[j];
      //    cur_tag += "/";
      //    cur_tag += tags[j];
      //  }
      //}

      ///**/fprintf(stderr, "cur_tag = %s\n", cur_tag.c_str());

      // 이전 어절 태그 -> 현 어절 태그
      inter_tag_bigram_freq[result[i-1]][result[i]]++; // 태그 bigram
      inter_tag_unigram_freq[result[i-1]]++; // 태그 unigram

      // dummy 이전 어절 태그 -> 현 어절 태그
      inter_tag_bigram_freq[BOW_TAG_1][result[i]]++; // 태그 bigram
      inter_tag_unigram_freq[BOW_TAG_1]++; // 태그 unigram


      ///**/fprintf(stdout, "%s -> %s\n", prev_tag.c_str(), cur_tag.c_str());
      ///**/fprintf(stdout, "%s -> %s\n", BOW_TAG_1, cur_tag.c_str());

      //prev_tag = cur_tag; // 이전 어절 태그 갱신

    } /* end of for */
  }
}


/******************************************************************************/
// for 품사 태거
// return value : 문장의 수
int extract_frequency_bigram_ej(FILE *fp, 
                      C_FREQ &inter_tag_unigram_freq, 
                      CC_FREQ &inter_tag_bigram_freq,
                      char delimiter) {
	
  static char words[MAX_LINE][MAX_WORD_LEN];
  static char result[MAX_LINE][MAX_RESULT_LEN];
  
  int num_word; /* 문장내의 단어의 수 */
  int i;
  int num_sentence = 0; /* 문장의 수 */
  int start = 2;
  int end;

  /* 변수의 값을 계속 유지하기 위해서가 아니라  */
  /* 함수 호출시 매번 변수 생성에 걸리는 시간때문에 static으로 선언함 */
  static int morph_num = 0; /* 어절 내의 형태소 수 */
  static char morphs[MAX_WORD][MAX_WORD] = {0}; /* 형태소 열 */
  static char tags[MAX_WORD][MAX_WORD] = {0}; /* 품사 태그 열 */
  static int spacing_tags[MAX_WORD] = {0}; /* 띄어쓰기 태그 열 */

  while (1) {
	/* 문장을 입력 */
    num_word = get_sentence(fp, words, result);

    num_sentence++;

    if (num_word <= 0) {
      report(3, "\n");
      return num_sentence; /* 문장의 수 */
    }

    start = 2;
    end = num_word + 1;

    string prev_tag = BOSTAG_1; // 문장 시작 태그
    string cur_tag; // 현재 태그

    // 모든 어절에 대해
    for (i = start; i <= end; i++) {

      cur_tag.clear(); // 초기화

      /* 어절 내의 형태소열과 태그열을 알아낸다. */
      // 0 ~ morph_num-1
      get_morphs_tags(result[i], &morph_num, morphs, tags, spacing_tags, delimiter);

      {
#ifdef USING_ALL_TAG
        // 현재 어절의 태그 (모든 품사를 결합)
        for (int j = 0; j < morph_num; j++) {
          cur_tag += tags[j];
          cur_tag += "|";
        }
#endif
#ifdef USING_HEAD_TAIL_TAG
        // 처음과 끝 태그의 결합
        cur_tag += tags[0];
        cur_tag += "|";
        cur_tag += tags[morph_num-1];
#endif
      }

#ifdef USING_FIRST_TAG
      // 이전 어절 태그 -> 현 어절 첫 태그
      inter_tag_bigram_freq[prev_tag][tags[0]]++; // 태그 bigram
      inter_tag_unigram_freq[prev_tag]++; // 태그 unigram

      // dummy 이전 어절 태그 -> 현 어절 첫 태그
      inter_tag_bigram_freq[BOW_TAG_1][tags[0]]++; // 태그 bigram
      inter_tag_unigram_freq[BOW_TAG_1]++; // 태그 unigram
#else
      // 이전 어절 태그 -> 현 어절 태그
      inter_tag_bigram_freq[prev_tag][cur_tag]++; // 태그 bigram
      inter_tag_unigram_freq[prev_tag]++; // 태그 unigram

      // dummy 이전 어절 태그 -> 현 어절 태그
      inter_tag_bigram_freq[BOW_TAG_1][cur_tag]++; // 태그 bigram
      inter_tag_unigram_freq[BOW_TAG_1]++; // 태그 unigram

#endif

      ///**/fprintf(stdout, "%s -> %s\n", prev_tag.c_str(), cur_tag.c_str());
      ///**/fprintf(stdout, "%s -> %s\n", BOW_TAG_1, cur_tag.c_str());

      prev_tag = cur_tag; // 이전 어절 태그 갱신

    } /* end of for */
  }
}

/******************************************************************************/
// for 품사 태거
// return value : 문장의 수
int extract_frequency_bigram(FILE *fp, 
                      C_FREQ &tag_unigram_freq, 
                      CC_FREQ &tag_bigram_freq,
                      C_FREQ &inter_tag_unigram_freq, 
                      CC_FREQ &inter_tag_bigram_freq,
                      char delimiter) {
	
  static char words[MAX_LINE][MAX_WORD_LEN];
  static char result[MAX_LINE][MAX_RESULT_LEN];
  
  int num_word; /* 문장내의 단어의 수 */
  int i;
  int num_sentence = 0; /* 문장의 수 */
  int start = 2;
  int end;

  /* 변수의 값을 계속 유지하기 위해서가 아니라  */
  /* 함수 호출시 매번 변수 생성에 걸리는 시간때문에 static으로 선언함 */
  static int morph_num = 0; /* 어절 내의 형태소 수 */
  static char morphs[MAX_WORD][MAX_WORD] = {0}; /* 형태소 열 */
  static char tags[MAX_WORD][MAX_WORD] = {0}; /* 품사 태그 열 */
  static int spacing_tags[MAX_WORD] = {0}; /* 띄어쓰기 태그 열 */

  while (1) {
	/* 문장을 입력 */
    num_word = get_sentence(fp, words, result);

    num_sentence++;

    if (num_word <= 0) {
      report(3, "\n");
      return num_sentence; /* 문장의 수 */
    }

    start = 2;
    end = num_word + 1;

    string prev_tag = BOSTAG_1; // 이전 어절의 마지막 품사 (문장 시작 태그)

    for (i = start; i <= end; i++) {

      /* 어절 내의 형태소열과 태그열을 알아낸다. */
      // 0 ~ morph_num-1
      get_morphs_tags(result[i], &morph_num, morphs, tags, spacing_tags, delimiter);

      /* 어절 시작 태그 -> 어절 처음 */
      // 어절내(intra-transition)
      tag_bigram_freq[BOW_TAG_1][tags[0]]++; // 태그 bigram
      tag_unigram_freq[BOW_TAG_1]++; // 태그 unigram

      /* 이전 어절 마지막 -> 현 어절 처음 */
      // 어절간(inter-transition)
      inter_tag_bigram_freq[prev_tag][tags[0]]++; // 태그 bigram
      inter_tag_unigram_freq[prev_tag]++; // 태그 unigram

      prev_tag = tags[morph_num-1];

    } /* end of for */
  }
}


/******************************************************************************/
// for 품사 태거
// return value : 문장의 수
int extract_frequency_trigram(FILE *fp, 
                      C_FREQ &tag_bigram_freq, 
                      CC_FREQ &tag_trigram_freq,
                      C_FREQ &inter_tag_bigram_freq, 
                      CC_FREQ &inter_tag_trigram_freq,
                      char delimiter) {
	
  static char words[MAX_LINE][MAX_WORD_LEN];
  static char result[MAX_LINE][MAX_RESULT_LEN];
  
  int num_word; /* 문장내의 단어의 수 */
  int i;
  int num_sentence = 0; /* 문장의 수 */
  int start = 2;
  int end;

  /* 변수의 값을 계속 유지하기 위해서가 아니라  */
  /* 함수 호출시 매번 변수 생성에 걸리는 시간때문에 static으로 선언함 */
  static int morph_num = 0; /* 어절 내의 형태소 수 */
  static char morphs[MAX_WORD][MAX_WORD] = {0}; /* 형태소 열 */
  static char tags[MAX_WORD][MAX_WORD] = {0}; /* 품사 태그 열 */
  static int spacing_tags[MAX_WORD] = {0}; /* 띄어쓰기 태그 열 */

  while (1) {
	/* 문장을 입력 */
    num_word = get_sentence(fp, words, result);

    num_sentence++;

    if (num_word <= 0) {
      report(3, "\n");
      return num_sentence; /* 문장의 수 */
    }

    start = 2;
    end = num_word + 1;

    string prev_tag = BOSTAG_1; // 문장 시작 태그
    string pprev_tag = BOSTAG_2;

    for (i = start; i <= end; i++) {

      /* 어절 내의 형태소열과 태그열을 알아낸다. */
      // 0 ~ morph_num-1
      get_morphs_tags(result[i], &morph_num, morphs, tags, spacing_tags, delimiter);

      /* 어절 시작 태그 -> 어절 처음 */
      // 어절내(intra-transition)
      string denom = BOW_TAG_2;
      denom += BOW_TAG_1;
      tag_trigram_freq[denom][tags[0]]++; // 태그 trigram // p-2 p-1 c0
      tag_bigram_freq[denom]++; // 태그 bigram

      ///**/fprintf(stderr, "intra[0] %s -> %s\n", denom.c_str(), tags[0]);

      denom = BOW_TAG_1;
      denom += tags[0];

      if (morph_num > 1) {
        tag_trigram_freq[denom][tags[1]]++; // 태그 trigram // p-1 c0 c1
      }
      else {
        tag_trigram_freq[denom][EOW_TAG]++; // 태그 trigram // p-1 c0 c1
      }
      tag_bigram_freq[denom]++; // 태그 bigram
     
      ///**/fprintf(stderr, "intra[1] %s -> %s\n", denom.c_str(), tags[1]);

      /* 이전 어절 마지막 -> 현 어절 처음 */
      // 어절간(inter-transition)
      denom = pprev_tag + prev_tag; // p-2 p-1
      denom += "->"; // 어절 전이 위치를 표시하기 위해
      inter_tag_trigram_freq[denom][tags[0]]++; // 태그 trigram // p-2 p-1 c0
      inter_tag_bigram_freq[denom]++; // 태그 bigram

      if (strstr(denom.c_str(), "풍습") != NULL) 
        fprintf(stderr, "\n풍습 found\n%s\n\n", result[i]);
      ///**/fprintf(stderr, "inter[0] %s -> %s\n", denom.c_str(), tags[0]);

      denom = prev_tag;
      denom += "->"; // 어절 전이 위치를 표시하기 위해
      denom += tags[0];

      if (morph_num > 1) {
        inter_tag_trigram_freq[denom][tags[1]]++; // 태그 trigram // p-1 c0 c1
      } 
      else {
        inter_tag_trigram_freq[denom][EOW_TAG]++; // 태그 trigram // p-1 c0 c1
      }
      inter_tag_bigram_freq[denom]++; // 태그 bigram

      ///**/fprintf(stderr, "inter[1] %s -> %s\n", denom.c_str(), tags[1]);

      if (morph_num > 1) pprev_tag = tags[morph_num-2];
      else pprev_tag = BOW_TAG_1;
      prev_tag = tags[morph_num-1];

    } /* end of for */
  }
}

/******************************************************************************/
/* syll : 음절 */
/* syll_type : 음절 타입 */
static int get_syllable_type(unsigned char *syll, char *syll_type) {

  if (syll[0] == FIL) {
    if (isalpha(syll[1])) { // 알파벳
      strcpy(syll_type, "alpha");
    }
    else if (isdigit(syll[1])) { // 숫자
      strcpy(syll_type, "digit");
    }
    else if (isascii(syll[1])) { // 기호
    strcpy(syll_type, "1symb");
    }
    else { // etc
      strcpy(syll_type, "1etc");
    }
  }
  
  else {
    if (isHanja(syll[0], syll[1])) { // 한자
      strcpy(syll_type, "hanja");
    }
    else if (isHangul(syll[0], syll[1])) { // 한글
      strcpy(syll_type, "hangul");
    }
    else if (is2Byte(syll[0], syll[1])) { // 2byte 기호
      strcpy(syll_type, "2symb");
    }
    else { // etc
      strcpy(syll_type, "2etc");
    }
  }
  return 1;
}

/******************************************************************************/
// for 음절 단위 형태소 분석
int extract_frequency_s(FILE *fp, C_FREQ &u_freq, C_FREQ &c_freq, 
                      CC_FREQ &uc_freq, CC_FREQ &cu_freq, 
                      CCC_FREQ &ucu_freq, CCC_FREQ &cuc_freq,
                      CCCC_FREQ &ucuc_freq, CCCC_FREQ &cucu_freq, 
                      CCCCC_FREQ &ucucu_freq, CCCCC_FREQ &cucuc_freq,
                      C_FREQ &c_type_freq, CC_FREQ &c_type_u_freq) {
	
  char syllables[MAX_LINE][MAX_WORD_LEN];
  char tags[MAX_LINE][MAX_TAG_LEN];
  
  int num_word; /* 문장내의 음절의 수 */
  int i;
  int num_sentence = 0; /* 문장의 수 */
  int start = 2;
  int end;

  while (1) {
	  /* 문장을 입력 (실제로는 어절) */
    num_word = get_sentence_by_syllable(fp, syllables, tags);

    num_sentence++;

    if (num_word <= 0) {
      fprintf(stderr, "\n");
      return num_sentence; /* 문장의 수 */
    }

    start = 2;
    end = num_word + 1;
    
    //for (i = start-1; i <= end; i++) {
    for (i = start; i <= end; i++) {
      u_freq[tags[i]]++; /* 태그 */
    }

    for (i = start; i <= end; i++) { /* end+1 -> 어절 끝 음절 포함 */
      c_freq[syllables[i]]++; /* 음절 */
    }

    for (i = start; i <= end+1; i++) {
      uc_freq[tags[i-1]][syllables[i]]++; /* 태그 음절 */
    }

    for (i = start; i <= end+1; i++) {
      cu_freq[syllables[i]][tags[i]]++; /* 음절 태그 */
    }

    for (i = start; i <= end+1; i++) {
      ucu_freq[tags[i-1]][syllables[i]][tags[i]]++; /* 태그 음절 태그 */
    }

    for (i = start; i <= end+1; i++) {
      cuc_freq[syllables[i-1]][tags[i-1]][syllables[i]]++; /* 음절 태그 음절*/
    }

    for (i = start; i <= end+1; i++) {
      ucuc_freq[tags[i-2]][syllables[i-1]][tags[i-1]][syllables[i]]++; /* 태그 음절 태그 음절 */
    }

    for (i = start; i <= end+1; i++) {
      cucu_freq[syllables[i-1]][tags[i-1]][syllables[i]][tags[i]]++; /* 음절 태그 음절 태그 */
    }

    for (i = start; i <= end+1; i++) {
      ucucu_freq[tags[i-2]][syllables[i-1]][tags[i-1]][syllables[i]][tags[i]]++; /* 태그 음절 태그 음절 태그 */
    }

    for (i = start; i <= end+1; i++) {
      cucuc_freq[syllables[i-2]][tags[i-2]][syllables[i-1]][tags[i-1]][syllables[i]]++; /* 음절 태그 음절 태그 음절 */
    }

    char syll_type[MAX_WORD];
    for (i = start; i <= end; i++) {
      get_syllable_type((unsigned char *)syllables[i], syll_type);
      c_type_freq[syll_type]++;
      c_type_u_freq[syll_type][tags[i]]++;
    }

  } // end of while
}
