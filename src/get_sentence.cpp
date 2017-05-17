#include <stdio.h>
#include <string.h>
#include "FST.h"
#include "hsplit.h"
#include "definitions.h"

// 마지막 함수는 stdin을 입력으로 할 수 없음

/*****************************************************************************/
/* 문장분리와 동시에 문장을 읽어옴 */
/* word[1] 부터 채워짐 */
int get_sentence_with_sbd(void *sb_fst, FILE *fp, vector<string> &word) {

  int num_word = 0;

  int num_index;
  int num_splitchar;

  char one_word[1024];

  static char ej_syl[1000][3]; /* 음절 단위로 변환된 입력 어절 */

  char last_two_char[10];

  word.clear();       // 초기화
  word.push_back(""); // [0]번째 채우기

  while (1) {

    /* 어절 읽기 */
    if (fscanf(fp, "%s", one_word) == EOF) return num_word;

    word.push_back(one_word);

    num_splitchar = split_by_char(one_word, ej_syl); /* 문자 단위로 쪼갠다 */

    if (num_splitchar > 1) {

      if (ej_syl[num_splitchar-2][0] == FIL && ej_syl[num_splitchar-1][0] == FIL) { // 둘 다 1byte 문자이면
        sprintf(last_two_char, "%s%s", &ej_syl[num_splitchar-2][1], &ej_syl[num_splitchar-1][1]);
      }
      else if (ej_syl[num_splitchar-2][0] == FIL) { // (마지막-1)번째 음절이 1byte 문자이면
        sprintf(last_two_char, "%s%s", &ej_syl[num_splitchar-2][1], ej_syl[num_splitchar-1]);
      }
      else if (ej_syl[num_splitchar-1][0] == FIL) { // 마지막 음절이 1byte 문자이면
        sprintf(last_two_char, "%s%s", ej_syl[num_splitchar-2], &ej_syl[num_splitchar-1][1]);
      }
      else { // 둘 다 2byte 문자이면
        sprintf(last_two_char, "%s%s", ej_syl[num_splitchar-2], ej_syl[num_splitchar-1]);
      }

      /* 문장 분리 정보와 비교 */
      if (String2Hash(sb_fst, last_two_char, &num_index) != (-1)) { /* 있으면 */
        num_word++;
        break; /* 문장 끝을 만났으므로 루프를 종료 */
      }
    }

    num_word++;
  }

  return num_word;
}

/*****************************************************************************/
/* fp로부터 문자열을 읽어들여 word에 저장한다. */
/* word[1] 부터 채워짐 */
// 한줄에 한 문장
int get_sentence_from_row_format(FILE *fp, vector<string> &word) {

  int num_word = 0;
  char str[MAX_MJ];
  char *pch;

  word.clear();
  word.push_back(""); // 0번째 채움

  // 한 줄 읽기
  if (fgets (str, MAX_MJ, fp) != NULL) { 
    
    if (str[0] == '\n') return 0; // 아무것도 없으면

    str[strlen(str)-1] = 0;
  
    pch = strtok (str, " \t\r\n");
    
    while (pch != NULL) {
      ///**/fprintf (stderr, "%d [%s]\n", num_word, pch);

      //strcpy(word[++num_word], pch);
      word.push_back(pch);
      num_word++;

      pch = strtok (NULL, " \t\r\n");
    }
    ///**/fprintf (stderr, "\n");
  }
  
  return num_word;
}


/*****************************************************************************/
/* 문장을 읽어옴 */
/* word[1]부터 word[num_word]에 저장됨 */
// 한 줄에 한 단어
// 빈 줄은 문장 구분에 쓰임
int get_sentence_from_column_format(FILE *fp, vector<string> &word) {
  char line[MAX_MJ];
  int num_word = 0;

  word.clear();
  word.push_back(""); // 0번째 채움
  
  while (fgets (line, MAX_MJ, fp) != NULL) {

    if (line[0] == '\n') {     // 문장의 끝
      ///**/fprintf(stderr, "%s num_word = %d\n", word[num_word], num_word);
      return num_word;
    }
    else {
      ///**/fprintf(stderr, "num_word = %d\n", num_word);
      line[strlen(line)-1] = 0;
      //word[++num_word][0] = 0;

      word.push_back(line);
      num_word++;
      //strcpy(word[num_word], line);
    }
  }
  return num_word;
}

/******************************************************************************/
/* return value : 문장의 단어수 */
/* morph_analyzed_result[1]부터 첫 어절이 채워짐 */
int get_sentence_from_morphological_analyzed_text(FILE *fp, vector<string> &word, 
                                                  vector<ANALYZED_RESULT> &morph_analyzed_result) {
  static int line_num = 0;

  int num_word = 0;
  char line[MAX_MJ];

  char morph_result[MAX_WORD];
  double prob;

  char one_word[1024];

  /* 문장 시작 */
  //morph_analyzed_result[0].clear(); /* 초기화 */
  morph_analyzed_result.clear(); /* 초기화 */

  ANALYZED_RESULT temp2;
  temp2.push_back(make_pair(1.0, BOW_TAG_1));
  morph_analyzed_result.push_back(temp2); /* 확률 = 1 */

//  morph_analyzed_result[1].clear(); /* 초기화 */

  word.clear();       // 초기화
  word.push_back(""); // 0번째 채움

  temp2.clear(); // 초기화

  while (fgets(line, MAX_MJ, fp) != NULL) {

    line_num++;

    if (line[0] == '\n') { // 문장의 끝

      if (!temp2.empty()) { // 연속된 공백을 대비
        morph_analyzed_result.push_back(temp2);
        temp2.clear();
        return num_word;
      }
    }

    if (line[0] != '\t') { // 원어절 시작이면
      ///**/fprintf(stderr, "not tab, num_word = %d\n", num_word);
      if (num_word) {
        morph_analyzed_result.push_back(temp2);
        temp2.clear();
      }

      num_word++;

      /* 원어절, 형태소분석결과, 확률 */
      sscanf(line, "%s%s%lf", one_word, morph_result, &prob);
      word.push_back(one_word);

      //morph_analyzed_result[num_word+1].clear(); /* 초기화 */
    }
    else {
      ///**/fprintf(stderr, "tab\n");
      /* 형태소분석결과, 확률 */
      sscanf(line, "%s%lf", morph_result, &prob);
    }

    /* 확률 + 분석결과 저장 */
    temp2.push_back(make_pair(prob, morph_result));
    
  } // end of while
  
  if (!temp2.empty()) {
    /**/fprintf(stderr, "비지 않은 결과 (%d 라인)\n", line_num);
  }
  return num_word;
}

/******************************************************************************/
/* return value : 문장의 단어수 */
/* morph_analyzed_result[1]부터 첫 어절이 채워짐 */
int get_sentence_from_morphological_analyzed_text_with_sbd(void *sb_fst, FILE *fp, vector<string> &word, 
                                                  vector<ANALYZED_RESULT> &morph_analyzed_result) {
  static int line_num = 0;

  int num_word = 0;
  char line[MAX_MJ];

  char morph_result[MAX_WORD];
  double prob;

  int quit = 0;

  int num_index;
  int num_splitchar;
  static char ej_syl[1000][3]; /* 음절 단위로 변환된 입력 어절 */

  char last_two_char[10];

  char one_word[1024];


  /* 문장 시작 */
  morph_analyzed_result.clear(); /* 초기화 */

  ANALYZED_RESULT temp2;
  temp2.push_back(make_pair(1.0, BOW_TAG_1)); // 0번째 채움

  morph_analyzed_result.push_back(temp2); /* 확률 = 1 */

  word.clear();
  word.push_back(""); // 0번째 채움

  temp2.clear();
  
  long curfpos = ftell(fp); // 파일 위치

  while (fgets(line, MAX_MJ, fp) != NULL) {

    line_num++;

    if (line[0] != '\t') { // 원어절 시작이면

      if (num_word) {
        morph_analyzed_result.push_back(temp2);
        temp2.clear();
      }

      if (quit) { // 문장 끝
        if (!temp2.empty()) { // 연속된 공백을 대비
          morph_analyzed_result.push_back(temp2);
          temp2.clear();
        }

        fseek(fp, curfpos, SEEK_SET); // 파일 위치 되돌림
        line_num--;
        break;
      }

      num_word++;

      /* 원어절, 형태소분석결과, 확률 */
      sscanf(line, "%s%s%lf", one_word, morph_result, &prob);
      word.push_back(one_word);

      num_splitchar = split_by_char(one_word, ej_syl); /* 문자 단위로 쪼갠다 */

      if (num_splitchar > 1) {

        if (ej_syl[num_splitchar-2][0] == FIL && ej_syl[num_splitchar-1][0] == FIL) { // 둘 다 1byte 문자이면
          sprintf(last_two_char, "%s%s", &ej_syl[num_splitchar-2][1], &ej_syl[num_splitchar-1][1]);
        }
        else if (ej_syl[num_splitchar-2][0] == FIL) { // (마지막-1)번째 음절이 1byte 문자이면
          sprintf(last_two_char, "%s%s", &ej_syl[num_splitchar-2][1], ej_syl[num_splitchar-1]);
        }
        else if (ej_syl[num_splitchar-1][0] == FIL) { // 마지막 음절이 1byte 문자이면
          sprintf(last_two_char, "%s%s", ej_syl[num_splitchar-2], &ej_syl[num_splitchar-1][1]);
        }
        else { // 둘 다 2byte 문자이면
          sprintf(last_two_char, "%s%s", ej_syl[num_splitchar-2], ej_syl[num_splitchar-1]);
        }

        //**/fprintf(stdout, "%s\n", last_two_char);
        /* 문장 분리 정보와 비교 */
        if (String2Hash(sb_fst, last_two_char, &num_index) != (-1)) { /* 있으면 */
          quit = 1; // 문장끝 어절임
          //**/fprintf(stdout, "문장끝\n");
        }
      }
    }

    else {
      /* 형태소분석결과, 확률 */
      sscanf(line, "%s%lf", morph_result, &prob);
    }

    
    /* 확률 + 분석결과 저장 */
    temp2.push_back(make_pair(prob, morph_result));
      
    curfpos = ftell(fp);  // 파일 위치

  } // end of while
  
  if (!temp2.empty()) {
    morph_analyzed_result.push_back(temp2);
    temp2.clear();
  }

  //**/fprintf(stderr, "num word = %d\n", num_word);

  return num_word;
}

/*****************************************************************************/
int sbd_open(void **sb_fst, char *fst_filename) {

  fprintf(stderr, "\tReading sentence boundary information FST.. [%s]", fst_filename);
  
  if (!(*sb_fst = LoadTransducer (fst_filename, NULL))) {
    fprintf (stderr, "[ERROR] Cannot open FST! [%s]\n", fst_filename);
    return 0;
  }
  fprintf(stderr, "\t[done]\n");
  
  return 1;
}

/*****************************************************************************/
void sbd_close(void *sb_fst) {
  if (sb_fst) FreeTransducer (sb_fst);      /* FST */
}
