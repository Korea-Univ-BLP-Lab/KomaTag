#include <stdio.h>
#include <math.h>               /* log () */
#include <limits.h>               /* LONG_MAX : 2147483647 */

#include "definitions.h"
#include "hsplit.h"
#include "phonetic_change.h"
#include "FST.h"
#include "probability_tool.h"

/*****************************************************************************/
/* ���� �±׿� ���� �±׿��� ���Ἲ �˻� */
/* ���ϰ� : 1 = ���� ����, 0 = ���� �Ұ��� */
int check_connectivity(const char *cur_tag, const char *prev_tag) {

  /* B_STYLE �϶� */
  if (cur_tag[0] == 'I') {
    if (strcmp(&cur_tag[2], &prev_tag[2]) != 0) return 0;
  }
  return 1;
}

/*****************************************************************************/
/* ���� �±׿� ���� �±׿��� ���Ἲ �˻� */
/* ���ϰ� : 1 = ���� ����, 0 = ���� �Ұ��� */
int check_connectivity2(const char *cur_tag, const char *prev_tag) {

   // ��) ���� �±װ� SL�϶�(�����̰ų� �߰��̰ų� �������) ���� �±װ� SL�� �����̸� ���� �Ұ�

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
/* tag_s_prob : ���� Ȯ�� */
/* syllalbe_s_prob : ���� Ȯ�� */
/* syllable_dic : ���� ���� */
/* syl_tag_seq : ������ ���� �±� �ĺ� */
/* splitchar : ���� */
/* total_time : �ܾ��� �� */
/* phonetic_prob : ����� Ȯ���� */
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
  vector<string> states[total_time+3]; /* �迭 ũ�Ⱑ �ſ� �߿��� */
  #endif

  int i, j, k, l; // time, ����, ����, ������

  int start_time = 2; /* ���� ��ġ */
  int end_time = total_time+1; /* �� ��ġ */

  double transition_prob;
  double lexical_prob;

  /* �ʱ�ȭ *****************************************************/
  /* ���� �±�, �����±� */
  {
    vector<string> init;
    
    states[0].push_back(BOW_TAG_2); /* ���� ���� �±� */
    states[1].push_back(BOW_TAG_1); /* ���� ���� �±� */

    init.push_back(BOW_TAG_2);
    init.push_back(BOW_TAG_1);
    

    //**/fprintf(stderr, "����� Ȯ�� = %lf\n", restored_prob);

    /* ����� Ȯ�� */ //0.0; /* log(1) */
    results[1].insert(make_pair(restored_prob, init));
  }

  /* �ʱ�ȭ (�� ������ ���� �� �ִ� �±׸� �̸� �־�д�.) */
  {
    int i, j; // time, ������ ǰ��

    ///**/fprintf(stderr, "total_time = %d, syl_tag_seq.size() = %d\n", total_time, syl_tag_seq.size());
    for (i = 0; i < (int) syl_tag_seq.size() /* total_time�� ����. */; i++) { /* ��� ������ ���� */
      
      ///**/fprintf(stderr, "[%d]", i);

      for (j = 0; j < (int) syl_tag_seq[i].size(); j++) { /* ���� ������ ���� ��� ������ ǰ�� */

        // ���� ù �±װ� 'I' �±װ� �� �� ����
        // �ӵ� ��� ����
        if (i == 0 && syl_tag_seq[i][j][0] == 'I') { 
          ///**/fprintf(stderr, "I ã�Ҿ� (%s)\n", syl_tag_seq[i][j].c_str());
          continue;
        }

        ///**/fprintf(stderr, " %s", syl_tag_seq[i][j].c_str());
        states[start_time+i].push_back(syl_tag_seq[i][j]/*ǰ��*/);
      }
      ///**/fprintf(stderr, "\n");
    } 
    states[end_time+1].push_back(EOW_TAG); /* ���� �� �±� */
  }

  double max_prob;
  double new_prob;

  /* Iteration Step */
  /* �� time(token) ���� */
  /* end_time+1 -> ���� �� ������ ó�� */
  for (i = start_time; i <= end_time+1; i++) {
    
    max_prob = -LONG_MAX; /* �ʱ�ȭ */

    /* ���� ����(�±�)�� ���� */ // j
    for (j = 0; j < (int) states[i].size(); j++) {

      /* ���� ����(�±�)�� ���� */ // k
      for (k = 0; k < (int) states[i-1].size(); k++) {

        /* ���� �±׿� ���� �±׿��� ���Ἲ �˻� */
        /* for �ӵ� ��� */
        if (!check_connectivity(states[i][j].c_str(), states[i-1][k].c_str() )) {
          ///**/fprintf(stderr, "stop\n");
          continue;
        }

        /* ���� ǰ��� ���� ǰ����� ���Ἲ �˻� */
        {
          string two_tags = states[i-1][k];
          two_tags += states[i][j];
          int Index, n;
          if ((n = String2Hash (s_transition_fst, (char *)two_tags.c_str(), &Index)) == (-1)) { /* ����Ʈ�� ������ */
            ///**/fprintf(stderr, "���� �ɸ� %s -> %s\n", states[i-1][k].c_str(), states[i][j].c_str());
            continue;
          }
        }

        // �����ؼ� ���� �� ���� �±� (����, ����)
        if (!check_connectivity2(states[i][j].c_str(), states[i-1][k].c_str())) {
            continue;
        }
        
        /* ������ ����(�±�)�� ���� */ // l
        for (l = 0; l < (int) states[i-2].size(); l++) {

         ///**/fprintf(stdout, "i = %d, �����±� = %s, ���� �±� = %s, ������ �±� = %s\n", 
           //                   i, states[i][j].c_str(), states[i-1][k].c_str(), states[i-2][l].c_str());

          if (i > 2) {
            /* ���� �±׿� ������ �±׿��� ���Ἲ �˻� */
            /* for �ӵ� ��� */
            if (!check_connectivity(states[i-1][k].c_str(), states[i-2][l].c_str() )) {
              ///**/fprintf(stderr, "stop\n");
              continue;
            }

            /* ������ ǰ��� ���� ǰ����� ���Ἲ �˻� */
            {
              string two_tags = states[i-2][l];
              two_tags += states[i-1][k];
              int Index, n;
              if ((n = String2Hash (s_transition_fst, (char *)two_tags.c_str(), &Index)) == (-1)) { /* ����Ʈ�� ������ */
                ///**/fprintf(stdout, "���� �ɸ� %s -> %s\n", states[i-2][l].c_str(), states[i-1][k].c_str());
                continue;
              }
              ///**/else fprintf(stdout, "���� �Ȱɸ� %s -> %s\n", states[i-2][l].c_str(), states[i-1][k].c_str());
            }

            // �����ؼ� ���� �� ���� �±� (����, ������)
            if (!check_connectivity2(states[i-1][k].c_str(), states[i-2][l].c_str())) {
              continue;
            }
          }

          /* ����Ȯ�� */
          ///**/fprintf(stdout, "transition prob\n");
          transition_prob = log( fst_get_probability5(tag_s_fst, tag_s_prob, states[i][j], states[i-2][l], splitchar[i-1], states[i-1][k], splitchar[i]) );
          ///**/fprintf(stderr, "transition prob = %lf\n", transition_prob);

          /* ���� Ȯ�� */
          ///**/fprintf(stdout, "lexical prob\n");
          lexical_prob = log( fst_get_probability5(syllable_s_fst, syllable_s_prob, splitchar[i], splitchar[i-2], states[i-2][l], splitchar[i-1], states[i-1][k]) );
          ///**/fprintf(stderr, "lexical_prob = %lf\n", lexical_prob);

          /* ���� time�� ��� ����� ������. */
          int count = 0;
          for (RESULT_S_MAP::iterator itr = results[i-1].begin(); 
               itr != results[i-1].end(); ++itr, count++) {

            if (count >= beam_size) break; // �׸�
          
            /* ���� �� �±װ� ���� ���� ã���� */
            if (itr->second[i-2] == states[i-2][l] && itr->second[i-1] == states[i-1][k]) {

              // ���������� Ȯ�� + �±�(����)Ȯ�� + ����(����)Ȯ��
              new_prob = itr->first + transition_prob + lexical_prob; 

              /* cut-off�� �������� �˻� */
              /* �ִ�Ȯ�������� ���� threshold���� ũ�� �������� ���� */
              if (cutoff_threshold > 0 && max_prob - new_prob > cutoff_threshold) {
                ///**/fprintf(stderr, ".\n");
                continue; /* ���⼭ ���� */
              }

              if (new_prob > max_prob) {
                max_prob = new_prob; /* �ִ밪���� ũ�� �ִ밪�� �� */
              }

              vector<string> new_str = itr->second;
              new_str.push_back(states[i][j]); // �±� �߰�

              results[i].insert(make_pair(new_prob, new_str)); // map�� ����

              ///**/fprintf(stderr, "%s %e\n", states[i][j].c_str(), results[i][newstr]);
            } // end of if
          } /* for */
        } /* for */
      } /* for */
    } /* for */
  } /* for */

  /**/
  /* ��� ��� */
  //for (RESULT_S_MAP::iterator itr = results[end_time].begin(); itr != results[end_time].end(); ++itr) {
    //for (int i = start_time; i < itr->first.size(); i++) {
      ///**/fprintf(stderr, "%s ", itr->first[i].c_str());
    //}
    //itr->second += restored_prob; /* ����� Ȯ�� �߰� */
    //**/fprintf(stderr, "\t%e\n", exp(itr->second));
  //}

}

