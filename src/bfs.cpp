#include <stdio.h>
#include <math.h>               /* log () */
#include <limits.h>               /* LONG_MAX : 2147483647 */

#include "definitions.h"
#include "hsplit.h"
#include "phonetic_change.h"
#include "FST.h"
#include "probability_tool.h"

/*****************************************************************************/
/* 현재 태그와 이전 태그와의 연결성 검사 */
/* 리턴값 : 1 = 연결 가능, 0 = 연결 불가능 */
int check_connectivity(const char *cur_tag, const char *prev_tag) {

  /* B_STYLE 일때 */
  if (cur_tag[0] == 'I') {
    if (strcmp(&cur_tag[2], &prev_tag[2]) != 0) return 0;
  }
  return 1;
}

/*****************************************************************************/
/* 현재 태그와 이전 태그와의 연결성 검사 */
/* 리턴값 : 1 = 연결 가능, 0 = 연결 불가능 */
int check_connectivity2(const char *cur_tag, const char *prev_tag) {

   // 예) 이전 태그가 SL일때(시작이거나 중간이거나 상관없이) 다음 태그가 SL의 시작이면 결합 불가

   if (strcmp(&prev_tag[2], "SL") == 0 && strcmp(cur_tag, "B-SL") == 0)
     return 0;

   if (strcmp(&prev_tag[2], "SH") == 0 && strcmp(cur_tag, "B-SH") == 0)
     return 0;

   if (strcmp(&prev_tag[2], "SN") == 0 && strcmp(cur_tag, "B-SN") == 0)
     return 0;

   if (strcmp(&prev_tag[2], "SE") == 0 && strcmp(cur_tag, "B-SE") == 0)
     return 0;

   return 1;
}

/*****************************************************************************/
/* tag_s_prob : 전이 확률 */
/* syllalbe_s_prob : 어휘 확률 */
/* syllable_dic : 음절 사전 */
/* syl_tag_seq : 가능한 음절 태그 후보 */
/* splitchar : 문장 */
/* total_time : 단어의 수 */
/* phonetic_prob : 음운복원 확률값 */
void trigram_breath_first_search(void *tag_s_fst, double *tag_s_prob,
                            void *syllable_s_fst, double *syllable_s_prob,
                            void *s_transition_fst,
                            SEQ_STAGS &syl_tag_seq,
                            char splitchar[][3], int total_time, 
                            double restored_prob, 
                            double cutoff_threshold, int beam_size,
                            RESULT_S_MAP results[]) {

  #ifdef WIN32
  vector<string> states[MAX_WORD];
  #else
  vector<string> states[total_time+3]; /* 배열 크기가 매우 중요함 */
  #endif

  int i, j, k, l; // time, 현재, 이전, 이이전

  int start_time = 2; /* 시작 위치 */
  int end_time = total_time+1; /* 끝 위치 */

  double transition_prob;
  double lexical_prob;

  /* 초기화 *****************************************************/
  /* 현재 태그, 이전태그 */
  {
    vector<string> init;
    
    states[0].push_back(BOW_TAG_2); /* 어절 시작 태그 */
    states[1].push_back(BOW_TAG_1); /* 어절 시작 태그 */

    init.push_back(BOW_TAG_2);
    init.push_back(BOW_TAG_1);
    

    //**/fprintf(stderr, "음운복원 확률 = %lf\n", restored_prob);

    /* 음운복원 확률 */ //0.0; /* log(1) */
    results[1].insert(make_pair(restored_prob, init));
  }

  /* 초기화 (각 음절이 취할 수 있는 태그를 미리 넣어둔다.) */
  {
    int i, j; // time, 가능한 품사

    ///**/fprintf(stderr, "total_time = %d, syl_tag_seq.size() = %d\n", total_time, syl_tag_seq.size());
    for (i = 0; i < (int) syl_tag_seq.size() /* total_time과 같다. */; i++) { /* 모든 음절에 대해 */
      
      ///**/fprintf(stderr, "[%d]", i);

      for (j = 0; j < (int) syl_tag_seq[i].size(); j++) { /* 현재 음절에 대한 모든 가능한 품사 */

        // 어절 첫 태그가 'I' 태그가 될 수 없음
        // 속도 향상에 도움
        if (i == 0 && syl_tag_seq[i][j][0] == 'I') { 
          ///**/fprintf(stderr, "I 찾았어 (%s)\n", syl_tag_seq[i][j].c_str());
          continue;
        }

        ///**/fprintf(stderr, " %s", syl_tag_seq[i][j].c_str());
        states[start_time+i].push_back(syl_tag_seq[i][j]/*품사*/);
      }
      ///**/fprintf(stderr, "\n");
    } 
    states[end_time+1].push_back(EOW_TAG); /* 어절 끝 태그 */
  }

  double max_prob;
  double new_prob;

  /* Iteration Step */
  /* 각 time(token) 마다 */
  /* end_time+1 -> 어절 끝 음절도 처리 */
  for (i = start_time; i <= end_time+1; i++) {
    
    max_prob = -LONG_MAX; /* 초기화 */

    /* 현재 상태(태그)에 대해 */ // j
    for (j = 0; j < (int) states[i].size(); j++) {

      /* 이전 상태(태그)에 대해 */ // k
      for (k = 0; k < (int) states[i-1].size(); k++) {

        /* 현재 태그와 이전 태그와의 연결성 검사 */
        /* for 속도 향상 */
        if (!check_connectivity(states[i][j].c_str(), states[i-1][k].c_str() )) {
          ///**/fprintf(stderr, "stop\n");
          continue;
        }

        /* 이전 품사와 현재 품사와의 연결성 검사 */
        {
          string two_tags = states[i-1][k];
          two_tags += states[i][j];
          int Index, n;
          if ((n = String2Hash (s_transition_fst, (char *)two_tags.c_str(), &Index)) == (-1)) { /* 리스트에 없으면 */
            ///**/fprintf(stderr, "제약 걸림 %s -> %s\n", states[i-1][k].c_str(), states[i][j].c_str());
            continue;
          }
        }

        // 연속해서 붙을 수 없는 태그 (현재, 이전)
        if (!check_connectivity2(states[i][j].c_str(), states[i-1][k].c_str())) {
            continue;
        }
        
        /* 이이전 상태(태그)에 대해 */ // l
        for (l = 0; l < (int) states[i-2].size(); l++) {

         ///**/fprintf(stdout, "i = %d, 현재태그 = %s, 이전 태그 = %s, 이이전 태그 = %s\n", 
           //                   i, states[i][j].c_str(), states[i-1][k].c_str(), states[i-2][l].c_str());

          if (i > 2) {
            /* 이전 태그와 이이전 태그와의 연결성 검사 */
            /* for 속도 향상 */
            if (!check_connectivity(states[i-1][k].c_str(), states[i-2][l].c_str() )) {
              ///**/fprintf(stderr, "stop\n");
              continue;
            }

            /* 이이전 품사와 이전 품사와의 연결성 검사 */
            {
              string two_tags = states[i-2][l];
              two_tags += states[i-1][k];
              int Index, n;
              if ((n = String2Hash (s_transition_fst, (char *)two_tags.c_str(), &Index)) == (-1)) { /* 리스트에 없으면 */
                ///**/fprintf(stdout, "제약 걸림 %s -> %s\n", states[i-2][l].c_str(), states[i-1][k].c_str());
                continue;
              }
              ///**/else fprintf(stdout, "제약 안걸림 %s -> %s\n", states[i-2][l].c_str(), states[i-1][k].c_str());
            }

            // 연속해서 붙을 수 없는 태그 (이전, 이이전)
            if (!check_connectivity2(states[i-1][k].c_str(), states[i-2][l].c_str())) {
              continue;
            }
          }

          /* 전이확률 */
          ///**/fprintf(stdout, "transition prob\n");
          transition_prob = log( fst_get_probability5(tag_s_fst, tag_s_prob, states[i][j], states[i-2][l], splitchar[i-1], states[i-1][k], splitchar[i]) );
          ///**/fprintf(stderr, "transition prob = %lf\n", transition_prob);

          /* 어휘 확률 */
          ///**/fprintf(stdout, "lexical prob\n");
          lexical_prob = log( fst_get_probability5(syllable_s_fst, syllable_s_prob, splitchar[i], splitchar[i-2], states[i-2][l], splitchar[i-1], states[i-1][k]) );
          ///**/fprintf(stderr, "lexical_prob = %lf\n", lexical_prob);

          /* 이전 time의 모든 결과를 뒤진다. */
          int count = 0;
          for (RESULT_S_MAP::iterator itr = results[i-1].begin(); 
               itr != results[i-1].end(); ++itr, count++) {

            if (count >= beam_size) break; // 그만
          
            /* 이전 두 태그가 같은 것을 찾으면 */
            if (itr->second[i-2] == states[i-2][l] && itr->second[i-1] == states[i-1][k]) {

              // 이전까지의 확률 + 태그(전이)확률 + 음절(어휘)확률
              new_prob = itr->first + transition_prob + lexical_prob; 

              /* cut-off가 가능한지 검사 */
              /* 최대확률값과의 차가 threshold보다 크면 저장하지 않음 */
              if (cutoff_threshold > 0 && max_prob - new_prob > cutoff_threshold) {
                ///**/fprintf(stderr, ".\n");
                continue; /* 여기서 중지 */
              }

              if (new_prob > max_prob) {
                max_prob = new_prob; /* 최대값보다 크면 최대값이 됨 */
              }

              vector<string> new_str = itr->second;
              new_str.push_back(states[i][j]); // 태그 추가

              results[i].insert(make_pair(new_prob, new_str)); // map에 삽입

              ///**/fprintf(stderr, "%s %e\n", states[i][j].c_str(), results[i][newstr]);
            } // end of if
          } /* for */
        } /* for */
      } /* for */
    } /* for */
  } /* for */

  /**/
  /* 결과 출력 */
  //for (RESULT_S_MAP::iterator itr = results[end_time].begin(); itr != results[end_time].end(); ++itr) {
    //for (int i = start_time; i < itr->first.size(); i++) {
      ///**/fprintf(stderr, "%s ", itr->first[i].c_str());
    //}
    //itr->second += restored_prob; /* 음운복원 확률 추가 */
    //**/fprintf(stderr, "\t%e\n", exp(itr->second));
  //}

}

