#include <stdio.h>
#include <math.h>               /* log () */
#include <limits.h>               /* LONG_MAX */

#include "definitions.h"
#include "probability_tool.h"
#include "hsplit.h"
#include "get_morph_tag.h"

//#define _DEBUG_

/* Ÿ�� ���� */
typedef struct {
  double lexical_prob; /* (���� ���¿� ����) ���¼� �м� Ȯ�� */
  double path_prob; /* (���� ���±����� ��ο� ����) Ȯ�� */
  int prev_state; /* (�ִ� Ȯ�� ��θ� ����) ���� ���� */
  string first_tag;// ù �±�
  string second_tag; // ��° �±�
  string last_tag; // ������ �±�
  string second_last_tag; // ���������� �ϳ� ���� �±�
} BI_STRUCT_PER_STATE;

typedef map<int, BI_STRUCT_PER_STATE> BIGRAM_STATE_MAP; /* �±�, Ȯ��, �������� */

/*****************************************************************************/
/* ���� Ȯ��, ���� Ȯ��, ����, �ܾ��� ��, �±�����, ���¿�(��� ����) */
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

  BIGRAM_STATE_MAP::iterator curt_itr; /* ���� �±׿� ���� �ݺ��� */
  BIGRAM_STATE_MAP::iterator prevt_itr; /* ���� �±׿� ���� �ݺ��� */

  int j;

  /* ������ ���� ��� �����ϱ� ���ؼ��� �ƴ϶�  */
  /* �Լ� ȣ��� �Ź� ���� ������ �ɸ��� �ð������� static���� ������ */
  int morph_num = 0; /* ���� ���� ���¼� �� */
  char morphs[MAX_WORD][MAX_WORD] = {0}; /* ���¼� �� */
  static char tags[MAX_WORD][MAX_WORD] = {0}; /* ǰ�� �±� �� */
  static int spacing_tags[MAX_WORD] = {0}; /* ���� �±� �� */


  /* �ʱ�ȭ *****************************************************/
  states[0][0].path_prob = 0.0; /* log (1) */
  states[0][0].last_tag = BOSTAG_1;
  // trigram
  states[0][0].second_last_tag = BOSTAG_2;

  for (i = 1; i <= total_time; i++) { // ��� ������ ����

    for (j = 0; j < (int) morph_analyzed_result[i].size(); j++) { // ��� �м� ����� ����
      states[i][j].path_prob = -LONG_MAX;
      states[i][j].lexical_prob = log(morph_analyzed_result[i][j].first); /* ���¼Һм� Ȯ�� */
      ///**/fprintf(stdout, "[%d]%lf\n", j, log(morph_analyzed_result[i][j].first));
    }

    /* �ι�°���͸� �ǹ̰� ���� (���Ǽ��� �����Ƿ�) */
      for (j = 0; j < (int) morph_analyzed_result[i].size(); j++) {

//        /**/fprintf(stderr, "i = %d, j = %d\n", i, j);
//        /**/fprintf(stderr, "%s\n", morph_analyzed_result[i][j].second.c_str());
        get_morphs_tags((char *)morph_analyzed_result[i][j].second.c_str(), &morph_num,
                        morphs, tags, spacing_tags, delimiter);

        /*for (int k = 0; k < morph_num; k++) {
          fprintf(stdout, "[%d]%s = %s\n", k, morphs[k], tags[k]);
        }*/

        if (morph_num < 1) { fprintf(stderr, "number of morpheme is 0\n"); exit(1); }

        states[i][j].first_tag = tags[0]; /* ù��° �±� */
        states[i][j].last_tag = tags[morph_num-1]; /* ������ �±� */

        if (morph_num > 1) {
          states[i][j].second_tag = tags[1]; /* �ι�° �±� */
          states[i][j].second_last_tag = tags[morph_num-2]; /* ������-1 �±� */
        }
        else { // �ϳ��� ���¼Ҹ� ���� �����̸�
          states[i][j].second_tag = EOW_TAG;
          states[i][j].second_last_tag = BOW_TAG_1;
        }

        ///**/fprintf(stderr, "first tag = %s, last tag = %s\n", states[i][j].first_tag.c_str(), states[i][j].last_tag.c_str());

        /* ������ ���� ���⼭ lexical_prob�� ���Ѵ�. */
        
        /* �̵��� ���� ���⼭ ù��° ǰ��� ������ ǰ���� ���� Ȯ��(�и� �ش��ϴ�)�� ���Ѵ�. */
        /* ���¼Һм� Ȯ��(lexcial_prob)���� ���� (�ֳĸ�, �и�ϱ�) */

#ifdef USING_DENOMINATOR
#ifdef BIGRAM_TAGGING
        // bigram �� ////////////////////////////
        {
          states[i][j].lexical_prob -= log(map_get_probability2(intra_transition_prob, states[i][j].first_tag, BOW_TAG_1)); /* <- */
        }
#endif

#ifdef TRIGRAM_TAGGING
        // trigram �� ////////////////////////////
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
  /* �� time(token) ���� */
  for (i = 1; i <= total_time; i++) {

    /* ���� ����(�±�)�� ���� */
    for (curt_itr = states[i].begin(); curt_itr != states[i].end(); ++curt_itr) {

      max_path_prob = -LONG_MAX;       /* log(0.0) *//* �ʱ�ȭ */
      max_prev_state = 0; // must be here...

      /* ���� ����(�±�)�� ���� */
      for (prevt_itr = states[i-1].begin(); prevt_itr != states[i-1].end(); ++prevt_itr) { 
        
        #ifdef _DEBUG_ /********************************************************/
        fprintf (stdout, "-----------------------------\n");
        fprintf (stdout, "get_probability([%d], prev_tag = %s, cur_tag = %s)\n",
          //morph_analyzed_result[i][j].second.c_str()
                  i, prevt_itr->second.last_tag.c_str(), curt_itr->second.first_tag.c_str());
        #endif  /********************************************************/
        
#ifdef TRIGRAM_TAGGING
        // trigram �� ////////////////////////////
        {
          double transition_prob1 = 0.0;
          double transition_prob2 = 0.0;

          string denom = prevt_itr->second.second_last_tag;
          denom += prevt_itr->second.last_tag;
          denom += "->";
   
          transition_prob1 = log ( map_get_probability2(inter_transition_prob, curt_itr->second.first_tag, denom) ); // ������ ����Ȯ��

          denom = prevt_itr->second.last_tag;
          denom += "->";
          denom += curt_itr->second.first_tag;

          transition_prob2 = log ( map_get_probability2(inter_transition_prob, curt_itr->second.second_tag, denom) ); // ������ ����Ȯ��

          /* Ȯ�� ��� */
          cur_path_prob = prevt_itr->second.path_prob /* ���� ���±����� ��ο� ���� Ȯ�� */ /*SeqPrb[i - 1][k]*/ 
                          + transition_prob1 + transition_prob2
                          + curt_itr->second.lexical_prob; /* ���� Ȯ�� */
        }
#endif

#ifdef BIGRAM_TAGGING
        // bigram �� ////////////////////////////
        {
          /* Ȯ�� ��� */
          cur_path_prob = prevt_itr->second.path_prob /* ���� ���±����� ��ο� ���� Ȯ�� */ /*SeqPrb[i - 1][k]*/ 
                          + log( map_get_probability2(inter_transition_prob, curt_itr->second.first_tag, prevt_itr->second.last_tag) ) // ������ ����Ȯ��
                          + curt_itr->second.lexical_prob; /* ���� Ȯ�� */
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

      curt_itr->second.path_prob = max_path_prob; /* arg ; �ִ� Ȯ�� */
      curt_itr->second.prev_state = max_prev_state; /* argmax ; ���� ���� */

      #ifdef _DEBUG_ /********************************************************/
      fprintf (stdout, "max prob = %lf, max prev state = %d\n",
               curt_itr->second.path_prob, curt_itr->second.prev_state);
      #endif /****************************************************************/
      
    }
  }
  
  /* Termination and path-readout ********************************************/
  max_path_prob = -LONG_MAX;   /* �ʱ�ȭ */

  for (curt_itr = states[total_time].begin(); curt_itr != states[total_time].end(); ++curt_itr) {
    cur_path_prob = curt_itr->second.path_prob;

    if (max_path_prob < cur_path_prob) {
      max_path_prob = cur_path_prob;
      max_prev_state = curt_itr->first;
    }
  }
  
  /* ��� ���� */
  /* state_sequence�� 1���� total_time���� ����� */
  state_sequence[total_time] = max_prev_state;

  #ifdef _DEBUG_ /********************************************************/
  fprintf (stdout, "state_sequence[total_time] = %d\n", state_sequence[total_time]);
  #endif  /********************************************************/
  
  for (i = total_time - 1; i >= 1; i--) {
    state_sequence[i] = states[i+1][state_sequence[i+1]].prev_state;
  }

  #ifdef _DEBUG_    /********************************************************/
  fprintf (stdout, "ǰ�� �±׿�\n");
  for (i = 1; i <= total_time; i++) {
    fprintf (stdout, "[%d]", state_sequence[i]);
  }
  fprintf (stdout, "%.2e\n", exp(max_path_prob));
  fflush(stdout);
  #endif /********************************************************/
}

/*****************************************************************************/
/* ���� Ȯ��, ���� Ȯ��, ����, �ܾ��� ��, �±�����, ���¿�(��� ����) */
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

  BIGRAM_STATE_MAP::iterator curt_itr; /* ���� �±׿� ���� �ݺ��� */
  BIGRAM_STATE_MAP::iterator prevt_itr; /* ���� �±׿� ���� �ݺ��� */

  int j;

  /* ������ ���� ��� �����ϱ� ���ؼ��� �ƴ϶�  */
  /* �Լ� ȣ��� �Ź� ���� ������ �ɸ��� �ð������� static���� ������ */
  int morph_num = 0; /* ���� ���� ���¼� �� */
  char morphs[MAX_WORD][MAX_WORD] = {0}; /* ���¼� �� */
  static char tags[MAX_WORD][MAX_WORD] = {0}; /* ǰ�� �±� �� */
  static int spacing_tags[MAX_WORD] = {0}; /* ���� �±� �� */


  /* �ʱ�ȭ *****************************************************/
  states[0][0].path_prob = 0.0; /* log (1) */
  states[0][0].last_tag = BOSTAG_1;
  // trigram
  states[0][0].second_last_tag = BOSTAG_2;

  for (i = 1; i <= total_time; i++) { // ��� ������ ����

    for (j = 0; j < (int) morph_analyzed_result[i].size(); j++) { // ��� �м� ����� ����
      states[i][j].path_prob = -LONG_MAX;
      states[i][j].lexical_prob = log(morph_analyzed_result[i][j].first); /* ���¼Һм� Ȯ�� */
      ///**/fprintf(stdout, "[%d]%lf\n", j, log(morph_analyzed_result[i][j].first));
    }

    /* �ι�°���͸� �ǹ̰� ���� (���Ǽ��� �����Ƿ�) */
      for (j = 0; j < (int) morph_analyzed_result[i].size(); j++) {

        get_morphs_tags((char *)morph_analyzed_result[i][j].second.c_str(), &morph_num,
                        morphs, tags, spacing_tags, delimiter);

        if (morph_num < 1) { fprintf(stderr, "number of morpheme is 0\n"); exit(1); }

        {
          string cur_tag;

#ifdef USING_ALL_TAG
          //**/fprintf(stderr, "�ٽ�\n");
          // ���� ������ �±� (��� ǰ�縦 ����)
          for (int k = 0; k < morph_num; k++) {
            cur_tag += tags[k];
            cur_tag += "|";
          }
#endif

#ifdef USING_HEAD_TAIL_TAG
          //**/fprintf(stderr, "�Ӹ�-����\n");
          // ó���� �� �±��� ����
          cur_tag += tags[0];
          cur_tag += "|";
          cur_tag += tags[morph_num-1];
#endif

          ///**/fprintf(stdout, "%s\n", cur_tag.c_str());
          states[i][j].last_tag = cur_tag; // ��� ǰ�� �Ǵ� ó���� �� �±�

          //fprintf(stderr, "%s\n", cur_tag.c_str());
          
          // ù �±�
          states[i][j].first_tag = tags[0];
        }


        // ���� Ȯ���� �и� ��� (����Ȯ���� �̸� ����� �д�.)
        /* ���¼Һм� Ȯ��(lexcial_prob)���� ���� (�ֳĸ�, �и�ϱ�) */

#ifdef USING_DENOMINATOR
//        fprintf(stderr, "�и���\n");
        {

          // ù �±׷� ����
#ifdef USING_FIRST_TAG
          double logprob_transition = log(map_get_probability2(inter_transition_prob, states[i][j].first_tag, BOW_TAG_1));
#else
          // ���� �±׷� ����
          double logprob_transition = log(map_get_probability2(inter_transition_prob, states[i][j].last_tag, BOW_TAG_1));
#endif          


          ///**/fprintf(stderr, "�и� %lf\n", logprob_transition);

          states[i][j].lexical_prob -= logprob_transition;
        }
#endif

       }  /* end of for */
  }

  /* Iteration Step ****************************************************/
  /* �� time(token) ���� */
  for (i = 1; i <= total_time; i++) {

    /* ���� ����(�±�)�� ���� */
    for (curt_itr = states[i].begin(); curt_itr != states[i].end(); ++curt_itr) {

      max_path_prob = -LONG_MAX;       /* log(0.0) *//* �ʱ�ȭ */
      max_prev_state = 0; // must be here...

      /* ���� ����(�±�)�� ���� */
      for (prevt_itr = states[i-1].begin(); prevt_itr != states[i-1].end(); ++prevt_itr) { 
        
        #ifdef _DEBUG_ /********************************************************/
        fprintf (stdout, "-----------------------------\n");
        fprintf (stdout, "get_probability([%d], prev_tag = %s, cur_tag = %s)\n",
          //morph_analyzed_result[i][j].second.c_str()
                  i, prevt_itr->second.last_tag.c_str(), curt_itr->second.first_tag.c_str());
        #endif  /********************************************************/
        
        {
          // ������ ����Ȯ��

#ifdef USING_FIRST_TAG
          // ���� ���� �±� -> �� ���� ù �±�
          double logprob_transition = log( map_get_probability2(inter_transition_prob, 
                                           curt_itr->second.first_tag, 
                                           prevt_itr->second.last_tag) ); 
#else
          // ���� ���� �±� -> �� ���� �±�
          double logprob_transition = log( map_get_probability2(inter_transition_prob, 
                                           curt_itr->second.last_tag, 
                                           prevt_itr->second.last_tag) ); 
#endif
          
          ///**/fprintf(stderr, "���� %lf\n\n", logprob_transition);

          /* Ȯ�� ��� */
          cur_path_prob = prevt_itr->second.path_prob /* ���� ���±����� ��ο� ���� Ȯ�� */ /*SeqPrb[i - 1][k]*/ 
                          + logprob_transition // ������ ����Ȯ��
                          + curt_itr->second.lexical_prob; /* ���� Ȯ�� */
        }

        #ifdef _DEBUG_ /********************************************************/
        #endif  /********************************************************/
        
        if (max_path_prob < cur_path_prob) {
          max_path_prob = cur_path_prob;
          max_prev_state = prevt_itr->first;
        }
      }

      curt_itr->second.path_prob = max_path_prob; /* arg ; �ִ� Ȯ�� */
      curt_itr->second.prev_state = max_prev_state; /* argmax ; ���� ���� */

      #ifdef _DEBUG_ /********************************************************/
      fprintf (stdout, "max prob = %lf, max prev state = %d\n",
               curt_itr->second.path_prob, curt_itr->second.prev_state);
      #endif /****************************************************************/
      
    }
  }
  
  /* Termination and path-readout ********************************************/
  max_path_prob = -LONG_MAX;   /* �ʱ�ȭ */

  for (curt_itr = states[total_time].begin(); curt_itr != states[total_time].end(); ++curt_itr) {
    cur_path_prob = curt_itr->second.path_prob;

    if (max_path_prob < cur_path_prob) {
      max_path_prob = cur_path_prob;
      max_prev_state = curt_itr->first;
    }
  }
  
  /* ��� ���� */
  /* state_sequence�� 1���� total_time���� ����� */
  state_sequence[total_time] = max_prev_state;

  #ifdef _DEBUG_ /********************************************************/
  fprintf (stdout, "state_sequence[total_time] = %d\n", state_sequence[total_time]);
  #endif  /********************************************************/
  
  for (i = total_time - 1; i >= 1; i--) {
    state_sequence[i] = states[i+1][state_sequence[i+1]].prev_state;
  }

  #ifdef _DEBUG_    /********************************************************/
  fprintf (stdout, "ǰ�� �±׿�\n");
  for (i = 1; i <= total_time; i++) {
    fprintf (stdout, "[%d]", state_sequence[i]);
  }
  fprintf (stdout, "%.2e\n", exp(max_path_prob));
  fflush(stdout);
  #endif /********************************************************/
}
