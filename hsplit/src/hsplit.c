#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "hsplit.h"

/****************************************************************************/
/* 주어진 문자열(source_str)을 1차 문자 배열로 된 문자열(target_str)로 변환 */
/* 1 byte 문자는 앞에 FIL을 유지 */
extern int convert_str(int num_char, char source_str[][3], char *target_str) {
  int i;
  int j = 0;
  
  target_str[0] = 0;
  
  for (i = 0; i < num_char; i++) {
    target_str[j++] = source_str[i][0];
    target_str[j++] = source_str[i][1];
  }
  target_str[j] = 0;
  
  return 1;
}

/****************************************************************************/
/* 주어진 문자열(source_str)을 1차 문자 배열로 된 문자열(target_str)로 변환 */
/* 1 byte 문자는 앞에 FIL을 제거 */
extern int convert_str_origin(int num_char, char source_str[][3], char *target_str) {
  int i;
  int j = 0;
  
  target_str[0] = 0;
  
  for (i = 0; i < num_char; i++) {

    if (source_str[i][0] != FIL && source_str[i][0] != FIL2) {
      target_str[j++] = source_str[i][0];
    }
    else i++;

    target_str[j++] = source_str[i][1];
  }

  target_str[j] = 0;
  
  return 1;
}

/****************************************************************************/
/* 주어진 문자열(source_str)을 문자열(target_str)로 변환 */
/* 1 byte 문자는 앞에 FIL을 제거 */
extern int convert_str_origin_array(char *source_str, char *target_str) {
  int i;
  int j = 0;
  int len;

  len = (int) strlen(source_str);

  for (i = 0; i < len; i++) {
    if (source_str[i] != FIL && source_str[i] != FIL2) {
      target_str[j++] = source_str[i];
    }
  }
  target_str[j] = 0;

  return 1;
}

/****************************************************************************/
short pushWord(word_type *sword, char *word, short type, short *count)
{
	if (*count < MAX_SPLIT) {
		strcpy(sword[*count].word, word);
		sword[*count].type = type;
		(*count)++;
		return 1;
	}
	
	return 0;
}

/****************************************************************************/
/* this function analyze a given string into several partable including Korean
   word-phrase, English string, digit string, punctuation, Chinese string,
   pictorial string, and so on. */
/* 완성형 */
short split_by_word_type (word_type *sword, char *hword) {
  char word[MAX_WORD];
  unsigned char* w;
  short start;
  short type;
  short len;
  short wi;
  short count=0;
  
  len = (short) strlen (hword);
  
  if(len >= MAX_WORD) {
  	 fprintf(stderr, "[Warning: Too long word] %s\n", hword);
  	 return 0;
  }
  
  wi = 0;
  w = (unsigned char*) hword;
  
  while (wi < len) {
    start = wi;
    if (isascii(w[wi])) { /* 아스키 코드인가? */
      if (isupper(w[wi])) { /* 알파벳인가? */
        /* foreign language - english */
        while (wi < len && isupper (w[wi])) wi++;
        strncpy (word, hword+start, wi-start);
        word[wi-start] = (char)NULL;
        type = T_ENG_UPPER; /* 외국어 대문자 */
      } 
      else if (islower(w[wi])) { /* 알파벳인가? */
        /* foreign language - english */
        while (wi < len && islower (w[wi])) wi++;
        strncpy (word, hword+start, wi-start);
        word[wi-start] = (char)NULL;
        type = T_ENG_LOWER; /* 외국어 소문자 */
      } 
      else if (isdigit (w[wi])) { /* 숫자인가? */
        /* digit */
        while (wi < len && isdigit (w[wi])) wi++;
        strncpy (word, hword+start, wi-start);
        word[wi-start] = (char)NULL;
        type = T_DIG; /* 수사 */
      } 
      else if (w[wi] == FIL) { /* FIL 인가? */
        while (wi < len && w[wi] == FIL) wi++;
        strncpy (word, hword+start, wi-start);
        word[wi-start] = (char)NULL;
        type = T_FIL; /* 수사 */
      } 
      else { /* 그밖의 문자 */
        while (wi < len && isascii(w[wi]) && !isdigit(w[wi]) && !isalpha(w[wi]) ) wi++;
        strncpy (word, hword+start, wi-start);
        word [wi-start] = (char)NULL;
        type = T_SYM;      /* 기호 */
      } 
    } 
    else { /* 한글 또는 한자 또는 2바이트 문자 */
      if (isHangul(w[wi], w[wi+1])) { /* 한글인가? */
        /* korean */
        while (wi < len && isHangul(w[wi], w[wi+1])) { 
          wi += 2;
        }
        {
          strncpy (word, hword+start, wi-start);
          word [wi-start] = (char)NULL;
          type = T_HAN; /* 한글 */
        }
      } 
      else if (isHanja(w[wi], w[wi+1])) { /* 한자인가? */
        /* hanja */
        while (wi < len && isHanja(w[wi], w[wi+1])) wi += 2;
        strncpy (word, hword+start, wi-start);
        word [wi-start] = (char)NULL;
        type = T_HJ; /* 자립 명사 */
      } 
      else { /* 2바이트 문자 */
        word [0] = hword [wi++];
        word [1] = hword [wi++];
        word [2] = (char)NULL;
        type = T_2BSYM; /* 2 byte 기호 */
      }
    }
    
    /* 조각난 단어 및 유형 저장 */
    if (!pushWord(sword, word, type, &count)) return 0;
  }
  
  return count;
}

/****************************************************************************/
/* 주어진 문자열을 각 문자별로 나눈다. */
/* hword = 입력 문자열 */
/* splitchar = 결과 저장 */
/* 1 byte 문자는 앞에 FIL을 붙여서 2byte로 만든다. */
/* return value : 분리된 문자의 수 */
int split_by_char (char *hword, char splitchar[][3]) {
  unsigned char* w = (unsigned char*) hword;
  short len = (short) strlen (hword);
  short wi = 0;
  int num_splitchar = 0;
  
  while (wi < len) {
    if (isascii(w[wi])) { /* 아스키 코드인가? */
      splitchar[num_splitchar][0] = FIL;
      splitchar[num_splitchar][1] = w[wi++];
      splitchar[num_splitchar++][2] = (char) NULL;
    } 
    else { /* 한글 또는 한자 또는 2바이트 문자 */
      splitchar[num_splitchar][0] = w[wi++];
      splitchar[num_splitchar][1] = w[wi++];
      splitchar[num_splitchar++][2] = (char) NULL;
    }
  }
  
  return num_splitchar;
}

/****************************************************************************/
/* 주어진 문자열을 각 문자별로 나눈다. */
/* hword = 입력 문자열 */
/* splitchar = 결과 저장 */
/* 1 byte 문자는 앞에 FIL을 제거 */
/* return value : 분리된 문자의 수 */
int split_by_char_origin (char *hword, char splitchar[][3]) {
  unsigned char* w = (unsigned char*) hword;
  short len = (short) strlen (hword);
  short wi = 0;
  int num_splitchar = 0;
  
  while (wi < len) {
    if (isascii(w[wi])) { /* 아스키 코드인가? */
      splitchar[num_splitchar][0] = w[wi++];
      splitchar[num_splitchar++][1] = (char) NULL;
    } 
    else { /* 한글 또는 한자 또는 2바이트 문자 */
      splitchar[num_splitchar][0] = w[wi++];
      splitchar[num_splitchar][1] = w[wi++];
      splitchar[num_splitchar++][2] = (char) NULL;
    }
  }
  
  return num_splitchar;
}

/****************************************************************************/
/* 주어진 문자열을 각 문자별로 나눈다. */
/* hword = 입력 문자열 */
/* splitchar = 결과 저장 */
/* 1 byte 문자는 앞에 FIL을 붙여서 2byte로 만든다. */
/* return value : 성공 = 1 */
int split_by_char_array (char *hword, char *splitchar) {
  unsigned char* w = (unsigned char*) hword;
  short len = (short) strlen (hword);
  short wi = 0;
  int i = 0;
  
  while (wi < len) {
    if (isascii(w[wi])) { /* 아스키 코드인가? */
      splitchar[i++] = FIL;
      splitchar[i++] = w[wi++];
    } 
    else { /* 한글 또는 한자 또는 2바이트 문자 */
      splitchar[i++] = w[wi++];
      splitchar[i++] = w[wi++];
    }
  }
  
  splitchar[i] = (char) NULL;
  return 1;
}
