#include <stdio.h>
#include <math.h>               /* log () */
#include <stdlib.h>

#include "definitions.h"
#include "hsplit.h"
#include "env.h"
#include "phonetic_change.h"
#include "bfs.h"
#include "unit_conversion.h"
#include "probability_tool.h"

/******************************************************************************/
/* ���ҽ� ���� */
int prokoma_s_open(char *TAG_S_FST_Path, char *TAG_S_hash_Path, char *TAG_S_PROB_Path, 
                   char *SYLLABLE_S_FST_Path, char *SYLLABLE_S_hash_Path, char *SYLLABLE_S_PROB_Path,
                   char *S_TRANSITION_FST_Path,
                   void **tag_s_fst, double **tag_s_prob,
                   void **syllable_s_fst, double **syllable_s_prob,
                   void **s_transition_fst) {

  if (!fst_probability_open(TAG_S_FST_Path, TAG_S_hash_Path, TAG_S_PROB_Path, 
                            tag_s_fst, tag_s_prob)) {
    fprintf(stderr, "Error\n");
  }

  if (!fst_probability_open(SYLLABLE_S_FST_Path, SYLLABLE_S_hash_Path, SYLLABLE_S_PROB_Path, 
                            syllable_s_fst, syllable_s_prob)) {
    fprintf(stderr, "Error\n");
  }

  if ((*s_transition_fst = LoadTransducer(S_TRANSITION_FST_Path, NULL)) == NULL) {
      fprintf(stderr, "Load failure [%s]\n", S_TRANSITION_FST_Path);
      return 0;
  }

  return 1;
}

/*****************************************************************************/
void prokoma_s_close(void *tag_s_fst, double *tag_s_prob, void *syllable_s_fst, double *syllable_s_prob) {

  if (tag_s_fst) FreeTransducer (tag_s_fst);      /* FST */
  if (syllable_s_fst) FreeTransducer (syllable_s_fst);      /* FST */

  /* �޸� ���� */
  if (tag_s_prob) free (tag_s_prob);
  tag_s_prob = NULL;

  if (syllable_s_prob) free (syllable_s_prob);
  syllable_s_prob = NULL;
}

/******************************************************************************/
/* result_s�� ����� ����� �����Ͽ� analyzed_result_s�� ���� */
/* ���ϰ� : �м� ����� �� */
int arrange_result_s(ANALYZED_RESULT_MAP &result, ANALYZED_RESULT &analyzed_result_s, double cutoff_threshold) {

  ANALYZED_RESULT_MAP::iterator res_itr;
  //double prob_sum = 0; /* Ȯ���� ��, for normalization */

  if (result.empty()) return 0; /* ����� ������ ���� */

  res_itr = result.begin();
  double max_prob = res_itr->first; /* �ְ� Ȯ���� */

  /* �ݺ� */
  for (res_itr = result.begin(); res_itr != result.end(); ++res_itr) {

    if (cutoff_threshold > 0) { /* �� ���� 0 �̻��� ��쿡�� ����� */

      /* cut-off�� �������� �˻� */
      if (max_prob - res_itr->first > cutoff_threshold) { /* �ְ� Ȯ���� ���� ������� �αװ��� ���� ����ġ�̻��̸� ���� */
        result.erase(res_itr, result.end()); /* ���⼭���� ������������ ���� */
        break;
      }
    }
    //prob_sum += exp(res_itr->first); /* �α׸� ������ �� ���� ���Ѵ�. */
  }

  //prob_sum = log(prob_sum); /* �ٽ� �α׷� �ٲ۴�. */

  char result_str[MAX_WORD]; /* FIL�� ������ ��� */

  /* ��� ��ȯ �� ��� */
  for (res_itr = result.begin(); res_itr != result.end(); ++res_itr) {

    /* ������� FIL�� ���� */
    convert_str_origin_array((char *) res_itr->second.c_str(), result_str);

    /* ��� ���� (Ȯ�� + ���¼�/ǰ�� ��) */

    // ����ȭ
    //double prob = exp(res_itr->first - prob_sum); /* �αװ��̹Ƿ� ���� ���� �����δ� ������ ���� */
    //analyzed_result_s.push_back(make_pair(prob, (char *) result_str)); /* ���� */

    // ����ȭ���� ����
    analyzed_result_s.push_back(make_pair(exp(res_itr->first), (char *) result_str)); /* ���� */
    
  } /* end of for */

  return (int) analyzed_result_s.size(); /* �м��� ����� �� */
}


/******************************************************************************/
/* Ȯ���� ���¼� �м� (���� ����) */
/* ej : �Է� ���� */
/* analyzed_result_s : �м� ��� + Ȯ�� */
int prokoma_s(void *tag_s_fst, double *tag_s_prob,
              void *syllable_s_fst, double *syllable_s_prob,
              void *s_transition_fst,
              RESTORED_RESULT &restored_ej, /* ���� ������ ���� */
              RESTORED_STAGS &str_syl_tag_seq,
              ANALYZED_RESULT &analyzed_result_s, 
              double cutoff_threshold, int beam_size,
              char delimiter) {
   
  char rest_ej[MAX_WORD]; /* FIL ���ŵ� (�������) ���� */
  char splitchar[MAX_WORD][3]; /* �������� �и��� ������ ���ڸ� ��� 2����Ʈ�� ��ȯ�Ͽ� ���� */
  int num_splitchar = 0;

  char result[MAX_WORD]; /* ��� ���� */
  char *result_ptr = result;

  static char tag_sequence[MAX_WORD][MAX_TAG_LEN];

  ANALYZED_RESULT_MAP result_s;
  /*****************************************/

  RESTORED_RESULT::iterator it = restored_ej.begin();
  double max_restored_prob = it->first; // ù ���� ���� Ȯ��

  /* ��� ������ ������ ���� �ݺ� */
  for (it = restored_ej.begin(); it != restored_ej.end(); ++it) {

    convert_str_origin_array((char *) it->second.c_str(), rest_ej); /* FIL ���� */
    num_splitchar = split_by_char(rest_ej, &splitchar[2]);

        /* �� ���� ó�� */
/*    if (num_splitchar >= 40) {
      fprintf(stderr, "Error: too long word! [%s]\n", rest_ej);
      continue;
    }
*/
    /* ���� ���� 15���� ���� ������ ù��° ������ ������ ���ؼ��� ���� */
    if (num_splitchar >= 15) {
      if (it != restored_ej.begin()) break;
    }

    /* ���� ���� Ȯ������ �ʹ� ���� ��쳪 0�� ��쿡�� �м����� �ʴ´�. */
    if ((max_restored_prob - it->first) > cutoff_threshold || exp(it->first) == 0.0) {
      break;
    }

    ///**/fprintf(stderr, "������ ���� = %s\n", rest_ej);

    /* �ʱ�ȭ */
    strcpy (splitchar[0], BOW_SYL_2);   /* ���� ���� ���� */
    strcpy (splitchar[1], BOW_SYL_1);   /* ���� ���� ���� */

    strcpy (splitchar[num_splitchar+2], EOW_SYL);   /* ���� �� ���� */

    /*****************************************************************************/
    /* it->first = ������ Ȯ�� */
    /* it->second = one of ������ ������ */
    RESULT_S_MAP results[MAX_WORD]; /* �� time���� ����� ���� ���� */

    /* ���� �� ���¸� ����� (trigram) */
    trigram_breath_first_search(tag_s_fst, tag_s_prob,
                                syllable_s_fst, syllable_s_prob,
                                s_transition_fst,
                                str_syl_tag_seq[it->second /* ������ ���� */],
                                splitchar, num_splitchar, it->first /* Ȯ�� */, 
                                cutoff_threshold, beam_size,
                                results /*�������*/);

    /* ��� ������ ����� ���� */
    int start_time = 2;
    int end_time = num_splitchar+1;
    int count = 0;
    /* ���������� �ϳ� �� (���� �� ��������) */
    for (RESULT_S_MAP::iterator itr = results[end_time+1].begin(); 
         itr != results[end_time+1].end(); ++itr, count++) {

      if (count >= beam_size) break;

      /* ����� �±׵��� ���ڿ� �迭�� ���� */
      for (int i = start_time; i < (int) itr->second.size()-1/* -1 -> ���� �� �±׸� ���� */; i++) {
        strcpy(tag_sequence[i], itr->second[i].c_str());
      }
    
      /* ��� ���� (���� ������ �±�� ���� ���¼� ���� ����� ��ȯ) */
      if (!syllable2morpheme(result, num_splitchar, splitchar, tag_sequence, delimiter)) {
        ///**/fprintf(stderr, "%s\n", it->second.c_str());
        continue; /* ���� �߻��� ��� */
      }
    
      /* ��� ���� (Ȯ�� + ���¼�/ǰ�� ��) */
      result_s.insert(make_pair(itr->first, result_ptr));

    } /* end of for */
  } /* end of for */

  /* ��� ���� */
  int num_result = arrange_result_s(result_s, analyzed_result_s, cutoff_threshold);

  return num_result;
}
