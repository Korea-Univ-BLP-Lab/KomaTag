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
/* 결과 생성 (음절 단위로 태깅된 것을 형태소 단위 결과로 변환) */
/* 리턴값 : 1 = 성공, 0 = 오류 */
int syllable2morpheme(char *result, int num_syl, char ej[][3], 
                      char tag_sequence[][MAX_TAG_LEN], char delimiter) {
  int i;

  char morph[MAX_WORD][MAX_WORD] = {0,}; /* 형태소 */
  char tags[MAX_WORD][MAX_TAG_LEN] = {0,};   /* 품사 */
  int num_morph;
  char tag_head;

  int start_time = 2;
  int end_time = num_syl+1;

  int tag_style = STYLE_ERROR;

  /* 초기화 */
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

  /* B나 E중에 아무것도 없을 경우 */
  if (tag_style == STYLE_ERROR) {
    return 0;
  }

  for (i = start_time; i <= end_time; i++) {
    tag_head = tag_sequence[i][0];

    switch (tag_head) {

    case 'S' :

      if (tag_style == B_STYLE) num_morph++;

      /* 음절 */
      if (ej[i][0] == FIL) strcat(morph[num_morph], &ej[i][1]);
      else strcat(morph[num_morph], ej[i]);

      /* 태그 */
      strcpy(tags[num_morph], &tag_sequence[i][2]);

      if (tag_style == E_STYLE) num_morph++;
      break;

    case 'B' :

      num_morph++; // 선증가

      /* 음절 */
      if (ej[i][0] == FIL) strcat(morph[num_morph], &ej[i][1]);
      else strcat(morph[num_morph], ej[i]);

      /* 태그 */
      strcpy(tags[num_morph], &tag_sequence[i][2]);
      break;
    
    case 'I' :

      /* 에러 체크 */
      if (tag_style == B_STYLE) {
        if (num_morph == -1) { /* 맨 처음에 I태그가 나온 경우 */
          return 0;
        }
        /* 앞 태그와 다른 경우 예) B-nc I-ef, I-nc I-ef */
        if (strcmp(tags[num_morph], &tag_sequence[i][2]) != 0) { 
          return 0;
        }
      }

      /* E_STYLE에 대한 에러 체크도 해야 함 */
      /* 맨 마지막에 I태그가 나온 경우 */
      /* 맨 처음 태그가 아니고, (앞 태그가 I태그이고, 앞 태그와 다른 경우) */

      /* 형태소 저장 */
      /* 태그는 저장할 필요없음 (앞의 것과 동일하므로) */
      if (ej[i][0] == FIL) strcat(morph[num_morph], &ej[i][1]); 
      else strcat(morph[num_morph], ej[i]);
      break;

    case 'E' :
      /* 음절 */
      if (ej[i][0] == FIL) strcat(morph[num_morph], &ej[i][1]);
      else strcat(morph[num_morph], ej[i]);

      /* 태그 */
      strcpy(tags[num_morph], &tag_sequence[i][2]);
      num_morph++; // 후증가
      break;
    }
  }

  /* 출력 */
  if (tag_style == E_STYLE) num_morph--; // 하나 감소시켜야 함

  {
    if (strlen(tags[0]) == 0) {
      ///**/fprintf(stderr, "빈태그발견\n");
      return 0; /* 빈 태그이면 에러 발생 */
    }

    sprintf(result, "%s%c%s", morph[0], delimiter, tags[0]); /* 첫번째 형태소와 태그 */

    for (i = 1; i <= num_morph; i++) {
      
      if (strlen(tags[i]) == 0) {
        ///**/fprintf(stderr, "빈태그발견\n");
        return 0; /* 빈 태그이면 에러 발생 */
      }
      sprintf(result+strlen(result), "+%s%c%s", morph[i], delimiter, tags[i]);
    }
  }
  return 1;
}

/******************************************************************************/
/* 생성된 태그열의 정당성 검사 */
/* tag_sequence는 음절 태그로 되어 있다. 예) B-nc */
int check_tag_sequence(char tag_sequence[][MAX_TAG_LEN], SEQ_STAGS &syl_tag_seq) {

  int num = 0;

  if (!syl_tag_seq.size()) return 0;

  for (int i = 0; i < (int) syl_tag_seq.size(); i++) {
    ///**/fprintf(stderr, "tag = %s\n", tag_sequence[i]);
    
    /* 찾아본다. */
    num = (int) count(syl_tag_seq[i].begin(), syl_tag_seq[i].end(), tag_sequence[i]);
    if (!num) {
      ///**/fprintf(stderr, "불가능한 조합\n");
      return 0; /* 없으면 */
    }
  } 
  ///**/fprintf(stderr, "\n");
  return 1;
}

/*****************************************************************************/
/* 어절 내의 형태소와 품사 태그를 알아낸다. */
/* str : 품사 부착된 형식의 입력 어절 */
/* morphs : 형태소 열 */
/* tags : 품사태그 열 */
/* spacing_tags : 띄어쓰기 태그 열 */
/* 리턴값 : 형태소 수 */
int morpheme2syllable(char *str, char morphs[][MAX_WORD], char tags[][MAX_WORD], 
                      int spacing_tags[MAX_WORD], char delimiter) {  

  char cur_char;
  int len; // 문자열의 길이
  int cur = 0;

  char tag_sub[MAX_WORD];
  int tag_i = 0;
  
  char morph_sub[MAX_WORD];
  int morph_i = 0;

  int num_morph = 0;

  len = (int) strlen(str); /* 문자열의 길이 */

  for (cur = 0; cur < len; cur++) {

    cur_char = str[cur]; /* 현재 글자 */
    morph_sub[morph_i++] = cur_char;

    if (cur_char == delimiter) { /* delimiter이면 */
      morph_sub[--morph_i] = (char) NULL;
      cur_char = str[++cur];
      
      if (cur_char == delimiter) { /* delimiter이면 */
        cur_char = str[++cur];
        morph_sub[morph_i++] = cur_char;
        morph_sub[morph_i] = (char) NULL;
      }
      
      /* 초기화 */
      tag_i = 0;
      morph_i = 0;
      
      while(cur_char != (char) NULL && cur_char != '+' && cur_char != '\n' && cur_char != ' ' && cur_char != '\t' && cur_char != ';') {
        tag_sub[tag_i++] = cur_char;
        cur++;
        cur_char = str[cur];
      }
      tag_sub[tag_i] = (char) NULL;

      strcpy(tags[num_morph], tag_sub); /* 품사 태그 */
      strcpy(morphs[num_morph], morph_sub); /* 형태소 */
      spacing_tags[num_morph] = 0; /* 띄어쓰기 태그 */ /* 일단 0을 만든다 */
      num_morph++;
      
    } /* end of if */
  }
  
  spacing_tags[num_morph-1] = 1; /* 마지막 형태소는 띄어쓰기 태그를 1로 만든다. */
  
  return num_morph;
}


/*****************************************************************************/
/* 어휘층 어절의 각 음절에 대응되는 태그를 결정 */
/* 결과는 syllable_tags에 저장 */
void get_syllable_tagging_result(int print_mode, int lexical_ej_len, 
                                 char *tags, char syllable_tags[][MAX_TAG_LEN]) {
  
  for (int j = 0; j < lexical_ej_len; j++) {  
    if ((print_mode == BIS || print_mode == IES) && lexical_ej_len == 1) {
      sprintf(syllable_tags[j], "S-%s", tags); /* 단독 */
      return;
    }
    
    /* 시작 */
    if ((print_mode == BI || print_mode == BIS) && !j) {
      sprintf(syllable_tags[j], "B-%s", tags);
    }
    
    /* 끝 */
    else if ((print_mode == IE || print_mode == IES) && j == lexical_ej_len-1) {
      sprintf(syllable_tags[j], "E-%s", tags); 
    }

    else {
      sprintf(syllable_tags[j], "I-%s", tags); /* 중간 */
    }
  } /* end of for */
}

/*****************************************************************************/
/* str : 형태소 분석 결과 */
/* 리턴값 : 어절내 형태소의 수 */
int check_morpheme_result(char *str, RESTORED_STAGS &str_syl_tag_seq, char delimiter) {

  int morph_num = 0; /* 어절 내의 형태소 수 */
  char morphs[MAX_WORD][MAX_WORD]; /* 형태소 */
  char tags[MAX_WORD][MAX_WORD]; /* 품사 태그 */

  int lexical_ej_len = 0; /* 어휘층 어절 길이 */
  
  char syllable_tags[MAX_WORD][MAX_TAG_LEN]; /* 표층 어절에 대한 음절 단위의 품사 태그 */
  int spacing_tags[MAX_WORD]; /* 띄어쓰기 태그 열 */

  /*****************************************************************************/
  /* 어절 내의 형태소와 품사 태그를 알아낸다. */
  morph_num = morpheme2syllable(str, morphs, tags, spacing_tags, delimiter);
  if (!morph_num) return 0;
  /*****************************************************************************/

  /* 어휘층 어절 및 음절 태그 구하기 */
  char lexical_ej_str[MAX_WORD] = {0,};
  {
    int num_char = 0;
    
    for (int i = 0; i < morph_num; i++) { /* 각 형태소에 대해 */
      num_char = (int) strlen(morphs[i])/2;
      strcat(lexical_ej_str, morphs[i]); /* 어휘층 어절 구하기 */

      /* 음절 단위 태깅 */
      get_syllable_tagging_result(BI, num_char, tags[i], &syllable_tags[lexical_ej_len]);

      lexical_ej_len += num_char; /* 어휘층 어절 길이 */
    }
  }

  ///**/fprintf(stderr, "어휘층어절 = %s\n", lexical_ej_str);
  if (!check_tag_sequence(syllable_tags, str_syl_tag_seq[lexical_ej_str])) {
    ///**/fprintf(stderr, "불가능한 조합 str = %s\n", str);
    return 0;
  }

  return morph_num;
}
