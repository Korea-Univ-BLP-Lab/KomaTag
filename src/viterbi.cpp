#include <stdio.h>
#include <math.h>               /* log () */
#include <limits.h>               /* LONG_MAX */

#include "definitions.h"
#include "probability_tool.h"
#include "hsplit.h"
#include "get_morph_tag.h"

//#define _DEBUG_

/* 타입 선언 */
typedef struct {
  double lexical_prob; /* (현재 상태에 대한) 형태소 분석 확률 */
  double path_prob; /* (현재 상태까지의 경로에 대한) 확률 */
  int prev_state; /* (최대 확률 경로를 가진) 이전 상태 */
  string first_tag;// 첫 태그
  string second_tag; // 둘째 태그
  string last_tag; // 마지막 태그
  string second_last_tag; // 마지막에서 하나 이전 태그
} BI_STRUCT_PER_STATE;

typedef map<int, BI_STRUCT_PER_STATE> BIGRAM_STATE_MAP; /* 태그, 확률, 이전상태 */

/*****************************************************************************/
/* 전이 확률, 어휘 확률, 문장, 단어의 수, 태그집합, 상태열(결과 저장) */
void bigram_viterbi_search (PROB_MAP &intra_transition_prob, PROB_MAP &inter_transition_prob, 
                            vector<ANALYZED_RESULT> &morph_analyzed_result, int total_time,
                            int *state_sequence, char delimiter) {
  int i;

#ifdef WIN32
  BIGRAM_STATE_MAP states[500];
#else
  BIGRAM_STATE_MAP states[total_time+1];
#endif

  int max_prev_state;
  double max_path_prob;
  double cur_path_prob;

  PROB_MAP::iterator itr;
  NUM_PROB::iterator itr2;

  BIGRAM_STATE_MAP::iterator curt_itr; /* 현재 태그에 대한 반복자 */
  BIGRAM_STATE_MAP::iterator prevt_itr; /* 이전 태그에 대한 반복자 */

  int j;

  /* 변수의 값을 계속 유지하기 위해서가 아니라  */
  /* 함수 호출시 매번 변수 생성에 걸리는 시간때문에 static으로 선언함 */
  int morph_num = 0; /* 어절 내의 형태소 수 */
  char morphs[MAX_WORD][MAX_WORD] = {0}; /* 형태소 열 */
  static char tags[MAX_WORD][MAX_WORD] = {0}; /* 품사 태그 열 */
  static int spacing_tags[MAX_WORD] = {0}; /* 띄어쓰기 태그 열 */


  /* 초기화 *****************************************************/
  states[0][0].path_prob = 0.0; /* log (1) */
  states[0][0].last_tag = BOSTAG_1;
  // trigram
  states[0][0].second_last_tag = BOSTAG_2;

  for (i = 1; i <= total_time; i++) { // 모든 어절에 대해

    for (j = 0; j < (int) morph_analyzed_result[i].size(); j++) { // 모든 분석 결과에 대해
      states[i][j].path_prob = -LONG_MAX;
      states[i][j].lexical_prob = log(morph_analyzed_result[i][j].first); /* 형태소분석 확률 */
      ///**/fprintf(stdout, "[%d]%lf\n", j, log(morph_analyzed_result[i][j].first));
    }

    /* 두번째부터만 의미가 있음 (중의성이 있으므로) */
      for (j = 0; j < (int) morph_analyzed_result[i].size(); j++) {

//        /**/fprintf(stderr, "i = %d, j = %d\n", i, j);
//        /**/fprintf(stderr, "%s\n", morph_analyzed_result[i][j].second.c_str());
        get_morphs_tags((char *)morph_analyzed_result[i][j].second.c_str(), &morph_num,
                        morphs, tags, spacing_tags, delimiter);

        /*for (int k = 0; k < morph_num; k++) {
          fprintf(stdout, "[%d]%s = %s\n", k, morphs[k], tags[k]);
        }*/

        if (morph_num < 1) { fprintf(stderr, "number of morpheme is 0\n"); exit(1); }

        states[i][j].first_tag = tags[0]; /* 첫번째 태그 */
        states[i][j].last_tag = tags[morph_num-1]; /* 마지막 태그 */

        if (morph_num > 1) {
          states[i][j].second_tag = tags[1]; /* 두번째 태그 */
          states[i][j].second_last_tag = tags[morph_num-2]; /* 마지막-1 태그 */
        }
        else { // 하나의 형태소를 가진 어절이면
          states[i][j].second_tag = EOW_TAG;
          states[i][j].second_last_tag = BOW_TAG_1;
        }

        ///**/fprintf(stderr, "first tag = %s, last tag = %s\n", states[i][j].first_tag.c_str(), states[i][j].last_tag.c_str());

        /* 김진동 모델은 여기서 lexical_prob을 구한다. */
        
        /* 이도길 모델은 여기서 첫번째 품사와 마지막 품사의 전이 확률(분모에 해당하는)을 구한다. */
        /* 형태소분석 확률(lexcial_prob)에서 빼자 (왜냐면, 분모니까) */

#ifdef USING_DENOMINATOR
#ifdef BIGRAM_TAGGING
        // bigram 모델 ////////////////////////////
        {
          states[i][j].lexical_prob -= log(map_get_probability2(intra_transition_prob, states[i][j].first_tag, BOW_TAG_1)); /* <- */
        }
#endif

#ifdef TRIGRAM_TAGGING
        // trigram 모델 ////////////////////////////
        {
          string denom = BOW_TAG_2;
          denom += BOW_TAG_1;
          states[i][j].lexical_prob -= log(map_get_probability2(intra_transition_prob, states[i][j].first_tag, denom)); 

          denom = BOW_TAG_1;
          denom += tags[0];
          states[i][j].lexical_prob -= log(map_get_probability2(intra_transition_prob, states[i][j].second_tag, denom)); /* <- */
        }
#endif
#endif // USING_DENOMINATOR

      }  /* end of for */
  }

  /* Iteration Step ****************************************************/
  /* 각 time(token) 마다 */
  for (i = 1; i <= total_time; i++) {

    /* 현재 상태(태그)에 대해 */
    for (curt_itr = states[i].begin(); curt_itr != states[i].end(); ++curt_itr) {

      max_path_prob = -LONG_MAX;       /* log(0.0) *//* 초기화 */
      max_prev_state = 0; // must be here...

      /* 이전 상태(태그)에 대해 */
      for (prevt_itr = states[i-1].begin(); prevt_itr != states[i-1].end(); ++prevt_itr) { 
        
        #ifdef _DEBUG_ /********************************************************/
        fprintf (stdout, "-----------------------------\n");
        fprintf (stdout, "get_probability([%d], prev_tag = %s, cur_tag = %s)\n",
          //morph_analyzed_result[i][j].second.c_str()
                  i, prevt_itr->second.last_tag.c_str(), curt_itr->second.first_tag.c_str());
        #endif  /********************************************************/
        
#ifdef TRIGRAM_TAGGING
        // trigram 모델 ////////////////////////////
        {
          double transition_prob1 = 0.0;
          double transition_prob2 = 0.0;

          string denom = prevt_itr->second.second_last_tag;
          denom += prevt_itr->second.last_tag;
          denom += "->";
   
          transition_prob1 = log ( map_get_probability2(inter_transition_prob, curt_itr->second.first_tag, denom) ); // 어절간 전이확률

          denom = prevt_itr->second.last_tag;
          denom += "->";
          denom += curt_itr->second.first_tag;

          transition_prob2 = log ( map_get_probability2(inter_transition_prob, curt_itr->second.second_tag, denom) ); // 어절간 전이확률

          /* 확률 계산 */
          cur_path_prob = prevt_itr->second.path_prob /* 이전 상태까지의 경로에 대한 확률 */ /*SeqPrb[i - 1][k]*/ 
                          + transition_prob1 + transition_prob2
                          + curt_itr->second.lexical_prob; /* 어휘 확률 */
        }
#endif

#ifdef BIGRAM_TAGGING
        // bigram 모델 ////////////////////////////
        {
          /* 확률 계산 */
          cur_path_prob = prevt_itr->second.path_prob /* 이전 상태까지의 경로에 대한 확률 */ /*SeqPrb[i - 1][k]*/ 
                          + log( map_get_probability2(inter_transition_prob, curt_itr->second.first_tag, prevt_itr->second.last_tag) ) // 어절간 전이확률
                          + curt_itr->second.lexical_prob; /* 어휘 확률 */
        }
#endif

        #ifdef _DEBUG_ /********************************************************/
        fprintf (stdout, "prev sequence + transition + lexical = %lf + %lf + %lf = %lf\n", 
          prevt_itr->second.path_prob, 
          //log ( map_get_probability2(inter_transition_prob, curt_itr->second.first_tag, prevt_itr->second.last_tag) ), 
          transition_prob1 + transition_prob2,
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
  fflush(stdout);
  #endif /********************************************************/
}

/*****************************************************************************/
/* 전이 확률, 어휘 확률, 문장, 단어의 수, 태그집합, 상태열(결과 저장) */
void bigram_viterbi_search_ej (PROB_MAP &inter_transition_prob, 
                            vector<ANALYZED_RESULT> &morph_analyzed_result, int total_time,
                            int *state_sequence, char delimiter) {
  int i;

#ifdef WIN32
  BIGRAM_STATE_MAP states[500];
#else
  BIGRAM_STATE_MAP states[total_time+1];
#endif

  int max_prev_state;
  double max_path_prob;
  double cur_path_prob;

  PROB_MAP::iterator itr;
  NUM_PROB::iterator itr2;

  BIGRAM_STATE_MAP::iterator curt_itr; /* 현재 태그에 대한 반복자 */
  BIGRAM_STATE_MAP::iterator prevt_itr; /* 이전 태그에 대한 반복자 */

  int j;

  /* 변수의 값을 계속 유지하기 위해서가 아니라  */
  /* 함수 호출시 매번 변수 생성에 걸리는 시간때문에 static으로 선언함 */
  int morph_num = 0; /* 어절 내의 형태소 수 */
  char morphs[MAX_WORD][MAX_WORD] = {0}; /* 형태소 열 */
  static char tags[MAX_WORD][MAX_WORD] = {0}; /* 품사 태그 열 */
  static int spacing_tags[MAX_WORD] = {0}; /* 띄어쓰기 태그 열 */


  /* 초기화 *****************************************************/
  states[0][0].path_prob = 0.0; /* log (1) */
  states[0][0].last_tag = BOSTAG_1;
  // trigram
  states[0][0].second_last_tag = BOSTAG_2;

  for (i = 1; i <= total_time; i++) { // 모든 어절에 대해

    for (j = 0; j < (int) morph_analyzed_result[i].size(); j++) { // 모든 분석 결과에 대해
      states[i][j].path_prob = -LONG_MAX;
      states[i][j].lexical_prob = log(morph_analyzed_result[i][j].first); /* 형태소분석 확률 */
      ///**/fprintf(stdout, "[%d]%lf\n", j, log(morph_analyzed_result[i][j].first));
    }

    /* 두번째부터만 의미가 있음 (중의성이 있으므로) */
      for (j = 0; j < (int) morph_analyzed_result[i].size(); j++) {

        get_morphs_tags((char *)morph_analyzed_result[i][j].second.c_str(), &morph_num,
                        morphs, tags, spacing_tags, delimiter);

        if (morph_num < 1) { fprintf(stderr, "number of morpheme is 0\n"); exit(1); }

        {
          string cur_tag;

#ifdef USING_ALL_TAG
          //**/fprintf(stderr, "다써\n");
          // 현재 어절의 태그 (모든 품사를 결합)
          for (int k = 0; k < morph_num; k++) {
            cur_tag += tags[k];
            cur_tag += "|";
          }
#endif

#ifdef USING_HEAD_TAIL_TAG
          //**/fprintf(stderr, "머리-꼬리\n");
          // 처음과 끝 태그의 결합
          cur_tag += tags[0];
          cur_tag += "|";
          cur_tag += tags[morph_num-1];
#endif

          ///**/fprintf(stdout, "%s\n", cur_tag.c_str());
          states[i][j].last_tag = cur_tag; // 모든 품사 또는 처음과 끝 태그

          //fprintf(stderr, "%s\n", cur_tag.c_str());
          
          // 첫 태그
          states[i][j].first_tag = tags[0];
        }


        // 전이 확률의 분모 계산 (어휘확률에 미리 계산해 둔다.)
        /* 형태소분석 확률(lexcial_prob)에서 빼자 (왜냐면, 분모니까) */

#ifdef USING_DENOMINATOR
//        fprintf(stderr, "분모유\n");
        {

          // 첫 태그로 전이
#ifdef USING_FIRST_TAG
          double logprob_transition = log(map_get_probability2(inter_transition_prob, states[i][j].first_tag, BOW_TAG_1));
#else
          // 어절 태그로 전이
          double logprob_transition = log(map_get_probability2(inter_transition_prob, states[i][j].last_tag, BOW_TAG_1));
#endif          


          ///**/fprintf(stderr, "분모 %lf\n", logprob_transition);

          states[i][j].lexical_prob -= logprob_transition;
        }
#endif

       }  /* end of for */
  }

  /* Iteration Step ****************************************************/
  /* 각 time(token) 마다 */
  for (i = 1; i <= total_time; i++) {

    /* 현재 상태(태그)에 대해 */
    for (curt_itr = states[i].begin(); curt_itr != states[i].end(); ++curt_itr) {

      max_path_prob = -LONG_MAX;       /* log(0.0) *//* 초기화 */
      max_prev_state = 0; // must be here...

      /* 이전 상태(태그)에 대해 */
      for (prevt_itr = states[i-1].begin(); prevt_itr != states[i-1].end(); ++prevt_itr) { 
        
        #ifdef _DEBUG_ /********************************************************/
        fprintf (stdout, "-----------------------------\n");
        fprintf (stdout, "get_probability([%d], prev_tag = %s, cur_tag = %s)\n",
          //morph_analyzed_result[i][j].second.c_str()
                  i, prevt_itr->second.last_tag.c_str(), curt_itr->second.first_tag.c_str());
        #endif  /********************************************************/
        
        {
          // 어절간 전이확률

#ifdef USING_FIRST_TAG
          // 이전 어절 태그 -> 현 어절 첫 태그
          double logprob_transition = log( map_get_probability2(inter_transition_prob, 
                                           curt_itr->second.first_tag, 
                                           prevt_itr->second.last_tag) ); 
#else
          // 이전 어절 태그 -> 현 어절 태그
          double logprob_transition = log( map_get_probability2(inter_transition_prob, 
                                           curt_itr->second.last_tag, 
                                           prevt_itr->second.last_tag) ); 
#endif
          
          ///**/fprintf(stderr, "전이 %lf\n\n", logprob_transition);

          /* 확률 계산 */
          cur_path_prob = prevt_itr->second.path_prob /* 이전 상태까지의 경로에 대한 확률 */ /*SeqPrb[i - 1][k]*/ 
                          + logprob_transition // 어절간 전이확률
                          + curt_itr->second.lexical_prob; /* 어휘 확률 */
        }

        #ifdef _DEBUG_ /********************************************************/
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
  fflush(stdout);
  #endif /********************************************************/
}
