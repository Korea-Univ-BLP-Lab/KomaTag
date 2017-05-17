#include <stdio.h>
#include <string>
#include <vector>
#include <string.h>
#include <math.h>

#include "definitions.h"
#include "hsplit.h"
#include "get_morph_tag.h"

/*****************************************************************************/
// ���ڿ� s���� ���� c�� ��Ÿ�� ȸ���� �����Ѵ�.
int count_strchr(const char *s, int c) {
  char *ptr = (char *)s;
  char *ptr2;
  int count = 0;
  
  while ((ptr2 = strchr(ptr, c)) != NULL) {
    count++;
    ptr = ++ptr2;
  }
  return count;
}

/*****************************************************************************/
/* �±� ��� ��� */
/* ejs : ���峻 ���� */
/* morph_analyzed_result : (���) ���¼� �м� ��� */
/* num_word : ������ �� */
/* tagging_result : (���Ǽ� �ؼҵ�) �±� ��� (���¼� �м� ����� ��ȣ) */
void print_nbest_tagging_result(FILE *fp, vector<string> ejs, 
                          vector<ANALYZED_RESULT> &morph_analyzed_result, 
                          int num_word, int *tagging_result,
                          char delimiter, int output_style, int constraint,
                          int relative_threshold, int absolute_threshold) {
                            
  int num_result;
  int print_nbest = 0;
  double relative_ratio;
  int num_morph;

  // �� ������ ����
  for (int i = 1; i <= num_word; i++) {
    
    num_result = (int) morph_analyzed_result[i].size(); // ������ ���¼� �м� ����� ��
    
    string anal_result = morph_analyzed_result[i][0].second; // ù ��° ���¼� �м� ���
    
    //**/fprintf(fp, "%s num_result = %d, tagging_result = %d\n", ejs[i].c_str(), num_result, tagging_result[i]);


    // �м� ����� ���� �ϳ��̰�, �̺м��� �ƴ϶� n-best ����� �ʿ��� ��쵵 ���� (�����Ӱ谪�� ����)

    // ���¼� �м��� �°��� ��ġ ����
    // ������ �������� ���ϸ�
    // �±� ����� ���¼� �м� ����� ���� �ʰų�, �̺м��̸�
    if (tagging_result[i] != 0 || strstr(anal_result.c_str(), "??") != NULL) {
      print_nbest = 1;
    }
    
    // ���¼� �м� Ȯ�� ��� �Ӱ谪 ����
    if (!print_nbest && (constraint & RELATIVE_MORPH_CONSTRAINT)) {
      
      if (num_result > 1) {

        relative_ratio = log(morph_analyzed_result[i][0].first) - log(morph_analyzed_result[i][1].first); // �� Ȯ���� ����
        
        // �Ӱ谪���� ũ�� ������ ���� �м� ����� ���
        // �Ӱ谪�� ũ�� Ȯ�� �ܾ����� ������ (Ȯ�� ��Ȯ���� ������)
        // �Ӱ谪�� ������... �ణ�� Ȯ�� ���̸� ���� �ϳ��� ����� ����ϹǷ� Ȯ�� �ܾ����� ������ (��� Ȯ�� ��Ȯ���� ����������)
        if (relative_ratio < relative_threshold) { 
          print_nbest = 1;
        }
      }
    }
    
    // ���¼� �м� Ȯ�� ���� �Ӱ谪 ����
    if (!print_nbest && (constraint & ABSOLUTE_MORPH_CONSTRAINT)) {
      num_morph = count_strchr(anal_result.c_str(), '+');
      num_morph++;
      
      if (log(morph_analyzed_result[i][0].first) < log(pow(0.001 * absolute_threshold, num_morph))) {
        print_nbest = 1;
      }
    }
  
    // ��º� //////////////////////////////

    // ��� ��Ÿ��
    if (output_style) {
      // �ټ� ���
      if (print_nbest) {
        // �� �м� ����� ����

        fprintf(fp, "@\t%s", ejs[i].c_str()); // ������
        for (int j = 0; j < num_result; j++) {
          if (!j) fprintf(fp, "\t%s\n", morph_analyzed_result[i][j].second.c_str()); // �м� ���
          else fprintf(fp, "@\t%s\n", morph_analyzed_result[i][j].second.c_str()); // �м� ���
        }
      }
      else { // �ϳ��� ���
        fprintf(fp, "%s", ejs[i].c_str()); // ������
        fprintf(fp, "\t%s\n", anal_result.c_str()); // �м� ���
      }
      print_nbest = 0;
    }

    // ������ȹ ��Ÿ��
    else {
      int morph_num = 0; /* ���� ���� ���¼� �� */
      char morphs[MAX_WORD][MAX_WORD]; /* ���¼� �� */
      char tags[MAX_WORD][MAX_WORD]; /* ǰ�� �±� �� */
      int spacing_tags[MAX_WORD]; /* ���� �±� �� */

      // �ټ� ���
      if (print_nbest) {
        // �� �м� ����� ����
        for (int j = 0; j < num_result; j++) {

          if (!j) fprintf(fp, "@\t%s\t", ejs[i].c_str()); // ������

          // ��
          else {
            fprintf(fp, "@\t");
          }

          // �м����
          get_morphs_tags((char *)morph_analyzed_result[i][j].second.c_str(), &morph_num,
                      morphs, tags, spacing_tags, delimiter);

          // ���¼Ҹ���
          for (int k = 0; k < morph_num; k++) {
            if (k) fprintf(fp, " + %s%c%s", morphs[k], delimiter, tags[k]); 
            else fprintf(fp, "%s%c%s", morphs[k], delimiter, tags[k]);
          }
          fprintf(fp, "\n");

        }
      }
      else { // �ϳ��� ���
        fprintf(fp, "%s\t", ejs[i].c_str()); // ������

        // �м���� (ù��°)
        get_morphs_tags((char *)morph_analyzed_result[i][0].second.c_str(), &morph_num,
                    morphs, tags, spacing_tags, delimiter);

        // ���¼Ҹ���
        for (int k = 0; k < morph_num; k++) {
          if (k) fprintf(fp, " + %s%c%s", morphs[k], delimiter, tags[k]); 
          else fprintf(fp, "%s%c%s", morphs[k], delimiter, tags[k]);
        }
        fprintf(fp, "\n");
      }
      print_nbest = 0;
    }

  } // end of for
}

/*****************************************************************************/
/* �±� ��� ��� */
/* ejs : ���峻 ���� */
/* morph_analyzed_result : (���) ���¼� �м� ��� */
/* num_word : ������ �� */
/* tagging_result : (���Ǽ� �ؼҵ�) �±� ��� (���¼� �м� ����� ��ȣ) */
// ���ϰ� : ���۾� �ؾ� �� ���� ��
int print_nbest_tagging_result2(FILE *fp, vector<string> ejs, 
                          vector<ANALYZED_RESULT> &morph_analyzed_result, 
                          int num_word, int *tagging_result,
                          char delimiter, int output_style, int constraint,
                          int relative_threshold, int absolute_threshold) {
                            
  int num_result;
  int print_nbest = 0;
  double relative_ratio;
  int num_morph;

  int to_be_revised = 0;

  // �� ������ ����
  for (int i = 1; i <= num_word; i++) {
    
    num_result = (int) morph_analyzed_result[i].size(); // ������ ���¼� �м� ����� ��
    
    string anal_result = morph_analyzed_result[i][0].second; // ù ��° ���¼� �м� ���
    
    //**/fprintf(fp, "%s num_result = %d, tagging_result = %d\n", ejs[i].c_str(), num_result, tagging_result[i]);


    // �м� ����� ���� �ϳ��̰�, �̺м��� �ƴ϶� n-best ����� �ʿ��� ��쵵 ���� (�����Ӱ谪�� ����)

    // ���¼� �м��� �°��� ��ġ ����
    // ������ �������� ���ϸ�
    // �±� ����� ���¼� �м� ����� ���� �ʰų�, �̺м��̸�
    if (tagging_result[i] != 0 || strstr(anal_result.c_str(), "??") != NULL) {
      print_nbest = 1;
    }
    
    // ���¼� �м� Ȯ�� ��� �Ӱ谪 ����
    if (!print_nbest && (constraint & RELATIVE_MORPH_CONSTRAINT)) {
      
      if (num_result > 1) {

        relative_ratio = log(morph_analyzed_result[i][0].first) - log(morph_analyzed_result[i][1].first); // �� Ȯ���� ����
        
        // �Ӱ谪���� ũ�� ������ ���� �м� ����� ���
        // �Ӱ谪�� ũ�� Ȯ�� �ܾ����� ������ (Ȯ�� ��Ȯ���� ������)
        // �Ӱ谪�� ������... �ణ�� Ȯ�� ���̸� ���� �ϳ��� ����� ����ϹǷ� Ȯ�� �ܾ����� ������ (��� Ȯ�� ��Ȯ���� ����������)
        if (relative_ratio < relative_threshold) { 
          print_nbest = 1;
        }
      }
    }
    
    // ���¼� �м� Ȯ�� ���� �Ӱ谪 ����
    if (!print_nbest && (constraint & ABSOLUTE_MORPH_CONSTRAINT)) {
      num_morph = count_strchr(anal_result.c_str(), '+');
      num_morph++;
      
      if (log(morph_analyzed_result[i][0].first) < log(pow(0.001 * absolute_threshold, num_morph))) {
        print_nbest = 1;
      }
    }
  
    // ��º� //////////////////////////////

    // ��� ��Ÿ��
    if (output_style) {
      // �ټ� ���
      if (print_nbest) {
        // �� �м� ����� ����

        to_be_revised++;

        fprintf(fp, "%s", ejs[i].c_str()); // ������
        for (int j = 0; j < num_result; j++) {
          if (!j) fprintf(fp, "\t@@ %s\n", morph_analyzed_result[i][j].second.c_str()); // �м� ���
          else fprintf(fp, "\t@ %s\n", morph_analyzed_result[i][j].second.c_str()); // �м� ���
        }
      }
      else { // �ϳ��� ���
        fprintf(fp, "%s", ejs[i].c_str()); // ������
        fprintf(fp, "\t%s\n", anal_result.c_str()); // �м� ���
      }
      print_nbest = 0;
    }

    // ������ȹ ��Ÿ��
    else {
      int morph_num = 0; /* ���� ���� ���¼� �� */
      char morphs[MAX_WORD][MAX_WORD]; /* ���¼� �� */
      char tags[MAX_WORD][MAX_WORD]; /* ǰ�� �±� �� */
      int spacing_tags[MAX_WORD]; /* ���� �±� �� */

      // �ټ� ���
      if (print_nbest) {

        to_be_revised++;

        // �� �м� ����� ����
        for (int j = 0; j < num_result; j++) {

          if (!j) fprintf(fp, "%s\t@@ ", ejs[i].c_str()); // ������

          // ��
          else {
            fprintf(fp, "\t@ ");
          }

          // �м����
          get_morphs_tags((char *)morph_analyzed_result[i][j].second.c_str(), &morph_num,
                      morphs, tags, spacing_tags, delimiter);

          // ���¼Ҹ���
          for (int k = 0; k < morph_num; k++) {
            if (k) fprintf(fp, " + %s%c%s", morphs[k], delimiter, tags[k]); 
            else fprintf(fp, "%s%c%s", morphs[k], delimiter, tags[k]);
          }
          fprintf(fp, "\n");

        }
      }
      else { // �ϳ��� ���
        fprintf(fp, "%s\t", ejs[i].c_str()); // ������

        // �м���� (ù��°)
        get_morphs_tags((char *)morph_analyzed_result[i][0].second.c_str(), &morph_num,
                    morphs, tags, spacing_tags, delimiter);

        // ���¼Ҹ���
        for (int k = 0; k < morph_num; k++) {
          if (k) fprintf(fp, " + %s%c%s", morphs[k], delimiter, tags[k]); 
          else fprintf(fp, "%s%c%s", morphs[k], delimiter, tags[k]);
        }
        fprintf(fp, "\n");
      }
      print_nbest = 0;
    }

  } // end of for

  return to_be_revised;
}
