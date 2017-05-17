#include <stdio.h>
#include <string.h>
#include "hsplit.h"
#include "definitions.h"
#include "probability_tool.h"
#include "tool_pos_tagged_corpus.h"
#include "entry2fst.h"

#define S_TRANSITION_LIST "S_TRANSITION.list" /* 엔트리 */
#define S_TRANSITION_INFO "S_TRANSITION.info" /* 정보 */
#define S_TRANSITION_FREQ "S_TRANSITION.freq" /* 빈도 (binary) */
#define S_TRANSITION_FST  "S_TRANSITION.FST"

/*****************************************************************************/
static int is_same(char surface_ej[][3], int surface_ej_len, 
            char lexical_ej[][3], int lexical_ej_len) {

  int i = 0, j = 0;

  if (surface_ej_len != lexical_ej_len) return 0; // 길이가 다르면
  
  for (; i < surface_ej_len && j < lexical_ej_len; i++, j++) {
    if (strcmp(surface_ej[i], lexical_ej[j]) != 0) // 다르면
      return 0;
  }

  if (i == surface_ej_len && j == lexical_ej_len) return 1; // 모든 문자열이 같고 마지막까지 비교가 되었으면
  else return 0;

  return 0;
}

/*****************************************************************************/
/* 표층 어절과 어휘층 어절 할당 */
/* 음운 현상 처리 */
int align_surface_lexical(SURFACE_FREQ &surface_freq, LEXICAL_FREQ &lexical_freq,
                          SURFACE_LEXCIAL_FREQ &surface_lexical_freq,
                          SURFACE_LEXCIAL_FREQ &surface_lexical_tag_freq,
                          char surface_ej[][3], int surface_ej_len, 
                          char lexical_ej[][3], int lexical_ej_len, char syllable_tag[][30]) {
  
  int i, j;
  
  int front_surface = 0; /* 불일치가 시작되는 부분 */
  int front_lexical = 0;
  
  int end_surface = surface_ej_len-1; /* 불일치가 끝나는 부분 */
  int end_lexical = lexical_ej_len-1;

  char surface_temp[MAX_WORD]; /* 표층 */
  char lexical_temp[MAX_WORD];/* 어휘층 */
  char surface_lexical_temp[MAX_WORD];/* 표층 + 어휘층 */
  char tag_temp[MAX_WORD]; /* 태그 */

  static int num = 0;

  num++;

  // 표층형과 어휘층형이 같으면
  if (is_same(surface_ej, surface_ej_len, lexical_ej, lexical_ej_len)) {
    for (i = 0; i < surface_ej_len; i++) {

      surface_freq[surface_ej[i]]++; // 표층
      lexical_freq[lexical_ej[i]]++; /* 어휘층 */
      surface_lexical_freq[surface_ej[i]][lexical_ej[i]]++; /* 표층 어휘층 */

      //fprintf(stdout, "%s\t%s\t%s\n", surface_ej[i], lexical_ej[i], syllable_tag[i]);
      sprintf(surface_lexical_temp, "%s%s%s", surface_ej[i], DELIM, lexical_ej[i]); // 표층_|어휘층
      
      surface_lexical_tag_freq[surface_lexical_temp][syllable_tag[i]]++; // 표층_|어휘층 태그

    }
    return 1;
  }

  // 음운현상이 나타났으면
  // 전방 탐색
  while(front_surface < surface_ej_len && front_lexical < lexical_ej_len) {
    if (strcmp(surface_ej[front_surface], lexical_ej[front_lexical]) == 0) {
      //fprintf(stdout, "%s\t%s\n", surface_ej[front_surface], lexical_ej[front_lexical]);
      front_surface++; front_lexical++;
    }
    else break;
  }
  
  if (front_surface == surface_ej_len) { // 끝까지 갔으면 한 음절 앞으로
    front_surface--; front_lexical--;
  }

  else {
    // 후방 탐색
    while(end_surface > front_surface && end_lexical > front_lexical) {
      if (strcmp(surface_ej[end_surface], lexical_ej[end_lexical]) == 0) {
        //fprintf(stdout, "%s\t%s\n", surface_ej[end_surface], lexical_ej[end_lexical]);
        end_surface--; end_lexical--;
      }
      else break;
    }
  }

  /* 출력부 */
  
  /* 앞부분 **************************************************/
  for (i = 0; i < front_surface; i++) {
    ///**/fprintf(stdout, "앞]%s\t%s\n", surface_ej[i], lexical_ej[i]);

    surface_freq[surface_ej[i]]++; /* 표층 */
    lexical_freq[lexical_ej[i]]++; /* 어휘층 */
    surface_lexical_freq[surface_ej[i]][lexical_ej[i]]++; /* 표층 어휘층 */

    //fprintf(stdout, "%s\t%s\t%s\n", surface_ej[i], lexical_ej[i], syllable_tag[i]);
    sprintf(surface_lexical_temp, "%s%s%s", surface_ej[i], DELIM, lexical_ej[i]);
    surface_lexical_tag_freq[surface_lexical_temp][syllable_tag[i]]++;
  }

  //**/fprintf(stdout, "front_surface = %d, end_surface = %d\n", front_surface, end_surface);
  //**/fprintf(stdout, "front_lexical = %d, end_lexical = %d\n", front_lexical, end_lexical);

  // NULL 방지
  if (front_surface > end_surface || front_lexical > end_lexical) {
    fprintf(stderr, "\nIn the [%d]th Ej., an unexpected morphological variation has been occurred\n", num);
    for (i = 0; i < surface_ej_len; i++) {
      fprintf(stderr, "%s", surface_ej[i]);
    }
    fprintf(stderr, "\n");
    return 1;
  }

  /* 중간부분 (음운 현상) **************************************************/
  surface_temp[0] = 0; /* 초기화 */
  for (i = front_surface; i <= end_surface; i++) {
    strcat(surface_temp, surface_ej[i]);
  }
  surface_freq[surface_temp]++; /* 표층 */
  
  lexical_temp[0] = 0; /* 초기화 */
  tag_temp[0] = 0; /* 초기화 */
  for (i = front_lexical; i <= end_lexical; i++) {
    if (i > front_lexical) strcat(tag_temp, "|"); /* 태그간 분리자 */
    strcat(lexical_temp, lexical_ej[i]);
    strcat(tag_temp, syllable_tag[i]); 
  }
  lexical_freq[lexical_temp]++; /* 어휘층 */
  
  ///**/fprintf(stdout, "중간]%s\t%s\n", surface_temp, lexical_temp);
  surface_lexical_freq[surface_temp][lexical_temp]++; /* 표층 어휘층 */
  
  //fprintf(stdout, "%s\t%s\t%s\n", surface_temp, lexical_temp, tag_temp);
  sprintf(surface_lexical_temp, "%s%s%s", surface_temp, DELIM, lexical_temp);
  surface_lexical_tag_freq[surface_lexical_temp][tag_temp]++;

  /* 끝부분 **************************************************/
  for (i = end_surface+1, j = end_lexical+1; i < surface_ej_len, j < lexical_ej_len; i++, j++) {
    ///**/fprintf(stdout, "뒤]%s\t%s\n", surface_ej[i], lexical_ej[j]);

    surface_freq[surface_ej[i]]++; /* 표층 */
    lexical_freq[lexical_ej[j]]++; /* 어휘층 */
    surface_lexical_freq[surface_ej[i]][lexical_ej[j]]++; /* 표층 어휘층 */

    //fprintf(stdout, "%s\t%s\t%s\n", surface_ej[i], lexical_ej[j], syllable_tag[j]);
    sprintf(surface_lexical_temp, "%s%s%s", surface_ej[i], DELIM, lexical_ej[j]);
    surface_lexical_tag_freq[surface_lexical_temp][syllable_tag[j]]++;
  }
  
  return 1;  
}

/*****************************************************************************/
/* 어휘층 어절 구하기 */
int get_lexical_ej(char lexical_ej[][3], char morphs[][MAX_WORD], int morph_num) {

  int num_char = 0;
  int len = 0;
  
  for (int i = 0; i < morph_num; i++) { /* 각 형태소에 대해 */

    num_char = split_by_char(morphs[i], &lexical_ej[len]); /* 문자 단위로 쪼갠다 */

    len += num_char;
  }
  
  return len; /* 어휘층 어절의 음절 수 */
}
    
/*****************************************************************************/
/* 어절을 분석하는 함수  */
int extract_phonetic_info(FILE *fp, FILE *s_transition_FP, 
                          SURFACE_FREQ &surface_freq, LEXICAL_FREQ &lexical_freq, 
                          SURFACE_LEXCIAL_FREQ &surface_lexical_freq,
                          SURFACE_LEXCIAL_FREQ &surface_lexical_tag_freq, char delimiter) {

  int morph_num = 0; /* 어절 내의 형태소 수 */
  char morphs[MAX_WORD][MAX_WORD]; /* 형태소 */
  char tags[MAX_WORD][MAX_WORD]; /* 품사 태그 */

  int lexical_ej_len = 0; /* 어휘층 어절 길이 */
  char lexical_ej[MAX_WORD][3]; /* 어휘층 어절 */
  
  char raw_ej[MAX_WORD]; /* 표층 어절 (원시) */
  int surface_ej_len = 0; /* 표층 어절 길이 */
  char surface_ej[MAX_WORD][3]; /* 표층 어절 */

  int line_count = 0;
  char InputString[MAX_WORD];
  char syllable_tag[MAX_WORD][30]; /* 어휘층 어절에 대한 음절 단위의 품사 태그 */
  int spacing_tags[MAX_WORD]; /* 띄어쓰기 태그 열 */

  /* 어절에 대해 반복 */
  while (fgets(InputString, MAX_WORD, fp) != NULL) {
    ++line_count;
    
    /* 초기화 */
    morph_num = 0;
    lexical_ej_len = 0;
    
    /*****************************************************************************/
    /* 어절 내의 형태소와 품사 태그를 알아낸다. */
    if (!get_morphs_tags(InputString, raw_ej, &morph_num, morphs, tags, spacing_tags, delimiter)) continue;
    /*****************************************************************************/
  
    /* 어휘층 어절과 음절 태그 구하기 */
    {
      int num_char = 0;
      for (int i = 0; i < morph_num; i++) { /* 각 형태소에 대해 */

        num_char = split_by_char(morphs[i], &lexical_ej[lexical_ej_len]); /* 문자 단위로 쪼갠다 */

        get_syllable_tagging_result(BI, &lexical_ej[lexical_ej_len], num_char, tags[i], &syllable_tag[lexical_ej_len]);
  
        lexical_ej_len += num_char;
      }
    }

    /**/
    // 태그 전이 정보 (형태통사 규칙 획득)
    {
      // 어절시작 태그, 첫번째 태그
      fprintf(s_transition_FP, "%s%s\t1\n", BOW_TAG_1, syllable_tag[0]); 

      // 이전 음절 태그, 현재 음절 태그
      // 두번째부터 마지막 태그까지
      for (int i = 1; i < lexical_ej_len; i++) {
        fprintf(s_transition_FP, "%s%s\t1\n", syllable_tag[i-1], syllable_tag[i]); 
      }
      
      // 마지막 태그, 어절끝 태그
      fprintf(s_transition_FP, "%s%s\t1\n", syllable_tag[lexical_ej_len-1], EOW_TAG); 
    }

    /* 표층 어절 */
    surface_ej_len = split_by_char(raw_ej, surface_ej); /* 문자 단위로 쪼갠다 */

    /* 결과 출력 */  
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

  /* 표층형에 대한 loop */
  for (SURFACE_FREQ::iterator word_itr = surface_freq.begin(); 
       word_itr != surface_freq.end(); ++word_itr)  {
    
    /* 단어-태그에 대한 loop */
    for (LEXICAL_FREQ::iterator tag_itr = surface_lexical_freq[word_itr->first].begin(); 
      tag_itr != surface_lexical_freq[word_itr->first].end(); ++tag_itr) {
      
      /* 표층 어휘층 */
      ///**/fprintf(stdout, "%s %s -> %e\n", word_itr->first.c_str(), tag_itr->first.c_str(), (double)tag_itr->second/surface_freq[word_itr->first]);
      
//      if (tag_itr->second > 1) { /* 빈도 2이상인 경우에만 저장 */
      if (tag_itr->second > 2) { /* 빈도 3이상인 경우에만 저장 */ 

        /* 단음절이고 표층과 어휘층이 같고 확률이 1인 경우에는 생략 */
        if (word_itr->first.size() == 2 && word_itr->first.compare(tag_itr->first) == 0 && 
            tag_itr->second == surface_freq[word_itr->first]) {
        }
        else phonetic_change_prob[word_itr->first][tag_itr->first] = (double)tag_itr->second/surface_freq[word_itr->first]; /* HMM */


        /* 음운 정보 */
        for (LEXICAL_FREQ::iterator itr = surface_lexical_tag_freq[word_itr->first+DELIM+tag_itr->first].begin();
          itr != surface_lexical_tag_freq[word_itr->first+DELIM+tag_itr->first].end(); ++itr) {

            //if (itr->second > 1) // 빈도 2 이상인 경우만 저장

          //fprintf(stdout, "%s%s%s\t%s\t%d\n", word_itr->first.c_str(), DELIM, tag_itr->first.c_str(), itr->first.c_str(), itr->second);
          phonetic_change_info[word_itr->first+DELIM+tag_itr->first][itr->first] = (double) itr->second; /* 표층/어휘층 + 태그 + 빈도(확률이 아님 -> 나중에 변경가능) */
        }
      }
    }
  }
  
  /* 총 token의 수(N)를 계산 */
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

/* infile : 품사 부착 말뭉치 */
/* outfile1 : 음운 정보 확률 */
/* outfile2 : 음운 정보 */
int phonetic_info(char *infile, char *outfile1, char *outfile2, char delimiter) {

  SURFACE_FREQ          surface_freq; /* 표층 음절(열) + 빈도 */
  LEXICAL_FREQ          lexical_freq; /* 어휘층 음절(열) + 빈도 */
  SURFACE_LEXCIAL_FREQ  surface_lexical_freq; /* 표층 음절(열) + 어휘층 음절(열) + 빈도 */
  SURFACE_LEXCIAL_FREQ  surface_lexical_tag_freq; /* 표층/어휘층 음절(열) + 태그(열) + 빈도 */

  FILE *infp;

  /* 파일 열기 */
  if ((infp = fopen(infile, "rt")) == NULL) {
    fprintf(stderr, "File open error : %s\n", infile);
    return 0;
  }

  FILE *s_transition_FP;

  if ((s_transition_FP = fopen(TEMP_OUTFILENAME, "wt")) == NULL) {
    fprintf(stderr, "File open error!\n");
  }

  fprintf(stderr, "\tExtracting information from [%s]...\n", infile);
  
  /* 정보 추출 */
  extract_phonetic_info(infp, s_transition_FP,
                        surface_freq, lexical_freq, surface_lexical_freq, 
                        surface_lexical_tag_freq, delimiter);

  /* 파일 닫기 */
  fclose(infp);
  fclose(s_transition_FP);

  // FST 만들기
  if (!entry2fst(TEMP_OUTFILENAME, S_TRANSITION_LIST, 
                  S_TRANSITION_FST, NULL, S_TRANSITION_INFO, S_TRANSITION_FREQ, 2, 1))  return 0;

  // 파일 삭제
  remove(TEMP_OUTFILENAME);

  /* 확률 추정 */  
  PROB_MAP phonetic_change_prob;
  PROB_MAP phonetic_change_info;

  fprintf(stderr, "Estimating the phonetic probabilities.\n");
  get_MLE_probability_phonetic(surface_freq, lexical_freq, surface_lexical_freq, 
                               surface_lexical_tag_freq,
                               phonetic_change_prob,
                               phonetic_change_info);

  /* 확률 출력 */
  fprintf(stderr, "Printing the phonetic probabilities.\n");
  map_print_probability(outfile1, phonetic_change_prob, "t");

  fprintf(stderr, "Printing the phonetic information.\n");
  map_print_probability(outfile2, phonetic_change_info, "t");
  
  return 1;
}
