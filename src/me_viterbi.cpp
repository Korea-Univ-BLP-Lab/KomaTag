#ifndef WIN32

#include <stdio.h>
#include <math.h>               /* log () */
#include <limits.h>               /* LONG_MAX */

#include "definitions.h"
#include "probability_tool.h"
#include "hsplit.h"
#include "get_morph_tag.h"

#include "probability_tool.h" // for ALMOST_ZERO

//#define _DEBUG_


#include <maxent/maxentmodel.hpp>
//#include "maxentmodel.hpp"

using namespace maxent;

/* 타입 선언 */
typedef struct {
  double lexical_prob; /* (현재 상태에 대한) 형태소 분석 확률 */
  double path_prob; /* (현재 상태까지의 경로에 대한) 확률 */
  int prev_state; /* (최대 확률 경로를 가진) 이전 상태(어절) */

//  string head_morph;
  string head_tag;
//  string tail_morph;
//  string tail_tag;

  string first_tag; // 첫번째 태그
  string last_tag;  // 마지막 태그
  string second_last_tag; // 마지막-1 태그

  string last_two_tag; // 마지막 두 태그

} ME_BI_STRUCT_PER_STATE;

typedef map<int, ME_BI_STRUCT_PER_STATE> ME_BIGRAM_STATE_MAP; /* 태그, 확률, 이전상태 */


/*****************************************************************************/
// 머리 형태소와 꼬리 형태소의 위치를 찾아낸다.
// 결과는 head_pos와 tail_pos에 저장된다.
int find_head_tail_morph(WORD_FREQ &full_morpheme_map, 
                         int morph_num, char morphs[][MAX_WORD], char tags[][MAX_WORD],
                         int *head_pos, int *tail_pos) {

  int i;

  WORD_FREQ::iterator it;

  // 초기화
  *head_pos = 0; // 첫 형태소
  *tail_pos = morph_num-1; // 마지막 형태소

  for (i = morph_num-1; i >= 0; i--) {
    it = full_morpheme_map.find(tags[i]); // 찾아보고
    
    if (it != full_morpheme_map.end()) { // 있으면
      *head_pos = i;
      break; // 끝에서부터 찾자마자 종료
    }
  }

//  if (*head_pos == *tail_pos)  // 머리와 꼬리의 위치가 같으면 꼬리는 필요없을지도...
  //*tail_pos = -1; // 없다.

  return 1;
}

/*****************************************************************************/
/* 전이 확률, 어휘 확률, 문장, 단어의 수, 태그집합, 상태열(결과 저장) */
void me_viterbi_search (MaxentModel &head_m, MaxentModel &tail_m,
                        MaxentModel &dummy_head_m, 
                        map<string, int> &full_morpheme_map, 
                        vector<ANALYZED_RESULT> &morph_analyzed_result, int total_time,
                        int *state_sequence, char delimiter) {
  int i;

#ifdef WIN32
  ME_BIGRAM_STATE_MAP states[500];
#else
  ME_BIGRAM_STATE_MAP states[total_time+1];
#endif

  int max_prev_state;
  double max_path_prob;
  double cur_path_prob;

  PROB_MAP::iterator itr;
  NUM_PROB::iterator itr2;

  ME_BIGRAM_STATE_MAP::iterator curt_itr; /* 현재 태그에 대한 반복자 */
  ME_BIGRAM_STATE_MAP::iterator prevt_itr; /* 이전 태그에 대한 반복자 */

  int j;

  /* 변수의 값을 계속 유지하기 위해서가 아니라  */
  /* 함수 호출시 매번 변수 생성에 걸리는 시간때문에 static으로 선언함 */
  int morph_num = 0; /* 어절 내의 형태소 수 */
  char morphs[MAX_WORD][MAX_WORD] = {0}; /* 형태소 열 */
  static char tags[MAX_WORD][MAX_WORD] = {0}; /* 품사 태그 열 */
  static int spacing_tags[MAX_WORD] = {0}; /* 띄어쓰기 태그 열 */


  /* 초기화 *****************************************************/
  states[0][0].path_prob = 0.0; /* log (1) */
  
  //states[0][0].head_morph = BOSTAG_1;
  states[0][0].head_tag = BOSTAG_1;
  //states[0][0].tail_morph = BOSTAG_1;
  //states[0][0].tail_tag = BOSTAG_1;

  states[0][0].first_tag = BOSTAG_1;
  states[0][0].last_tag = BOSTAG_1;
  states[0][0].second_last_tag = BOSTAG_1;

  states[0][0].last_two_tag = BOSTAG_1;

  // 모든 어절에 대해
  for (i = 1; i <= total_time; i++) {

    for (j = 0; j < (int) morph_analyzed_result[i].size(); j++) {
      states[i][j].path_prob = -LONG_MAX;
      states[i][j].lexical_prob = log(morph_analyzed_result[i][j].first); /* 형태소분석 확률 */
    }

    for (j = 0; j < (int) morph_analyzed_result[i].size(); j++) {

      ///**/fprintf(stderr, "i = %d, j = %d\n", i, j);
      ///**/fprintf(stderr, "%s\n", morph_analyzed_result[i][j].second.c_str());
      get_morphs_tags((char *)morph_analyzed_result[i][j].second.c_str(), &morph_num,
                      morphs, tags, spacing_tags, delimiter);

      if (morph_num < 1) { fprintf(stderr, "number of morpheme is 0\n"); exit(1); }

      int head_pos, tail_pos;

      // 머리 형태소와 꼬리 형태소 찾기
      find_head_tail_morph(full_morpheme_map, morph_num, morphs, tags, &head_pos, &tail_pos);

//      states[i][j].head_morph = morphs[head_pos]; /* 머리 형태소 */
      states[i][j].head_tag   = tags[head_pos];   /* 머리 태그 */
//      states[i][j].tail_morph = morphs[tail_pos]; /* 꼬리 형태소 */
//      states[i][j].tail_tag   = tags[tail_pos];   /* 꼬리 태그 */

      states[i][j].first_tag  = tags[0];   // 첫번째 태그
      states[i][j].last_tag   = tags[morph_num-1];   // 마지막 태그
      states[i][j].second_last_tag = (morph_num == 1) ? BOW_TAG_1 : tags[morph_num-2];   // 마지막-1 태그
      states[i][j].last_two_tag = states[i][j].second_last_tag + states[i][j].last_tag; // 마지막 두 태그

      ///**/fprintf(stderr, "tm = %s, ht = %s, tm = %s, tt = %s\n", states[i][j].head_morph.c_str(), states[i][j].head_tag.c_str(), states[i][j].tail_morph.c_str(), states[i][j].tail_tag.c_str());

    }  /* end of for */
  }

  /* Iteration Step ****************************************************/
  /* 각 time(token) 마다 */
  for (i = 1; i <= total_time; i++) {

    /* 현재 상태(태그)에 대해 */
    for (curt_itr = states[i].begin(); curt_itr != states[i].end(); ++curt_itr) {

      max_path_prob = -LONG_MAX;       /* log(0.0) *//* 초기화 */

      /* 이전 상태(태그)에 대해 */ // 사실.... 이전 상태가 하나뿐이라면 선택의 여지가 없지 않는가... (시간을 줄일 수도 있다는 의미)
      for (prevt_itr = states[i-1].begin(); prevt_itr != states[i-1].end(); ++prevt_itr) { 

        #ifdef _DEBUG_ /********************************************************/
        fprintf (stdout, "-----------------------------\n");
        fprintf (stdout, "get_probability([%d], prev_tag = %s, cur_tag = %s)\n",
          //morph_analyzed_result[i][j].second.c_str()
                  i, prevt_itr->second.last_tag.c_str(), curt_itr->second.first_tag.c_str());
        #endif  /********************************************************/

        vector <string> context;
        vector <string> dummy_context;
        char  dummy_predicate[100];

        context.clear();
        dummy_context.clear();

        //context.push_back("hm="+prevt_itr->second.head_morph);

        // 머리 태그
        context.push_back("ht="+prevt_itr->second.head_tag);
        sprintf(dummy_predicate, "ht=%s", BOW_TAG_1);
        dummy_context.push_back(dummy_predicate);

        //context.push_back("tm="+prevt_itr->second.tail_morph);
        //context.push_back("tt="+prevt_itr->second.tail_tag);

        // 마지막 태그
        context.push_back("lt="+prevt_itr->second.last_tag); 
        sprintf(dummy_predicate, "lt=%s", BOW_TAG_1);
        dummy_context.push_back(dummy_predicate);

        // 마지막-1 태그
        context.push_back("lt-1="+prevt_itr->second.second_last_tag); 
        sprintf(dummy_predicate, "lt-1=%s", BOW_TAG_1);
        dummy_context.push_back(dummy_predicate);

        // 마지막 두 태그
        context.push_back("l2t="+prevt_itr->second.last_two_tag); 
        sprintf(dummy_predicate, "l2t=%s", BOW_TAG_1);
        dummy_context.push_back(dummy_predicate);

        // 태그 전이 확률 구하기

        // 형태소/태그
        //double logprob_head_transition = log(head_m.eval(context, curt_itr->second.head_morph+"/"+curt_itr->second.head_tag));

        // 첫번째 태그로 전이
        double logprob_transition       = log(head_m.eval(context, curt_itr->second.first_tag)); 

        // 첫번째 태그로 전이 (분모)
        double logprob_dummy_transition = log(head_m.eval(dummy_context, curt_itr->second.first_tag));

        // 꼬리 태그로 전이
        //double logprob_tail_transition = log(tail_m.eval(context, curt_itr->second.tail_tag)); 

        ///**/fprintf(stderr, "%s <- context = %s %s %s: ", curt_itr->second.head_tag.c_str(), context[0].c_str(), context[1].c_str(), context[2].c_str());
        ///**/fprintf(stderr, "%lf\n", exp(logprob_head_transition));

        // 품사만
        /*if (logprob_head_transition < LOG_ALMOST_ZERO)
          logprob_head_transition = log(head_m.eval(context, curt_itr->second.head_tag));

        if (logprob_tail_transition < LOG_ALMOST_ZERO)
          logprob_tail_transition = log(tail_m.eval(context, curt_itr->second.tail_morph+"/"+curt_itr->second.tail_tag));
*/
        
        // 너무 작은 확률인 경우
        if (logprob_transition < LOG_ALMOST_ZERO) {
          //**/fprintf(stderr, ".");
          logprob_transition = LOG_ALMOST_ZERO;
        }

        if (logprob_dummy_transition < LOG_ALMOST_ZERO) {
          //**/fprintf(stderr, "*");
          logprob_dummy_transition = LOG_ALMOST_ZERO;
        }

        ///**/fprintf(stderr, "%lf, %lf\n", logprob_head_transition, logprob_tail_transition);
        
        /* 확률 계산 */
        cur_path_prob = prevt_itr->second.path_prob /* 이전 상태까지의 경로에 대한 확률 */ /*SeqPrb[i - 1][k]*/ 
          + logprob_transition // 전이확률 (머리)

#ifdef USING_DENOMINATOR
          - logprob_dummy_transition // 전이확률 분모 (dummy 머리) // 빼야 함
#endif
          + curt_itr->second.lexical_prob; // 어휘 확률 (형태소 분석 확률)

        #ifdef _DEBUG_ /********************************************************/
        fprintf (stdout, "prev sequence + transition + lexical = %lf + %lf + %lf = %lf\n", 
          prevt_itr->second.path_prob, 
          log ( map_get_probability2(inter_transition_prob, curt_itr->second.first_tag, prevt_itr->second.last_tag) ), 
          curt_itr->second.lexical_prob, 
          cur_path_prob);
        #endif  /********************************************************/
        
        if (max_path_prob < cur_path_prob) {
          max_path_prob = cur_path_prob;
          max_prev_state = prevt_itr->first;
        }
      }

      curt_itr->second.path_prob = max_path_prob; /* arg ; 최대 확률 */
      curt_itr->second.prev_state = max_prev_state; /* argmax ; 이전 상태 */

      #ifdef _DEBUG_ /********************************************************/
      fprintf (stdout, "max prob = %lf, max prev state = %d\n",
               curt_itr->second.path_prob, curt_itr->second.prev_state);
      #endif /****************************************************************/
      
    }
  }
  
  /* Termination and path-readout ********************************************/
  max_path_prob = -LONG_MAX;   /* 초기화 */

  for (curt_itr = states[total_time].begin(); curt_itr != states[total_time].end(); ++curt_itr) {
    cur_path_prob = curt_itr->second.path_prob;

    if (max_path_prob < cur_path_prob) {
      max_path_prob = cur_path_prob;
      max_prev_state = curt_itr->first;
    }
  }
  
  /* 경로 저장 */
  /* state_sequence에 1부터 total_time까지 저장됨 */
  state_sequence[total_time] = max_prev_state;

  #ifdef _DEBUG_ /********************************************************/
  fprintf (stdout, "state_sequence[total_time] = %d\n", state_sequence[total_time]);
  #endif  /********************************************************/
  
  for (i = total_time - 1; i >= 1; i--) {
    state_sequence[i] = states[i+1][state_sequence[i+1]].prev_state;
  }

  #ifdef _DEBUG_    /********************************************************/
  fprintf (stdout, "품사 태그열\n");
  for (i = 1; i <= total_time; i++) {
    fprintf (stdout, "[%d]", state_sequence[i]);
  }
  fprintf (stdout, "%.2e\n", exp(max_path_prob));
  #endif /********************************************************/
}

#endif
