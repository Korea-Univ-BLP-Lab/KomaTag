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

/* Ÿ�� ���� */
typedef struct {
  double lexical_prob; /* (���� ���¿� ����) ���¼� �м� Ȯ�� */
  double path_prob; /* (���� ���±����� ��ο� ����) Ȯ�� */
  int prev_state; /* (�ִ� Ȯ�� ��θ� ����) ���� ����(����) */

//  string head_morph;
  string head_tag;
//  string tail_morph;
//  string tail_tag;

  string first_tag; // ù��° �±�
  string last_tag;  // ������ �±�
  string second_last_tag; // ������-1 �±�

  string last_two_tag; // ������ �� �±�

} ME_BI_STRUCT_PER_STATE;

typedef map<int, ME_BI_STRUCT_PER_STATE> ME_BIGRAM_STATE_MAP; /* �±�, Ȯ��, �������� */


/*****************************************************************************/
// �Ӹ� ���¼ҿ� ���� ���¼��� ��ġ�� ã�Ƴ���.
// ����� head_pos�� tail_pos�� ����ȴ�.
int find_head_tail_morph(WORD_FREQ &full_morpheme_map, 
                         int morph_num, char morphs[][MAX_WORD], char tags[][MAX_WORD],
                         int *head_pos, int *tail_pos) {

  int i;

  WORD_FREQ::iterator it;

  // �ʱ�ȭ
  *head_pos = 0; // ù ���¼�
  *tail_pos = morph_num-1; // ������ ���¼�

  for (i = morph_num-1; i >= 0; i--) {
    it = full_morpheme_map.find(tags[i]); // ã�ƺ���
    
    if (it != full_morpheme_map.end()) { // ������
      *head_pos = i;
      break; // ���������� ã�ڸ��� ����
    }
  }

//  if (*head_pos == *tail_pos)  // �Ӹ��� ������ ��ġ�� ������ ������ �ʿ��������...
  //*tail_pos = -1; // ����.

  return 1;
}

/*****************************************************************************/
/* ���� Ȯ��, ���� Ȯ��, ����, �ܾ��� ��, �±�����, ���¿�(��� ����) */
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

  ME_BIGRAM_STATE_MAP::iterator curt_itr; /* ���� �±׿� ���� �ݺ��� */
  ME_BIGRAM_STATE_MAP::iterator prevt_itr; /* ���� �±׿� ���� �ݺ��� */

  int j;

  /* ������ ���� ��� �����ϱ� ���ؼ��� �ƴ϶�  */
  /* �Լ� ȣ��� �Ź� ���� ������ �ɸ��� �ð������� static���� ������ */
  int morph_num = 0; /* ���� ���� ���¼� �� */
  char morphs[MAX_WORD][MAX_WORD] = {0}; /* ���¼� �� */
  static char tags[MAX_WORD][MAX_WORD] = {0}; /* ǰ�� �±� �� */
  static int spacing_tags[MAX_WORD] = {0}; /* ���� �±� �� */


  /* �ʱ�ȭ *****************************************************/
  states[0][0].path_prob = 0.0; /* log (1) */
  
  //states[0][0].head_morph = BOSTAG_1;
  states[0][0].head_tag = BOSTAG_1;
  //states[0][0].tail_morph = BOSTAG_1;
  //states[0][0].tail_tag = BOSTAG_1;

  states[0][0].first_tag = BOSTAG_1;
  states[0][0].last_tag = BOSTAG_1;
  states[0][0].second_last_tag = BOSTAG_1;

  states[0][0].last_two_tag = BOSTAG_1;

  // ��� ������ ����
  for (i = 1; i <= total_time; i++) {

    for (j = 0; j < (int) morph_analyzed_result[i].size(); j++) {
      states[i][j].path_prob = -LONG_MAX;
      states[i][j].lexical_prob = log(morph_analyzed_result[i][j].first); /* ���¼Һм� Ȯ�� */
    }

    for (j = 0; j < (int) morph_analyzed_result[i].size(); j++) {

      ///**/fprintf(stderr, "i = %d, j = %d\n", i, j);
      ///**/fprintf(stderr, "%s\n", morph_analyzed_result[i][j].second.c_str());
      get_morphs_tags((char *)morph_analyzed_result[i][j].second.c_str(), &morph_num,
                      morphs, tags, spacing_tags, delimiter);

      if (morph_num < 1) { fprintf(stderr, "number of morpheme is 0\n"); exit(1); }

      int head_pos, tail_pos;

      // �Ӹ� ���¼ҿ� ���� ���¼� ã��
      find_head_tail_morph(full_morpheme_map, morph_num, morphs, tags, &head_pos, &tail_pos);

//      states[i][j].head_morph = morphs[head_pos]; /* �Ӹ� ���¼� */
      states[i][j].head_tag   = tags[head_pos];   /* �Ӹ� �±� */
//      states[i][j].tail_morph = morphs[tail_pos]; /* ���� ���¼� */
//      states[i][j].tail_tag   = tags[tail_pos];   /* ���� �±� */

      states[i][j].first_tag  = tags[0];   // ù��° �±�
      states[i][j].last_tag   = tags[morph_num-1];   // ������ �±�
      states[i][j].second_last_tag = (morph_num == 1) ? BOW_TAG_1 : tags[morph_num-2];   // ������-1 �±�
      states[i][j].last_two_tag = states[i][j].second_last_tag + states[i][j].last_tag; // ������ �� �±�

      ///**/fprintf(stderr, "tm = %s, ht = %s, tm = %s, tt = %s\n", states[i][j].head_morph.c_str(), states[i][j].head_tag.c_str(), states[i][j].tail_morph.c_str(), states[i][j].tail_tag.c_str());

    }  /* end of for */
  }

  /* Iteration Step ****************************************************/
  /* �� time(token) ���� */
  for (i = 1; i <= total_time; i++) {

    /* ���� ����(�±�)�� ���� */
    for (curt_itr = states[i].begin(); curt_itr != states[i].end(); ++curt_itr) {

      max_path_prob = -LONG_MAX;       /* log(0.0) *//* �ʱ�ȭ */

      /* ���� ����(�±�)�� ���� */ // ���.... ���� ���°� �ϳ����̶�� ������ ������ ���� �ʴ°�... (�ð��� ���� ���� �ִٴ� �ǹ�)
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

        // �Ӹ� �±�
        context.push_back("ht="+prevt_itr->second.head_tag);
        sprintf(dummy_predicate, "ht=%s", BOW_TAG_1);
        dummy_context.push_back(dummy_predicate);

        //context.push_back("tm="+prevt_itr->second.tail_morph);
        //context.push_back("tt="+prevt_itr->second.tail_tag);

        // ������ �±�
        context.push_back("lt="+prevt_itr->second.last_tag); 
        sprintf(dummy_predicate, "lt=%s", BOW_TAG_1);
        dummy_context.push_back(dummy_predicate);

        // ������-1 �±�
        context.push_back("lt-1="+prevt_itr->second.second_last_tag); 
        sprintf(dummy_predicate, "lt-1=%s", BOW_TAG_1);
        dummy_context.push_back(dummy_predicate);

        // ������ �� �±�
        context.push_back("l2t="+prevt_itr->second.last_two_tag); 
        sprintf(dummy_predicate, "l2t=%s", BOW_TAG_1);
        dummy_context.push_back(dummy_predicate);

        // �±� ���� Ȯ�� ���ϱ�

        // ���¼�/�±�
        //double logprob_head_transition = log(head_m.eval(context, curt_itr->second.head_morph+"/"+curt_itr->second.head_tag));

        // ù��° �±׷� ����
        double logprob_transition       = log(head_m.eval(context, curt_itr->second.first_tag)); 

        // ù��° �±׷� ���� (�и�)
        double logprob_dummy_transition = log(head_m.eval(dummy_context, curt_itr->second.first_tag));

        // ���� �±׷� ����
        //double logprob_tail_transition = log(tail_m.eval(context, curt_itr->second.tail_tag)); 

        ///**/fprintf(stderr, "%s <- context = %s %s %s: ", curt_itr->second.head_tag.c_str(), context[0].c_str(), context[1].c_str(), context[2].c_str());
        ///**/fprintf(stderr, "%lf\n", exp(logprob_head_transition));

        // ǰ�縸
        /*if (logprob_head_transition < LOG_ALMOST_ZERO)
          logprob_head_transition = log(head_m.eval(context, curt_itr->second.head_tag));

        if (logprob_tail_transition < LOG_ALMOST_ZERO)
          logprob_tail_transition = log(tail_m.eval(context, curt_itr->second.tail_morph+"/"+curt_itr->second.tail_tag));
*/
        
        // �ʹ� ���� Ȯ���� ���
        if (logprob_transition < LOG_ALMOST_ZERO) {
          //**/fprintf(stderr, ".");
          logprob_transition = LOG_ALMOST_ZERO;
        }

        if (logprob_dummy_transition < LOG_ALMOST_ZERO) {
          //**/fprintf(stderr, "*");
          logprob_dummy_transition = LOG_ALMOST_ZERO;
        }

        ///**/fprintf(stderr, "%lf, %lf\n", logprob_head_transition, logprob_tail_transition);
        
        /* Ȯ�� ��� */
        cur_path_prob = prevt_itr->second.path_prob /* ���� ���±����� ��ο� ���� Ȯ�� */ /*SeqPrb[i - 1][k]*/ 
          + logprob_transition // ����Ȯ�� (�Ӹ�)

#ifdef USING_DENOMINATOR
          - logprob_dummy_transition // ����Ȯ�� �и� (dummy �Ӹ�) // ���� ��
#endif
          + curt_itr->second.lexical_prob; // ���� Ȯ�� (���¼� �м� Ȯ��)

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
  #endif /********************************************************/
}

#endif
