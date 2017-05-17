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
/* 리소스 열기 */
int prokoma_m_open(char *LEXICAL_PRB_Path, char *TRANSITION_PRB_Path, 
                   PROB_MAP &lexical_prob, PROB_MAP &transition_prob) {

  /* 확률을 입력 */
  /* 어휘 확률 */
  fprintf(stderr, "\tReading lexical probabilities.. [%s]", LEXICAL_PRB_Path);
  if (!map_scan_probability(LEXICAL_PRB_Path, lexical_prob, "t")) {
    fprintf(stderr, "\t[ERROR] can't read the lexical probabilities! [%s]\n", LEXICAL_PRB_Path);
    return 0;
  }
  fprintf(stderr, "\t[done]\n");

  /* 전이 확률 */
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
/* 실제 형태소 분석 모듈 */
// CYK 알고리즘
int cyk_m(PROB_MAP &transition_prob, /* 전이 확률 */
              SUB_STRING_INFO &substr_info, /* 부분 문자열의 사전 정보 */
              int which_ej, /* 몇 번째 어절인가? */
              SUB_STRING &sub_str, /* 부분 문자열 */
              int len, /* 음절 수 */
              double phonetic_change_prob /* 음운 복원 확률 */, 
              ANALYZED_RESULT_MAP &analyzed_result_m, /* 결과 저장 */
              double cutoff_threshold, 
              double threshold2,
              char delimiter) {

  int i, j;
  int cur_tab; // 현재
  int front_tab; // 앞쪽

  SUB_STRING_INFO table; // 부분 결과 저장

  SUB_STRING_INFO::iterator it, it2;
  BI_STATE_MAP::iterator iter, iter2;
  
  double trans_prob, trans_prob2; // 전이확률

  static double max_prob; // 최대 확률
  double cur_prob;

  if (which_ej == 0) max_prob = -LONG_MAX;

  /*
  영화전문
  영(0), MAG, -8.18
  영(0), NNG, -9.88
  영(0), NNP, -8.24
  영(0), NR, -8.00
  영화(1), NNG, -7.32
  화(4), NNG, -8.66
  화(4), NNP, -11.50
  화(4), XSN, -2.91
  화전(5), NNG, -10.32
  전(7), MM, -3.72
  전(7), NNB, -9.56
  전(7), NNG, -6.35
  전(7), NNP, -6.30
  전문(8), NNG, -7.65
  문(9), NNB, -10.86
  문(9), NNG, -7.83
  문(9), NNP, -7.94
  */

  t_TAB pos;

  const char delim[2] = {delimiter, 0};

  /* base case */
  // 사전에 저장된 형태소들의 품사와 확률을 알아내어 테이블에 저장
  for (it = substr_info.begin(); it != substr_info.end(); it++) {
    
    TabPos1to2(it->first, len, &pos);
      ///**/fprintf(stderr, "(%d, %d)\n", pos.x, pos.y);

    // 모든 품사에 대해
    for (iter = it->second.begin(); iter != it->second.end(); iter++) {

      ///**/fprintf(stderr, "%s(%d), %s, %.2lf\n", sub_str[it->first].c_str(), it->first, iter->first.c_str(), iter->second);

      if (pos.x == 0) { // 어절의 시작부분이면

        // 어절 처음 태그와의 연결성 검사
        // 연결 불가능하면
        if ((trans_prob = transition_prob[BOW_TAG_1/*이전품사*/][iter->first/*현재품사*/]) <= 0.0) {
          ///**/fprintf(stderr, "불가 %s -> %s\n", BOW_TAG_1, iter->first.c_str());
          continue;
        }

        if (pos.y == len) { // 어절 전체가 하나의 형태소이면

          // 어절 끝 태그와의 연결성 검사
          // 연결 불가능하면
          if ((trans_prob2 = transition_prob[iter->first/*현재품사*/][EOW_TAG/*마지막품사*/]) <= 0.0) {
            ///**/fprintf(stderr, "불가 %s -> %s\n", iter->first.c_str(), EOW_TAG);
            continue;
          }

          // 어휘 확률 + 전이확률 + 음운복원 확률 + 전이확률
          cur_prob = iter->second + log(trans_prob) + phonetic_change_prob + log(trans_prob2);
          
          // 최대 확률 갱신
          if (cur_prob > max_prob) max_prob = cur_prob;
        }

        else { // 어절의 시작부분 and 전체가 하나의 형태소가 아님
          // 어휘 확률 + 전이확률 + 음운복원 확률
          cur_prob = iter->second + log(trans_prob) + phonetic_change_prob; 
        }
      }
      else { // 어절의 중간부분이면
        cur_prob = iter->second; // 어휘 확률
      }
      
      /* cut-off가 가능한지 검사 */
      if (cutoff_threshold > 0 && max_prob - cur_prob > cutoff_threshold) {
        ///**/fprintf(stderr, "max_prob %lf - cur_prob %lf = %lf\n", max_prob, cur_prob, max_prob-cur_prob);
        ///**/fprintf(stderr, ">\n");
        continue; // 여기서 중지
      }

      // table에 저장
      // table[인덱스][형태소/품사] = 확률
      table[it->first][sub_str[it->first]+delimiter+iter->first] = cur_prob;

      ///**/fprintf(stderr, "%d, %s/%s\n", it->first, sub_str[it->first].c_str(), iter->first.c_str());
      
    }
  }

  char result[1000];
  char *prev_last_pos;
  //for (j = 2; j <= len; j++) {
  //  for (i = j-1; i >= 1; i--) {

  // T(0, i) : 앞쪽
  // T(i, j) : 현재
  // T(0, j) : 합친 결과
  for (i = 1; i < len; i++) {
    for (j = i+1; j <= len; j++) {

      // 앞부분 + 현재부분
      ///**/fprintf(stderr, "%s + %s\n", sub_str[TabPos2to1(0, i, len)].c_str(), sub_str[TabPos2to1(i, j, len)].c_str());

      double cur_max = -LONG_MAX;

      // 현재부분이 사전에 등록되어 있으면
      cur_tab = TabPos2to1(i, j, len); // 현재
      it = substr_info.find(cur_tab);
      
      if ( it == substr_info.end() ) { // 없으면
        continue;
      }
      
      // 앞부분의 기분석 결과가 있으면
      front_tab = TabPos2to1(0, i, len); // 앞쪽
      it2 = table.find(front_tab);

      if ( it2 == table.end() ) { // 없으면
        continue;
      }

      // 앞부분 (모든 분석 결과에 대해)
      for (iter = it2->second.begin(); iter != it2->second.end(); ++iter) {

        // 이전 분석의 마지막 품사
        prev_last_pos = strRstr((char *)iter->first.c_str(), delim) + 1;

        // 현재부분 (모든 품사에 대해)
        for (iter2 = it->second.begin(); iter2 != it->second.end(); ++iter2) {

          // 결합 가능성 검사
          ///**/fprintf(stderr, "\n'%s'의 현재 품사 = %s\n", sub_str[cur_tab].c_str(), iter2->first.c_str()); /* 현재 형태소의 품사 */
          ///**/fprintf(stderr, "이전 분석 = %s\n", iter->first.c_str());

          ///**/fprintf(stderr, "이전 분석의 마지막 품사 = %s\n", prev_last_pos);

          /* 이전 품사와의 연결성 검사 */
          // 전이확률 : 이전 분석의 마지막 품사 + 현재 형태소의 품사 */
          if ((trans_prob = transition_prob[prev_last_pos][iter2->first]) <= 0.0) { /* 연결 불가능하면 */ /* 확인해 볼 것! */
            ///**/fprintf(stderr, "불가 %s -> %s\n", prev_last_pos, iter2->first.c_str());
            continue;
          }

          // 연속해서 붙을 수 없는 태그
          if (strcmp(prev_last_pos, iter2->first.c_str()) == 0) {
            if (strcmp(prev_last_pos, "SL") == 0 
                || strcmp(prev_last_pos, "SH") == 0 
                || strcmp(prev_last_pos, "SN") == 0
                || strcmp(prev_last_pos, "SE") == 0) {
              continue;
            }
          }

          // 여기까지 온 경우는 연결가능한 경우임
          // 분석결과 및 확률 저장
          // 이전 분석 결과 + 현재 형태소/품사
          sprintf(result, "%s+%s%c%s", iter->first.c_str(), sub_str[cur_tab].c_str(), delimiter, iter2->first.c_str());
          
          // 어절 끝인가?
          if (j == len) {
            /* 어절끝 품사와의 연결성 검사 */
            if ((trans_prob2 = transition_prob[iter2->first][EOW_TAG]) <= 0.0) {
              ///**/fprintf(stderr, "불가 %s -> %s\n", iter2->first.c_str(), EOW_TAG);
              continue;
            }
              
            // 이전 분석 확률 + 전이 확률 + 어휘 확률 + (마지막 품사와 EOS간의) 전이 확률 
            cur_prob = iter->second + log(trans_prob) + iter2->second + log(trans_prob2);

            if (cur_prob > max_prob) max_prob = cur_prob; // 최대확률 갱신
          }
          
          else { // 어절 끝이 아니면
            // 이전 분석 확률 + 전이 확률 + 어휘 확률
            cur_prob = iter->second + log(trans_prob) + iter2->second;
          }
            
          /* cut-off가 가능한지 검사 */
          if (cutoff_threshold > 0 && max_prob - cur_prob > cutoff_threshold) {
            continue; /* 여기서 중지 */
          }

          /* 음절수에 비례하여 정한 threshold 보다 확률값이 작으면 */
          #ifdef SYLLABLE_ANALYSIS // 뒤에서 음절 단위 분석을 하는 경우만 검사
          if (cur_prob < threshold2) { 
            continue;
          }
          #endif

          // 속도 향상에 상당한 기여, but 누락되는 결과들이 생길 수도 있다.
          if (cur_max - cur_prob > 15) { // 10을 써보기도 했었음
            continue;
          }
          if (cur_max < cur_prob) cur_max = cur_prob;

          // 저장
          table[TabPos2to1(0, j, len)][result] = cur_prob;

          ///**/fprintf(stderr, "(0, %d) %s\n", j, result);

          ///**/fprintf(stderr, "%s\t%12.11e\n", result, table[TabPos2to1(0, j, len)][result]);

          ///**/fprintf(stderr, "\n");

        } // end of for 현재부분 (모든 품사에 대해)
      } // end of for 앞부분 (모든 분석 결과에 대해)
    } // end of for (i = j-1; i >= 1; i--)
  } // end of for (j = 2; j <= len; j++)

  // (전체 어절에 대한) 최종 분석 결과 /////////////////////////////////////////////
  cur_tab = TabPos2to1(0, len, len);
  max_prob = -LONG_MAX;

  for (iter = table[cur_tab].begin(); iter != table[cur_tab].end(); ++iter) {

    // 최대 확률 갱신
    if (iter->second > max_prob) max_prob = iter->second;

    if (cutoff_threshold > 0 && max_prob - iter->second > cutoff_threshold) continue;

    /* 결과 저장 (확률 + 분석결과) */
    analyzed_result_m.insert(make_pair(iter->second, iter->first));
  }

  return 1;
}

/******************************************************************************/
/* 실제 형태소 분석 모듈 */
// 깊이 우선 탐색
int depth_first_m(PROB_MAP &transition_prob, 
                  SUB_STRING_INFO &substr_info, 
                  int which_ej, /* 몇 번째 어절인가? */
                  SUB_STRING sub_str, int len,  /* 여기까지는 불변 */
                  t_TAB position, const char *prev_pos, int depth, double path_prob, char *result, 
                  ANALYZED_RESULT_MAP &analyzed_result_m, double cutoff_threshold, double threshold2,
                  char delimiter) {

  static double max_prob; /* 지금까지 분석된 결과중 가장 높은 확률값 (for cutoff) */

  int j;
  t_TAB head, tail;
 
  BI_STATE_MAP::iterator itr = NULL;
  int cur_tab;
  double trans_prob; /* 전이 확률 */
  double path_prob_backup;

  char *cur_result = result + strlen(result);
  
  char one_result[500]; /* 임시 */
  
  path_prob_backup = path_prob; /* 이전 경로 확률 저장 (반드시 해야 함) */
  *cur_result = 0;


  /* 초기화 (제일 처음인 경우에만) */
  if (depth == 0 && which_ej == 0) max_prob = -LONG_MAX;

  /* 문자열 크기 별로 반복 */       	
  //for (j = position.x+1; j <= len; j++) { /* 최단우선 */
  for (j = len; j > position.x; j--) { /* 최장우선 */
  	
  	cur_tab = TabPos2to1(position.x, j, len);
 		
  	/* 사전 탐색 */
  	/* 만약 사전에 있으면 모든 품사에 대해 반복 */
    for (itr = substr_info[cur_tab].begin(); itr != substr_info[cur_tab].end(); ++itr ) { /* 있으면 */
      
      /* 이전 품사와의 연결성 검사 */
      if ((trans_prob = transition_prob[prev_pos][itr->first]) <= 0.0) { /* 연결 불가능하면 */ /* 확인해 볼 것! */
        continue;
      }

      /* 확률 갱신 : 이전 경로 확률, (이전에서 현재로의) 전이 확률, 어휘 확률 */
      path_prob = path_prob_backup + log(trans_prob) + itr->second; 

      /* cut-off가 가능한지 검사 */
      if (cutoff_threshold > 0 && max_prob - path_prob > cutoff_threshold) {
        continue; /* 여기서 중지 */
      }

      /**/
      /* 음절수에 비례하여 정한 threshold 보다 확률값이 작으면 */
      #ifdef SYLLABLE_ANALYSIS // 뒤에서 음절 단위 분석을 하는 경우만 검사
      if (path_prob < threshold2) { 
        continue;
      }
      #endif
      
      /* head / tail을 변경한 후에 재귀적으로 호출 */
      setpos(&tail, j, len);
      setpos(&head, position.x, j); /* tail의 이전(앞)부분 */

      /* 종료 조건 검사 */
      if (is_empty(tail)) { /* tail을 가지고 더 이상 분석할 수 없을 때 */

        /* 문장끝 품사와의 연결성 검사 */
        if ((trans_prob = transition_prob[itr->first][EOW_TAG]) > 0.0) { /* 확인해 볼 것! */
        
          /* 확률 갱신 : (마지막 품사와 EOS간의) 전이 확률 */
          path_prob += log(trans_prob);
          
          /* cut-off가 가능한지 검사 */
          if (cutoff_threshold > 0 && max_prob - path_prob > cutoff_threshold) {
            continue; /* 여기서 중지 */
          }

          /**/
          /* 음절수에 비례하여 정한 threshold 보다 확률값이 작으면 */
          #ifdef SYLLABLE_ANALYSIS // 뒤에서 음절 단위 분석을 하는 경우만 검사
          if (path_prob < threshold2) {
            continue;
          }
          #endif

          /* 형태소/품사 저장 */
          if (depth) sprintf(one_result, "+%s%c%s", sub_str[cur_tab].c_str(), delimiter, itr->first.c_str());
          else sprintf(one_result, "%s%c%s", sub_str[cur_tab].c_str(), delimiter, itr->first.c_str());
          *cur_result = 0;
          strcat(cur_result, one_result);
          
          /* 결과 저장 (형태소/품사 열 + 확률) */
          analyzed_result_m.insert(make_pair(path_prob, result));
            
          if (path_prob > max_prob) {
            max_prob = path_prob; /* 최대값보다 크면 최대값이 됨 */
          }
        }
      }
      else { /* 종료 조건을 만족시키지 못한 경우 */

        /* 형태소/품사 저장 */
        if (depth) {
          sprintf(one_result, "+%s%c%s", sub_str[TabPos2to1(head.x, head.y, len)].c_str(), delimiter, itr->first.c_str());
        }
        else {
          sprintf(one_result, "%s%c%s", sub_str[TabPos2to1(head.x, head.y, len)].c_str(), delimiter, itr->first.c_str());
        }
        *cur_result = 0;
        strcat(cur_result, one_result); /* 결과 저장 */
          
        /* 재귀적으로 호출 */
        if (!depth_first_m(transition_prob, substr_info, which_ej, sub_str, len, 
                  tail, itr->first.c_str(), depth+1, path_prob, result, 
                  analyzed_result_m, cutoff_threshold, threshold2, delimiter)) return 0;
      }
    } /* end of for */
  } /* end of for */
  
  return 1;
}

/******************************************************************************/
/* 모든 부분 문자열을 미리 사전에서 찾는다. */
// 출력 : substr_info, sub_str
int get_all_possible_pos_from_substring(int len, char *source_str, PROB_MAP &lexical_prob, 
                                        SUB_STRING_INFO &substr_info, 
                                        SUB_STRING &sub_str) {

  PROB_MAP::iterator itr;
  NUM_PROB::iterator itr2;

  int i, j, k;

  char str[MAX_WORD];

  /* 반드시 초기화해야 함 */
  // 음운복원 후 같은 변수에 다시 저장을 하기 때문 (초기화를 하지 않으면 이전 데이타로 인해 문제 발생)
  sub_str.clear(); 
  substr_info.clear();

  // 부분 문자열 저장
  for (k = i = 0; i < len; i++) {

    for (j = i; j < len; j++, k++) {

      strncpy(str, source_str+i*2, (j-i+1)*2);
      str[(j-i+1)*2] = 0;
      ///**/fprintf(stderr, "[%d] %s\n", k, str);
      sub_str.push_back(str); // 삽입

      itr = lexical_prob.find(str); /* 부분 문자열을 사전에서 찾는다. */

      if ( itr != lexical_prob.end() ) { /* 있으면 */

        /* 취할 수 있는 모든 품사를 찾는다. */
        for (itr2 = lexical_prob[itr->first].begin(); itr2 != lexical_prob[itr->first].end(); ++itr2) {
          substr_info[k][itr2->first/*품사*/] = log(itr2->second); /* 어휘 확률 */
        }
      } // end of if
    } // end of for
  } // end of for

  return 1;
}

/******************************************************************************/
/* result_m에 저장된 결과를 정리하여 analyzed_result_m에 저장 */
/* 리턴값 : 분석 결과의 수 */
int arrange_result_m(ANALYZED_RESULT_MAP &result_m, ANALYZED_RESULT &analyzed_result_m, RESTORED_STAGS &str_syl_tag_seq, double cutoff_threshold, char delimiter) {

  ANALYZED_RESULT_MAP::iterator res_itr;
  //double prob_sum = 0.0; /* 확률의 합, for normalization */
  int num_result = 0;

  int num_morph = 0;

  if (result_m.empty()) return 0; /* 결과가 없으면 종료 */

  /* 확률 normalization **********************************************/
  res_itr = result_m.begin();
  double max_prob = res_itr->first; /* 최고 확률값 */


  /* 모든 분석 결과에 대해 */
  /* res_itr->first : 확률값 */
  /* res_itr->second : 형태소 분석결과 */
  for (res_itr = result_m.begin(); res_itr != result_m.end(); ) {

    /* cut-off가 가능한지 검사 */
    if (cutoff_threshold > 0) { /* 이 값이 0 이상인 경우에만 사용함 */
  
      /* 최고 확률을 갖는 결과보다 로그값의 차가 기준치이상이면 종료 */
      if (max_prob - res_itr->first > cutoff_threshold) {
        result_m.erase(res_itr, result_m.end()); /* 여기서부터 마지막까지는 삭제 (나머지 결과의 확률값은 더 작으므로) */
        break;
      }
    }

    /* 생성된 태그열의 정당성 검사 */
    if (!(num_morph = check_morpheme_result((char *) res_itr->second.c_str(), str_syl_tag_seq, delimiter))) {

      /* 맵에서 제거하는 것이 이상하게 느리다. */
      result_m.erase(res_itr++); /* 제거 */
      continue; /* do nothing */
    }

    //prob_sum += exp(res_itr->first); /* 로그를 제거한 후 합을 구한다. */

    res_itr++; /* 증가 */
  } /* end of for */

  //prob_sum = log(prob_sum); /* 다시 로그로 바꾼다. */

  /* 결과 변환 **********************************************/
  char result_str[MAX_WORD]; /* FIL을 제거한 결과 */

  /* 모든 분석 결과에 대해 */
  for (res_itr = result_m.begin(); res_itr != result_m.end(); ++res_itr) {

    /* 결과에서 FIL을 제거 */
    convert_str_origin_array((char *) res_itr->second.c_str(), result_str);

    /* 결과 저장 (확률 + 형태소/품사 열) */

    // 정규화
//    double prob = exp(res_itr->first - prob_sum); /* 로그값이므로 빼는 것이 실제로는 나누는 것임 */
//    analyzed_result_m.push_back(make_pair(prob, (char *) result_str));
    
    // 정규화하지 않음
    analyzed_result_m.push_back(make_pair(exp(res_itr->first), (char *) result_str));

    
    num_result++;
  } /* end of for */

  return num_result; /* 분석된 결과의 수 */
}

/******************************************************************************/
/* 확률적 형태소 분석 (형태소 레벨) */
/* input_ej : 입력 어절 */
/* analyzed_result_m : 분석 결과 + 확률 */
int prokoma_m(PROB_MAP &transition_prob, PROB_MAP &lexical_prob,  
              RESTORED_RESULT &restored_ej,
              RESTORED_STAGS &str_syl_tag_seq,
              ANALYZED_RESULT &analyzed_result_m, double cutoff_threshold, char delimiter) {
   
  int len;  
  t_TAB position; /* 문자열의 위치값 */

  SUB_STRING_INFO substr_info; // 각 부분 문자열들의 품사 정보를 저장

  SUB_STRING sub_str; // 부분 문자열 저장

  sub_str.reserve(TabNum(30)); // capacity를 위해

  ANALYZED_RESULT_MAP result_m;
  /*****************************************/

  RESTORED_RESULT::iterator it = restored_ej.begin();
  double max_restored_prob = it->first; // 첫 음운 복원 확률

  /* 모든 복원된 어절에 대해 반복 */
  int i; // 복원된 어절의 번호
  for (it = restored_ej.begin(), i = 0; it != restored_ej.end(); ++it, i++) {
    
    /* 음절 수가 15보다 많은 어절은 첫번째 복원된 어절에 대해서만 실행 */
    if (strlen(it->second.c_str()) >= 15 * 2) {
      if (it != restored_ej.begin()) break;
    }

    /* 음운 복원 확률값이 너무 작은 경우나 0인 경우에는 분석하지 않는다. */
    if (exp(it->first) == 0.0 || (cutoff_threshold && (max_restored_prob - it->first) > cutoff_threshold) ) {
      ///**/if (exp(it->first) == 0.0) fprintf(stderr, "haha\n");
      ///**/else fprintf(stderr, "hehe\n");
      break;
    }


    //#define DEBUG_PHONETIC
    //#ifdef DEBUG_PHONETIC /********************************************/
    //fprintf(stderr, "[%d]%s : %lf\n", i, it->second.c_str(), it->first);
    //#endif /***********************************************************/

    len = (int) strlen(it->second.c_str()) / 2; // 문자열 길이
    if (len > 30) { // 30음절 초과이면 CYK 알고리즘이 너무 비효율적이므로 분석 하지 않음
      continue;
    }

    //**/fprintf(stderr, "len = %d\n", len);
  
    /* 초기화 *****************************************************/
    setpos(&position, 0, len); // 위치 지정
	  
    /* 부분 문자열 저장 */
    /* 모든 부분 문자열을 미리 사전에서 찾는다. */
    /* 입력 : sub_str (부분 문자열 집합) */
    /* 출력 : substr_info (각 부분 문자열이 취할 수 있는 품사와 확률의 열 저장) */
    get_all_possible_pos_from_substring(len, (char *) it->second.c_str(), 
                                        lexical_prob, substr_info, sub_str);

    // 음운복원된 어절 출력
    ///**/fprintf(stderr, "%s\n", it->second.c_str());

    /* 형태소 분석 모듈 */
    cyk_m(transition_prob, 
          substr_info, i, sub_str, len, 
          it->first, // 로그 확률 (초기값)
          result_m, cutoff_threshold, log(pow(1.0e-03, len)), delimiter);

/*    result[0] = 0;
    char result[MAX_WORD] = {0,}; // 결과 저장 
    depth_first_m(
                  transition_prob, 
                  substr_info, i, sub_str, len, 
                  position, BOW_TAG_1, 0, it->first, // 로그 확률 (초기값)
                  result, result_m, cutoff_threshold, log(pow(1.0e-03, len)), delimiter);
*/
  } /* end of for */

  /* 결과 정리 */
  int num_result = arrange_result_m(result_m, analyzed_result_m, 
                                    str_syl_tag_seq, cutoff_threshold, delimiter);

  return num_result;
}
