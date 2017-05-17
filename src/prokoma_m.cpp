#include <stdio.h>
#include <math.h>               /* log () */
#include <stdlib.h>
#include <limits.h>               /* LONG_MAX */

#include "definitions.h"
#include "probability_tool.h"
#include "env.h"
#include "phonetic_change.h"
#include "triangular_matrix.h"
#include "unit_conversion.h"

/******************************************************************************/
/* ���ҽ� ���� */
int prokoma_m_open(char *LEXICAL_PRB_Path, char *TRANSITION_PRB_Path, 
                   PROB_MAP &lexical_prob, PROB_MAP &transition_prob) {

  /* Ȯ���� �Է� */
  /* ���� Ȯ�� */
  fprintf(stderr, "\tReading lexical probabilities.. [%s]", LEXICAL_PRB_Path);
  if (!map_scan_probability(LEXICAL_PRB_Path, lexical_prob, "t")) {
    fprintf(stderr, "\t[ERROR] can't read the lexical probabilities! [%s]\n", LEXICAL_PRB_Path);
    return 0;
  }
  fprintf(stderr, "\t[done]\n");

  /* ���� Ȯ�� */
  fprintf(stderr, "\tReading transition probabilities.. [%s]", TRANSITION_PRB_Path);
  if (!map_scan_probability(TRANSITION_PRB_Path, transition_prob, "t")) {
    fprintf(stderr, "\t[ERROR] can't read the transition probabilities! [%s]\n", TRANSITION_PRB_Path);
    return 0;
  }
  fprintf(stderr, "\t[done]\n");

  return 1;
}


/******************************************************************************/
/**/
void print_tab(int depth) {
	int i;
	
	for (i = 0; i < depth; i++) {
    fprintf(stdout, "\t");
  }
}

/*****************************************************************************/
static char *strRstr(char *string, const char *find) {
	size_t stringlen, findlen;
	char *cp;

	findlen = strlen(find);
	stringlen = strlen(string);
	if (findlen > stringlen)
		return NULL;

	for (cp = string + stringlen - findlen; cp >= string; cp--) {
		if (strncmp(cp, find, findlen) == 0)
			return cp;
  }
	return NULL;
}

/******************************************************************************/
/* ���� ���¼� �м� ��� */
// CYK �˰���
int cyk_m(PROB_MAP &transition_prob, /* ���� Ȯ�� */
              SUB_STRING_INFO &substr_info, /* �κ� ���ڿ��� ���� ���� */
              int which_ej, /* �� ��° �����ΰ�? */
              SUB_STRING &sub_str, /* �κ� ���ڿ� */
              int len, /* ���� �� */
              double phonetic_change_prob /* ���� ���� Ȯ�� */, 
              ANALYZED_RESULT_MAP &analyzed_result_m, /* ��� ���� */
              double cutoff_threshold, 
              double threshold2,
              char delimiter) {

  int i, j;
  int cur_tab; // ����
  int front_tab; // ����

  SUB_STRING_INFO table; // �κ� ��� ����

  SUB_STRING_INFO::iterator it, it2;
  BI_STATE_MAP::iterator iter, iter2;
  
  double trans_prob, trans_prob2; // ����Ȯ��

  static double max_prob; // �ִ� Ȯ��
  double cur_prob;

  if (which_ej == 0) max_prob = -LONG_MAX;

  /*
  ��ȭ����
  ��(0), MAG, -8.18
  ��(0), NNG, -9.88
  ��(0), NNP, -8.24
  ��(0), NR, -8.00
  ��ȭ(1), NNG, -7.32
  ȭ(4), NNG, -8.66
  ȭ(4), NNP, -11.50
  ȭ(4), XSN, -2.91
  ȭ��(5), NNG, -10.32
  ��(7), MM, -3.72
  ��(7), NNB, -9.56
  ��(7), NNG, -6.35
  ��(7), NNP, -6.30
  ����(8), NNG, -7.65
  ��(9), NNB, -10.86
  ��(9), NNG, -7.83
  ��(9), NNP, -7.94
  */

  t_TAB pos;

  const char delim[2] = {delimiter, 0};

  /* base case */
  // ������ ����� ���¼ҵ��� ǰ��� Ȯ���� �˾Ƴ��� ���̺� ����
  for (it = substr_info.begin(); it != substr_info.end(); it++) {
    
    TabPos1to2(it->first, len, &pos);
      ///**/fprintf(stderr, "(%d, %d)\n", pos.x, pos.y);

    // ��� ǰ�翡 ����
    for (iter = it->second.begin(); iter != it->second.end(); iter++) {

      ///**/fprintf(stderr, "%s(%d), %s, %.2lf\n", sub_str[it->first].c_str(), it->first, iter->first.c_str(), iter->second);

      if (pos.x == 0) { // ������ ���ۺκ��̸�

        // ���� ó�� �±׿��� ���Ἲ �˻�
        // ���� �Ұ����ϸ�
        if ((trans_prob = transition_prob[BOW_TAG_1/*����ǰ��*/][iter->first/*����ǰ��*/]) <= 0.0) {
          ///**/fprintf(stderr, "�Ұ� %s -> %s\n", BOW_TAG_1, iter->first.c_str());
          continue;
        }

        if (pos.y == len) { // ���� ��ü�� �ϳ��� ���¼��̸�

          // ���� �� �±׿��� ���Ἲ �˻�
          // ���� �Ұ����ϸ�
          if ((trans_prob2 = transition_prob[iter->first/*����ǰ��*/][EOW_TAG/*������ǰ��*/]) <= 0.0) {
            ///**/fprintf(stderr, "�Ұ� %s -> %s\n", iter->first.c_str(), EOW_TAG);
            continue;
          }

          // ���� Ȯ�� + ����Ȯ�� + ����� Ȯ�� + ����Ȯ��
          cur_prob = iter->second + log(trans_prob) + phonetic_change_prob + log(trans_prob2);
          
          // �ִ� Ȯ�� ����
          if (cur_prob > max_prob) max_prob = cur_prob;
        }

        else { // ������ ���ۺκ� and ��ü�� �ϳ��� ���¼Ұ� �ƴ�
          // ���� Ȯ�� + ����Ȯ�� + ����� Ȯ��
          cur_prob = iter->second + log(trans_prob) + phonetic_change_prob; 
        }
      }
      else { // ������ �߰��κ��̸�
        cur_prob = iter->second; // ���� Ȯ��
      }
      
      /* cut-off�� �������� �˻� */
      if (cutoff_threshold > 0 && max_prob - cur_prob > cutoff_threshold) {
        ///**/fprintf(stderr, "max_prob %lf - cur_prob %lf = %lf\n", max_prob, cur_prob, max_prob-cur_prob);
        ///**/fprintf(stderr, ">\n");
        continue; // ���⼭ ����
      }

      // table�� ����
      // table[�ε���][���¼�/ǰ��] = Ȯ��
      table[it->first][sub_str[it->first]+delimiter+iter->first] = cur_prob;

      ///**/fprintf(stderr, "%d, %s/%s\n", it->first, sub_str[it->first].c_str(), iter->first.c_str());
      
    }
  }

  char result[1000];
  char *prev_last_pos;
  //for (j = 2; j <= len; j++) {
  //  for (i = j-1; i >= 1; i--) {

  // T(0, i) : ����
  // T(i, j) : ����
  // T(0, j) : ��ģ ���
  for (i = 1; i < len; i++) {
    for (j = i+1; j <= len; j++) {

      // �պκ� + ����κ�
      ///**/fprintf(stderr, "%s + %s\n", sub_str[TabPos2to1(0, i, len)].c_str(), sub_str[TabPos2to1(i, j, len)].c_str());

      double cur_max = -LONG_MAX;

      // ����κ��� ������ ��ϵǾ� ������
      cur_tab = TabPos2to1(i, j, len); // ����
      it = substr_info.find(cur_tab);
      
      if ( it == substr_info.end() ) { // ������
        continue;
      }
      
      // �պκ��� ��м� ����� ������
      front_tab = TabPos2to1(0, i, len); // ����
      it2 = table.find(front_tab);

      if ( it2 == table.end() ) { // ������
        continue;
      }

      // �պκ� (��� �м� ����� ����)
      for (iter = it2->second.begin(); iter != it2->second.end(); ++iter) {

        // ���� �м��� ������ ǰ��
        prev_last_pos = strRstr((char *)iter->first.c_str(), delim) + 1;

        // ����κ� (��� ǰ�翡 ����)
        for (iter2 = it->second.begin(); iter2 != it->second.end(); ++iter2) {

          // ���� ���ɼ� �˻�
          ///**/fprintf(stderr, "\n'%s'�� ���� ǰ�� = %s\n", sub_str[cur_tab].c_str(), iter2->first.c_str()); /* ���� ���¼��� ǰ�� */
          ///**/fprintf(stderr, "���� �м� = %s\n", iter->first.c_str());

          ///**/fprintf(stderr, "���� �м��� ������ ǰ�� = %s\n", prev_last_pos);

          /* ���� ǰ����� ���Ἲ �˻� */
          // ����Ȯ�� : ���� �м��� ������ ǰ�� + ���� ���¼��� ǰ�� */
          if ((trans_prob = transition_prob[prev_last_pos][iter2->first]) <= 0.0) { /* ���� �Ұ����ϸ� */ /* Ȯ���� �� ��! */
            ///**/fprintf(stderr, "�Ұ� %s -> %s\n", prev_last_pos, iter2->first.c_str());
            continue;
          }

          // �����ؼ� ���� �� ���� �±�
          if (strcmp(prev_last_pos, iter2->first.c_str()) == 0) {
            if (strcmp(prev_last_pos, "SL") == 0 
                || strcmp(prev_last_pos, "SH") == 0 
                || strcmp(prev_last_pos, "SN") == 0
                || strcmp(prev_last_pos, "SE") == 0) {
              continue;
            }
          }

          // ������� �� ���� ���ᰡ���� �����
          // �м���� �� Ȯ�� ����
          // ���� �м� ��� + ���� ���¼�/ǰ��
          sprintf(result, "%s+%s%c%s", iter->first.c_str(), sub_str[cur_tab].c_str(), delimiter, iter2->first.c_str());
          
          // ���� ���ΰ�?
          if (j == len) {
            /* ������ ǰ����� ���Ἲ �˻� */
            if ((trans_prob2 = transition_prob[iter2->first][EOW_TAG]) <= 0.0) {
              ///**/fprintf(stderr, "�Ұ� %s -> %s\n", iter2->first.c_str(), EOW_TAG);
              continue;
            }
              
            // ���� �м� Ȯ�� + ���� Ȯ�� + ���� Ȯ�� + (������ ǰ��� EOS����) ���� Ȯ�� 
            cur_prob = iter->second + log(trans_prob) + iter2->second + log(trans_prob2);

            if (cur_prob > max_prob) max_prob = cur_prob; // �ִ�Ȯ�� ����
          }
          
          else { // ���� ���� �ƴϸ�
            // ���� �м� Ȯ�� + ���� Ȯ�� + ���� Ȯ��
            cur_prob = iter->second + log(trans_prob) + iter2->second;
          }
            
          /* cut-off�� �������� �˻� */
          if (cutoff_threshold > 0 && max_prob - cur_prob > cutoff_threshold) {
            continue; /* ���⼭ ���� */
          }

          /* �������� ����Ͽ� ���� threshold ���� Ȯ������ ������ */
          #ifdef SYLLABLE_ANALYSIS // �ڿ��� ���� ���� �м��� �ϴ� ��츸 �˻�
          if (cur_prob < threshold2) { 
            continue;
          }
          #endif

          // �ӵ� ��� ����� �⿩, but �����Ǵ� ������� ���� ���� �ִ�.
          if (cur_max - cur_prob > 15) { // 10�� �Ẹ�⵵ �߾���
            continue;
          }
          if (cur_max < cur_prob) cur_max = cur_prob;

          // ����
          table[TabPos2to1(0, j, len)][result] = cur_prob;

          ///**/fprintf(stderr, "(0, %d) %s\n", j, result);

          ///**/fprintf(stderr, "%s\t%12.11e\n", result, table[TabPos2to1(0, j, len)][result]);

          ///**/fprintf(stderr, "\n");

        } // end of for ����κ� (��� ǰ�翡 ����)
      } // end of for �պκ� (��� �м� ����� ����)
    } // end of for (i = j-1; i >= 1; i--)
  } // end of for (j = 2; j <= len; j++)

  // (��ü ������ ����) ���� �м� ��� /////////////////////////////////////////////
  cur_tab = TabPos2to1(0, len, len);
  max_prob = -LONG_MAX;

  for (iter = table[cur_tab].begin(); iter != table[cur_tab].end(); ++iter) {

    // �ִ� Ȯ�� ����
    if (iter->second > max_prob) max_prob = iter->second;

    if (cutoff_threshold > 0 && max_prob - iter->second > cutoff_threshold) continue;

    /* ��� ���� (Ȯ�� + �м����) */
    analyzed_result_m.insert(make_pair(iter->second, iter->first));
  }

  return 1;
}

/******************************************************************************/
/* ���� ���¼� �м� ��� */
// ���� �켱 Ž��
int depth_first_m(PROB_MAP &transition_prob, 
                  SUB_STRING_INFO &substr_info, 
                  int which_ej, /* �� ��° �����ΰ�? */
                  SUB_STRING sub_str, int len,  /* ��������� �Һ� */
                  t_TAB position, const char *prev_pos, int depth, double path_prob, char *result, 
                  ANALYZED_RESULT_MAP &analyzed_result_m, double cutoff_threshold, double threshold2,
                  char delimiter) {

  static double max_prob; /* ���ݱ��� �м��� ����� ���� ���� Ȯ���� (for cutoff) */

  int j;
  t_TAB head, tail;
 
  BI_STATE_MAP::iterator itr = NULL;
  int cur_tab;
  double trans_prob; /* ���� Ȯ�� */
  double path_prob_backup;

  char *cur_result = result + strlen(result);
  
  char one_result[500]; /* �ӽ� */
  
  path_prob_backup = path_prob; /* ���� ��� Ȯ�� ���� (�ݵ�� �ؾ� ��) */
  *cur_result = 0;


  /* �ʱ�ȭ (���� ó���� ��쿡��) */
  if (depth == 0 && which_ej == 0) max_prob = -LONG_MAX;

  /* ���ڿ� ũ�� ���� �ݺ� */       	
  //for (j = position.x+1; j <= len; j++) { /* �ִܿ켱 */
  for (j = len; j > position.x; j--) { /* ����켱 */
  	
  	cur_tab = TabPos2to1(position.x, j, len);
 		
  	/* ���� Ž�� */
  	/* ���� ������ ������ ��� ǰ�翡 ���� �ݺ� */
    for (itr = substr_info[cur_tab].begin(); itr != substr_info[cur_tab].end(); ++itr ) { /* ������ */
      
      /* ���� ǰ����� ���Ἲ �˻� */
      if ((trans_prob = transition_prob[prev_pos][itr->first]) <= 0.0) { /* ���� �Ұ����ϸ� */ /* Ȯ���� �� ��! */
        continue;
      }

      /* Ȯ�� ���� : ���� ��� Ȯ��, (�������� �������) ���� Ȯ��, ���� Ȯ�� */
      path_prob = path_prob_backup + log(trans_prob) + itr->second; 

      /* cut-off�� �������� �˻� */
      if (cutoff_threshold > 0 && max_prob - path_prob > cutoff_threshold) {
        continue; /* ���⼭ ���� */
      }

      /**/
      /* �������� ����Ͽ� ���� threshold ���� Ȯ������ ������ */
      #ifdef SYLLABLE_ANALYSIS // �ڿ��� ���� ���� �м��� �ϴ� ��츸 �˻�
      if (path_prob < threshold2) { 
        continue;
      }
      #endif
      
      /* head / tail�� ������ �Ŀ� ��������� ȣ�� */
      setpos(&tail, j, len);
      setpos(&head, position.x, j); /* tail�� ����(��)�κ� */

      /* ���� ���� �˻� */
      if (is_empty(tail)) { /* tail�� ������ �� �̻� �м��� �� ���� �� */

        /* ���峡 ǰ����� ���Ἲ �˻� */
        if ((trans_prob = transition_prob[itr->first][EOW_TAG]) > 0.0) { /* Ȯ���� �� ��! */
        
          /* Ȯ�� ���� : (������ ǰ��� EOS����) ���� Ȯ�� */
          path_prob += log(trans_prob);
          
          /* cut-off�� �������� �˻� */
          if (cutoff_threshold > 0 && max_prob - path_prob > cutoff_threshold) {
            continue; /* ���⼭ ���� */
          }

          /**/
          /* �������� ����Ͽ� ���� threshold ���� Ȯ������ ������ */
          #ifdef SYLLABLE_ANALYSIS // �ڿ��� ���� ���� �м��� �ϴ� ��츸 �˻�
          if (path_prob < threshold2) {
            continue;
          }
          #endif

          /* ���¼�/ǰ�� ���� */
          if (depth) sprintf(one_result, "+%s%c%s", sub_str[cur_tab].c_str(), delimiter, itr->first.c_str());
          else sprintf(one_result, "%s%c%s", sub_str[cur_tab].c_str(), delimiter, itr->first.c_str());
          *cur_result = 0;
          strcat(cur_result, one_result);
          
          /* ��� ���� (���¼�/ǰ�� �� + Ȯ��) */
          analyzed_result_m.insert(make_pair(path_prob, result));
            
          if (path_prob > max_prob) {
            max_prob = path_prob; /* �ִ밪���� ũ�� �ִ밪�� �� */
          }
        }
      }
      else { /* ���� ������ ������Ű�� ���� ��� */

        /* ���¼�/ǰ�� ���� */
        if (depth) {
          sprintf(one_result, "+%s%c%s", sub_str[TabPos2to1(head.x, head.y, len)].c_str(), delimiter, itr->first.c_str());
        }
        else {
          sprintf(one_result, "%s%c%s", sub_str[TabPos2to1(head.x, head.y, len)].c_str(), delimiter, itr->first.c_str());
        }
        *cur_result = 0;
        strcat(cur_result, one_result); /* ��� ���� */
          
        /* ��������� ȣ�� */
        if (!depth_first_m(transition_prob, substr_info, which_ej, sub_str, len, 
                  tail, itr->first.c_str(), depth+1, path_prob, result, 
                  analyzed_result_m, cutoff_threshold, threshold2, delimiter)) return 0;
      }
    } /* end of for */
  } /* end of for */
  
  return 1;
}

/******************************************************************************/
/* ��� �κ� ���ڿ��� �̸� �������� ã�´�. */
// ��� : substr_info, sub_str
int get_all_possible_pos_from_substring(int len, char *source_str, PROB_MAP &lexical_prob, 
                                        SUB_STRING_INFO &substr_info, 
                                        SUB_STRING &sub_str) {

  PROB_MAP::iterator itr;
  NUM_PROB::iterator itr2;

  int i, j, k;

  char str[MAX_WORD];

  /* �ݵ�� �ʱ�ȭ�ؾ� �� */
  // ����� �� ���� ������ �ٽ� ������ �ϱ� ���� (�ʱ�ȭ�� ���� ������ ���� ����Ÿ�� ���� ���� �߻�)
  sub_str.clear(); 
  substr_info.clear();

  // �κ� ���ڿ� ����
  for (k = i = 0; i < len; i++) {

    for (j = i; j < len; j++, k++) {

      strncpy(str, source_str+i*2, (j-i+1)*2);
      str[(j-i+1)*2] = 0;
      ///**/fprintf(stderr, "[%d] %s\n", k, str);
      sub_str.push_back(str); // ����

      itr = lexical_prob.find(str); /* �κ� ���ڿ��� �������� ã�´�. */

      if ( itr != lexical_prob.end() ) { /* ������ */

        /* ���� �� �ִ� ��� ǰ�縦 ã�´�. */
        for (itr2 = lexical_prob[itr->first].begin(); itr2 != lexical_prob[itr->first].end(); ++itr2) {
          substr_info[k][itr2->first/*ǰ��*/] = log(itr2->second); /* ���� Ȯ�� */
        }
      } // end of if
    } // end of for
  } // end of for

  return 1;
}

/******************************************************************************/
/* result_m�� ����� ����� �����Ͽ� analyzed_result_m�� ���� */
/* ���ϰ� : �м� ����� �� */
int arrange_result_m(ANALYZED_RESULT_MAP &result_m, ANALYZED_RESULT &analyzed_result_m, RESTORED_STAGS &str_syl_tag_seq, double cutoff_threshold, char delimiter) {

  ANALYZED_RESULT_MAP::iterator res_itr;
  //double prob_sum = 0.0; /* Ȯ���� ��, for normalization */
  int num_result = 0;

  int num_morph = 0;

  if (result_m.empty()) return 0; /* ����� ������ ���� */

  /* Ȯ�� normalization **********************************************/
  res_itr = result_m.begin();
  double max_prob = res_itr->first; /* �ְ� Ȯ���� */


  /* ��� �м� ����� ���� */
  /* res_itr->first : Ȯ���� */
  /* res_itr->second : ���¼� �м���� */
  for (res_itr = result_m.begin(); res_itr != result_m.end(); ) {

    /* cut-off�� �������� �˻� */
    if (cutoff_threshold > 0) { /* �� ���� 0 �̻��� ��쿡�� ����� */
  
      /* �ְ� Ȯ���� ���� ������� �αװ��� ���� ����ġ�̻��̸� ���� */
      if (max_prob - res_itr->first > cutoff_threshold) {
        result_m.erase(res_itr, result_m.end()); /* ���⼭���� ������������ ���� (������ ����� Ȯ������ �� �����Ƿ�) */
        break;
      }
    }

    /* ������ �±׿��� ���缺 �˻� */
    if (!(num_morph = check_morpheme_result((char *) res_itr->second.c_str(), str_syl_tag_seq, delimiter))) {

      /* �ʿ��� �����ϴ� ���� �̻��ϰ� ������. */
      result_m.erase(res_itr++); /* ���� */
      continue; /* do nothing */
    }

    //prob_sum += exp(res_itr->first); /* �α׸� ������ �� ���� ���Ѵ�. */

    res_itr++; /* ���� */
  } /* end of for */

  //prob_sum = log(prob_sum); /* �ٽ� �α׷� �ٲ۴�. */

  /* ��� ��ȯ **********************************************/
  char result_str[MAX_WORD]; /* FIL�� ������ ��� */

  /* ��� �м� ����� ���� */
  for (res_itr = result_m.begin(); res_itr != result_m.end(); ++res_itr) {

    /* ������� FIL�� ���� */
    convert_str_origin_array((char *) res_itr->second.c_str(), result_str);

    /* ��� ���� (Ȯ�� + ���¼�/ǰ�� ��) */

    // ����ȭ
//    double prob = exp(res_itr->first - prob_sum); /* �αװ��̹Ƿ� ���� ���� �����δ� ������ ���� */
//    analyzed_result_m.push_back(make_pair(prob, (char *) result_str));
    
    // ����ȭ���� ����
    analyzed_result_m.push_back(make_pair(exp(res_itr->first), (char *) result_str));

    
    num_result++;
  } /* end of for */

  return num_result; /* �м��� ����� �� */
}

/******************************************************************************/
/* Ȯ���� ���¼� �м� (���¼� ����) */
/* input_ej : �Է� ���� */
/* analyzed_result_m : �м� ��� + Ȯ�� */
int prokoma_m(PROB_MAP &transition_prob, PROB_MAP &lexical_prob,  
              RESTORED_RESULT &restored_ej,
              RESTORED_STAGS &str_syl_tag_seq,
              ANALYZED_RESULT &analyzed_result_m, double cutoff_threshold, char delimiter) {
   
  int len;  
  t_TAB position; /* ���ڿ��� ��ġ�� */

  SUB_STRING_INFO substr_info; // �� �κ� ���ڿ����� ǰ�� ������ ����

  SUB_STRING sub_str; // �κ� ���ڿ� ����

  sub_str.reserve(TabNum(30)); // capacity�� ����

  ANALYZED_RESULT_MAP result_m;
  /*****************************************/

  RESTORED_RESULT::iterator it = restored_ej.begin();
  double max_restored_prob = it->first; // ù ���� ���� Ȯ��

  /* ��� ������ ������ ���� �ݺ� */
  int i; // ������ ������ ��ȣ
  for (it = restored_ej.begin(), i = 0; it != restored_ej.end(); ++it, i++) {
    
    /* ���� ���� 15���� ���� ������ ù��° ������ ������ ���ؼ��� ���� */
    if (strlen(it->second.c_str()) >= 15 * 2) {
      if (it != restored_ej.begin()) break;
    }

    /* ���� ���� Ȯ������ �ʹ� ���� ��쳪 0�� ��쿡�� �м����� �ʴ´�. */
    if (exp(it->first) == 0.0 || (cutoff_threshold && (max_restored_prob - it->first) > cutoff_threshold) ) {
      ///**/if (exp(it->first) == 0.0) fprintf(stderr, "haha\n");
      ///**/else fprintf(stderr, "hehe\n");
      break;
    }


    //#define DEBUG_PHONETIC
    //#ifdef DEBUG_PHONETIC /********************************************/
    //fprintf(stderr, "[%d]%s : %lf\n", i, it->second.c_str(), it->first);
    //#endif /***********************************************************/

    len = (int) strlen(it->second.c_str()) / 2; // ���ڿ� ����
    if (len > 30) { // 30���� �ʰ��̸� CYK �˰����� �ʹ� ��ȿ�����̹Ƿ� �м� ���� ����
      continue;
    }

    //**/fprintf(stderr, "len = %d\n", len);
  
    /* �ʱ�ȭ *****************************************************/
    setpos(&position, 0, len); // ��ġ ����
	  
    /* �κ� ���ڿ� ���� */
    /* ��� �κ� ���ڿ��� �̸� �������� ã�´�. */
    /* �Է� : sub_str (�κ� ���ڿ� ����) */
    /* ��� : substr_info (�� �κ� ���ڿ��� ���� �� �ִ� ǰ��� Ȯ���� �� ����) */
    get_all_possible_pos_from_substring(len, (char *) it->second.c_str(), 
                                        lexical_prob, substr_info, sub_str);

    // ������� ���� ���
    ///**/fprintf(stderr, "%s\n", it->second.c_str());

    /* ���¼� �м� ��� */
    cyk_m(transition_prob, 
          substr_info, i, sub_str, len, 
          it->first, // �α� Ȯ�� (�ʱⰪ)
          result_m, cutoff_threshold, log(pow(1.0e-03, len)), delimiter);

/*    result[0] = 0;
    char result[MAX_WORD] = {0,}; // ��� ���� 
    depth_first_m(
                  transition_prob, 
                  substr_info, i, sub_str, len, 
                  position, BOW_TAG_1, 0, it->first, // �α� Ȯ�� (�ʱⰪ)
                  result, result_m, cutoff_threshold, log(pow(1.0e-03, len)), delimiter);
*/
  } /* end of for */

  /* ��� ���� */
  int num_result = arrange_result_m(result_m, analyzed_result_m, 
                                    str_syl_tag_seq, cutoff_threshold, delimiter);

  return num_result;
}
