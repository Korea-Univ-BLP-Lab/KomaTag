#include <stdio.h>
#include <string.h>
#include "FST.h"
#include "hsplit.h"
#include "definitions.h"

// ������ �Լ��� stdin�� �Է����� �� �� ����

/*****************************************************************************/
/* ����и��� ���ÿ� ������ �о�� */
/* word[1] ���� ä���� */
int get_sentence_with_sbd(void *sb_fst, FILE *fp, vector<string> &word) {

  int num_word = 0;

  int num_index;
  int num_splitchar;

  char one_word[1024];

  static char ej_syl[1000][3]; /* ���� ������ ��ȯ�� �Է� ���� */

  char last_two_char[10];

  word.clear();       // �ʱ�ȭ
  word.push_back(""); // [0]��° ä���

  while (1) {

    /* ���� �б� */
    if (fscanf(fp, "%s", one_word) == EOF) return num_word;

    word.push_back(one_word);

    num_splitchar = split_by_char(one_word, ej_syl); /* ���� ������ �ɰ��� */

    if (num_splitchar > 1) {

      if (ej_syl[num_splitchar-2][0] == FIL && ej_syl[num_splitchar-1][0] == FIL) { // �� �� 1byte �����̸�
        sprintf(last_two_char, "%s%s", &ej_syl[num_splitchar-2][1], &ej_syl[num_splitchar-1][1]);
      }
      else if (ej_syl[num_splitchar-2][0] == FIL) { // (������-1)��° ������ 1byte �����̸�
        sprintf(last_two_char, "%s%s", &ej_syl[num_splitchar-2][1], ej_syl[num_splitchar-1]);
      }
      else if (ej_syl[num_splitchar-1][0] == FIL) { // ������ ������ 1byte �����̸�
        sprintf(last_two_char, "%s%s", ej_syl[num_splitchar-2], &ej_syl[num_splitchar-1][1]);
      }
      else { // �� �� 2byte �����̸�
        sprintf(last_two_char, "%s%s", ej_syl[num_splitchar-2], ej_syl[num_splitchar-1]);
      }

      /* ���� �и� ������ �� */
      if (String2Hash(sb_fst, last_two_char, &num_index) != (-1)) { /* ������ */
        num_word++;
        break; /* ���� ���� �������Ƿ� ������ ���� */
      }
    }

    num_word++;
  }

  return num_word;
}

/*****************************************************************************/
/* fp�κ��� ���ڿ��� �о�鿩 word�� �����Ѵ�. */
/* word[1] ���� ä���� */
// ���ٿ� �� ����
int get_sentence_from_row_format(FILE *fp, vector<string> &word) {

  int num_word = 0;
  char str[MAX_MJ];
  char *pch;

  word.clear();
  word.push_back(""); // 0��° ä��

  // �� �� �б�
  if (fgets (str, MAX_MJ, fp) != NULL) { 
    
    if (str[0] == '\n') return 0; // �ƹ��͵� ������

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
/* ������ �о�� */
/* word[1]���� word[num_word]�� ����� */
// �� �ٿ� �� �ܾ�
// �� ���� ���� ���п� ����
int get_sentence_from_column_format(FILE *fp, vector<string> &word) {
  char line[MAX_MJ];
  int num_word = 0;

  word.clear();
  word.push_back(""); // 0��° ä��
  
  while (fgets (line, MAX_MJ, fp) != NULL) {

    if (line[0] == '\n') {     // ������ ��
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
/* return value : ������ �ܾ�� */
/* morph_analyzed_result[1]���� ù ������ ä���� */
int get_sentence_from_morphological_analyzed_text(FILE *fp, vector<string> &word, 
                                                  vector<ANALYZED_RESULT> &morph_analyzed_result) {
  static int line_num = 0;

  int num_word = 0;
  char line[MAX_MJ];

  char morph_result[MAX_WORD];
  double prob;

  char one_word[1024];

  /* ���� ���� */
  //morph_analyzed_result[0].clear(); /* �ʱ�ȭ */
  morph_analyzed_result.clear(); /* �ʱ�ȭ */

  ANALYZED_RESULT temp2;
  temp2.push_back(make_pair(1.0, BOW_TAG_1));
  morph_analyzed_result.push_back(temp2); /* Ȯ�� = 1 */

//  morph_analyzed_result[1].clear(); /* �ʱ�ȭ */

  word.clear();       // �ʱ�ȭ
  word.push_back(""); // 0��° ä��

  temp2.clear(); // �ʱ�ȭ

  while (fgets(line, MAX_MJ, fp) != NULL) {

    line_num++;

    if (line[0] == '\n') { // ������ ��

      if (!temp2.empty()) { // ���ӵ� ������ ���
        morph_analyzed_result.push_back(temp2);
        temp2.clear();
        return num_word;
      }
    }

    if (line[0] != '\t') { // ������ �����̸�
      ///**/fprintf(stderr, "not tab, num_word = %d\n", num_word);
      if (num_word) {
        morph_analyzed_result.push_back(temp2);
        temp2.clear();
      }

      num_word++;

      /* ������, ���¼Һм����, Ȯ�� */
      sscanf(line, "%s%s%lf", one_word, morph_result, &prob);
      word.push_back(one_word);

      //morph_analyzed_result[num_word+1].clear(); /* �ʱ�ȭ */
    }
    else {
      ///**/fprintf(stderr, "tab\n");
      /* ���¼Һм����, Ȯ�� */
      sscanf(line, "%s%lf", morph_result, &prob);
    }

    /* Ȯ�� + �м���� ���� */
    temp2.push_back(make_pair(prob, morph_result));
    
  } // end of while
  
  if (!temp2.empty()) {
    /**/fprintf(stderr, "���� ���� ��� (%d ����)\n", line_num);
  }
  return num_word;
}

/******************************************************************************/
/* return value : ������ �ܾ�� */
/* morph_analyzed_result[1]���� ù ������ ä���� */
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
  static char ej_syl[1000][3]; /* ���� ������ ��ȯ�� �Է� ���� */

  char last_two_char[10];

  char one_word[1024];


  /* ���� ���� */
  morph_analyzed_result.clear(); /* �ʱ�ȭ */

  ANALYZED_RESULT temp2;
  temp2.push_back(make_pair(1.0, BOW_TAG_1)); // 0��° ä��

  morph_analyzed_result.push_back(temp2); /* Ȯ�� = 1 */

  word.clear();
  word.push_back(""); // 0��° ä��

  temp2.clear();
  
  long curfpos = ftell(fp); // ���� ��ġ

  while (fgets(line, MAX_MJ, fp) != NULL) {

    line_num++;

    if (line[0] != '\t') { // ������ �����̸�

      if (num_word) {
        morph_analyzed_result.push_back(temp2);
        temp2.clear();
      }

      if (quit) { // ���� ��
        if (!temp2.empty()) { // ���ӵ� ������ ���
          morph_analyzed_result.push_back(temp2);
          temp2.clear();
        }

        fseek(fp, curfpos, SEEK_SET); // ���� ��ġ �ǵ���
        line_num--;
        break;
      }

      num_word++;

      /* ������, ���¼Һм����, Ȯ�� */
      sscanf(line, "%s%s%lf", one_word, morph_result, &prob);
      word.push_back(one_word);

      num_splitchar = split_by_char(one_word, ej_syl); /* ���� ������ �ɰ��� */

      if (num_splitchar > 1) {

        if (ej_syl[num_splitchar-2][0] == FIL && ej_syl[num_splitchar-1][0] == FIL) { // �� �� 1byte �����̸�
          sprintf(last_two_char, "%s%s", &ej_syl[num_splitchar-2][1], &ej_syl[num_splitchar-1][1]);
        }
        else if (ej_syl[num_splitchar-2][0] == FIL) { // (������-1)��° ������ 1byte �����̸�
          sprintf(last_two_char, "%s%s", &ej_syl[num_splitchar-2][1], ej_syl[num_splitchar-1]);
        }
        else if (ej_syl[num_splitchar-1][0] == FIL) { // ������ ������ 1byte �����̸�
          sprintf(last_two_char, "%s%s", ej_syl[num_splitchar-2], &ej_syl[num_splitchar-1][1]);
        }
        else { // �� �� 2byte �����̸�
          sprintf(last_two_char, "%s%s", ej_syl[num_splitchar-2], ej_syl[num_splitchar-1]);
        }

        //**/fprintf(stdout, "%s\n", last_two_char);
        /* ���� �и� ������ �� */
        if (String2Hash(sb_fst, last_two_char, &num_index) != (-1)) { /* ������ */
          quit = 1; // ���峡 ������
          //**/fprintf(stdout, "���峡\n");
        }
      }
    }

    else {
      /* ���¼Һм����, Ȯ�� */
      sscanf(line, "%s%lf", morph_result, &prob);
    }

    
    /* Ȯ�� + �м���� ���� */
    temp2.push_back(make_pair(prob, morph_result));
      
    curfpos = ftell(fp);  // ���� ��ġ

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
