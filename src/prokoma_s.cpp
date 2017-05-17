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
/* 리소스 열기 */
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

  /* 메모리 해제 */
  if (tag_s_prob) free (tag_s_prob);
  tag_s_prob = NULL;

  if (syllable_s_prob) free (syllable_s_prob);
  syllable_s_prob = NULL;
}

/******************************************************************************/
/* result_s에 저장된 결과를 정리하여 analyzed_result_s에 저장 */
/* 리턴값 : 분석 결과의 수 */
int arrange_result_s(ANALYZED_RESULT_MAP &result, ANALYZED_RESULT &analyzed_result_s, double cutoff_threshold) {

  ANALYZED_RESULT_MAP::iterator res_itr;
  //double prob_sum = 0; /* 확률의 합, for normalization */

  if (result.empty()) return 0; /* 결과가 없으면 종료 */

  res_itr = result.begin();
  double max_prob = res_itr->first; /* 최고 확률값 */

  /* 반복 */
  for (res_itr = result.begin(); res_itr != result.end(); ++res_itr) {

    if (cutoff_threshold > 0) { /* 이 값이 0 이상인 경우에만 사용함 */

      /* cut-off가 가능한지 검사 */
      if (max_prob - res_itr->first > cutoff_threshold) { /* 최고 확률을 갖는 결과보다 로그값의 차가 기준치이상이면 종료 */
        result.erase(res_itr, result.end()); /* 여기서부터 마지막까지는 삭제 */
        break;
      }
    }
    //prob_sum += exp(res_itr->first); /* 로그를 제거한 후 합을 구한다. */
  }

  //prob_sum = log(prob_sum); /* 다시 로그로 바꾼다. */

  char result_str[MAX_WORD]; /* FIL을 제거한 결과 */

  /* 결과 변환 후 출력 */
  for (res_itr = result.begin(); res_itr != result.end(); ++res_itr) {

    /* 결과에서 FIL을 제거 */
    convert_str_origin_array((char *) res_itr->second.c_str(), result_str);

    /* 결과 저장 (확률 + 형태소/품사 열) */

    // 정규화
    //double prob = exp(res_itr->first - prob_sum); /* 로그값이므로 빼는 것이 실제로는 나누는 것임 */
    //analyzed_result_s.push_back(make_pair(prob, (char *) result_str)); /* 삽입 */

    // 정규화하지 않음
    analyzed_result_s.push_back(make_pair(exp(res_itr->first), (char *) result_str)); /* 삽입 */
    
  } /* end of for */

  return (int) analyzed_result_s.size(); /* 분석된 결과의 수 */
}


/******************************************************************************/
/* 확률적 형태소 분석 (음절 단위) */
/* ej : 입력 어절 */
/* analyzed_result_s : 분석 결과 + 확률 */
int prokoma_s(void *tag_s_fst, double *tag_s_prob,
              void *syllable_s_fst, double *syllable_s_prob,
              void *s_transition_fst,
              RESTORED_RESULT &restored_ej, /* 음운 복원된 어절 */
              RESTORED_STAGS &str_syl_tag_seq,
              ANALYZED_RESULT &analyzed_result_s, 
              double cutoff_threshold, int beam_size,
              char delimiter) {
   
  char rest_ej[MAX_WORD]; /* FIL 제거된 (음운복원된) 어절 */
  char splitchar[MAX_WORD][3]; /* 어절에서 분리된 개개의 문자를 모두 2바이트로 변환하여 저장 */
  int num_splitchar = 0;

  char result[MAX_WORD]; /* 결과 저장 */
  char *result_ptr = result;

  static char tag_sequence[MAX_WORD][MAX_TAG_LEN];

  ANALYZED_RESULT_MAP result_s;
  /*****************************************/

  RESTORED_RESULT::iterator it = restored_ej.begin();
  double max_restored_prob = it->first; // 첫 음운 복원 확률

  /* 모든 복원된 어절에 대해 반복 */
  for (it = restored_ej.begin(); it != restored_ej.end(); ++it) {

    convert_str_origin_array((char *) it->second.c_str(), rest_ej); /* FIL 제거 */
    num_splitchar = split_by_char(rest_ej, &splitchar[2]);

        /* 긴 어절 처리 */
/*    if (num_splitchar >= 40) {
      fprintf(stderr, "Error: too long word! [%s]\n", rest_ej);
      continue;
    }
*/
    /* 음절 수가 15보다 많은 어절은 첫번째 복원된 어절에 대해서만 실행 */
    if (num_splitchar >= 15) {
      if (it != restored_ej.begin()) break;
    }

    /* 음운 복원 확률값이 너무 작은 경우나 0인 경우에는 분석하지 않는다. */
    if ((max_restored_prob - it->first) > cutoff_threshold || exp(it->first) == 0.0) {
      break;
    }

    ///**/fprintf(stderr, "복원된 어절 = %s\n", rest_ej);

    /* 초기화 */
    strcpy (splitchar[0], BOW_SYL_2);   /* 문장 시작 음절 */
    strcpy (splitchar[1], BOW_SYL_1);   /* 문장 시작 음절 */

    strcpy (splitchar[num_splitchar+2], EOW_SYL);   /* 문장 끝 음절 */

    /*****************************************************************************/
    /* it->first = 복원될 확률 */
    /* it->second = one of 복원된 어절들 */
    RESULT_S_MAP results[MAX_WORD]; /* 각 time마다 결과를 따로 저장 */

    /* 이전 두 상태를 고려함 (trigram) */
    trigram_breath_first_search(tag_s_fst, tag_s_prob,
                                syllable_s_fst, syllable_s_prob,
                                s_transition_fst,
                                str_syl_tag_seq[it->second /* 복원된 어절 */],
                                splitchar, num_splitchar, it->first /* 확률 */, 
                                cutoff_threshold, beam_size,
                                results /*결과저장*/);

    /* 모든 가능한 결과에 대해 */
    int start_time = 2;
    int end_time = num_splitchar+1;
    int count = 0;
    /* 마지막보다 하나 더 (어절 끝 음절까지) */
    for (RESULT_S_MAP::iterator itr = results[end_time+1].begin(); 
         itr != results[end_time+1].end(); ++itr, count++) {

      if (count >= beam_size) break;

      /* 저장된 태그들을 문자열 배열로 저장 */
      for (int i = start_time; i < (int) itr->second.size()-1/* -1 -> 어절 끝 태그만 빼고 */; i++) {
        strcpy(tag_sequence[i], itr->second[i].c_str());
      }
    
      /* 결과 생성 (음절 단위로 태깅된 것을 형태소 단위 결과로 변환) */
      if (!syllable2morpheme(result, num_splitchar, splitchar, tag_sequence, delimiter)) {
        ///**/fprintf(stderr, "%s\n", it->second.c_str());
        continue; /* 에러 발생한 경우 */
      }
    
      /* 결과 저장 (확률 + 형태소/품사 열) */
      result_s.insert(make_pair(itr->first, result_ptr));

    } /* end of for */
  } /* end of for */

  /* 결과 정리 */
  int num_result = arrange_result_s(result_s, analyzed_result_s, cutoff_threshold);

  return num_result;
}
