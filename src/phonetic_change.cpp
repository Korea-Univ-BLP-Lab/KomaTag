#include <ctype.h>
#include <math.h>

#include "definitions.h"
#include "triangular_matrix.h"
#include "hsplit.h"
#include "phonetic_change.h"
#include "env.h"
#include "probability_tool.h"

#include <algorithm> /* count */
/******************************************************************************/
int prokoma_phonetic_open(char *PHONETIC_PRB_Path, char *PHONETIC_INFO_Path, char *SYLLABLE_DIC_Path,
                          PROB_MAP &phonetic_prob, PROB_MAP &phonetic_info, PROB_MAP &syllable_dic) {

  /* 음운 복원 확률 */
  fprintf(stderr, "\tReading phonetic probabilities.. [%s]", PHONETIC_PRB_Path);
  if (!map_scan_probability(PHONETIC_PRB_Path, phonetic_prob, "t")) {
    fprintf(stderr, "\t[ERROR] can't read the phonetic probabilities!\n");
    return 0;
  }
  fprintf(stderr, "\t[done]\n");

  /* 음운 복원 정보 */
  fprintf(stderr, "\tReading phonetic information.. [%s]", PHONETIC_INFO_Path);
  if (!map_scan_probability(PHONETIC_INFO_Path, phonetic_info, "t")) {
    fprintf(stderr, "\t[ERROR] can't read the phonetic information!\n");
    return 0;
  }
  fprintf(stderr, "\t[done]\n");

  /* 미등록 음절 확률 */
  fprintf(stderr, "\tReading syllable dictionary.. [%s]", SYLLABLE_DIC_Path);
  if (!map_scan_probability(SYLLABLE_DIC_Path, syllable_dic, "t")) {
    fprintf(stderr, "\t[ERROR] can't read the syllable dictionary!\n");
    return 0;
  }
  fprintf(stderr, "\t[done]\n");

  return 1;
}

/******************************************************************************/
int get_tag_sequence(char *concatenated_str, STAGS &words) {

  char *pch;

  char text[MAX_WORD];

  words.clear(); /* 초기화 */

  strcpy(text, concatenated_str);

  pch = strtok (text, "|");

  while (pch != NULL) {
    words.push_back(pch);
    pch = strtok (NULL, "|");
  }
  return 1;
}

/******************************************************************************/
/* str : 예) 둘렀지만_|두르었지마는 */
/* itr2->first : 예) I-pv|I-pv|B-ep|B-ec|I-ec|I-ec */
/* 결과는 syl_tag_seq에 저장 */
int get_syllable_tag_sequence(PROB_MAP &phonetic_info, char *str, SEQ_STAGS &syl_tag_seq) {

  PROB_MAP::iterator itr;
  NUM_PROB::iterator itr2;

  STAGS tags;

  STAGS tag_seq[100];

  syl_tag_seq.clear(); /* 초기화 */

  itr = phonetic_info.find(str); /* 사전에서 찾는다. */
  
  if ( itr != phonetic_info.end() ) { /* 있으면 */
    
    int str_len;

    for (//itr2 = phonetic_info[str].begin(); 
         itr2 = itr->second.begin(); 
         itr2 != itr->second.end(); ++itr2) {

      ///**/fprintf(stderr, "%s %s\n", str, itr2->first.c_str() /* 음절 태그열 */); 

      /*  I-pv|I-pv|B-ep|B-ec|I-ec|I-ec 로부터 I-pv I-pv B-ep B-ec I-ec I-ec를 얻는다. */
      get_tag_sequence((char *) itr2->first.c_str(), tags);

      str_len = (int) tags.size();

      for (int i = 0; i < (int) tags.size(); i++) {
        int num = 0;
        num = (int) count (tag_seq[i].begin(), tag_seq[i].end(), tags[i]);
        
        if (num) ; /* do nothing */
        else tag_seq[i].push_back(tags[i]);

        ///**/fprintf(stderr, "[%d] %s\n", i, tags[i].c_str());
      }
    }

    for (int j = 0; j < str_len; j++) {
      syl_tag_seq.push_back(tag_seq[j]);
    }
  }
  else return 0;

  return 1;
}

/******************************************************************************/
/* 출력 */
int print_syl_tag_seq(SEQ_STAGS &syl_tag_seq) {
  int i, j;

  for (i = 0; i < (int) syl_tag_seq.size(); i++) {
    fprintf(stderr, "[%d]", i);

    for (j = 0; j < (int) syl_tag_seq[i].size(); j++) {
      fprintf(stderr, " %s", syl_tag_seq[i][j].c_str());
    }
    fprintf(stderr, "\n");
  } 
  return 1;
}

/******************************************************************************/
/* 주어진 음절(syll)이 취할 수 있는 태그 후보들을 얻어낸다. */
/* 결과는 tag_seq에 저장 */
int get_tag_candidate(PROB_MAP &syllable_dic, const unsigned char *syll, STAGS &tag_seq) {
  
  char syll_type[MAX_WORD];

  if (syll[0] == FIL) {
    if (isalpha(syll[1])) { // 알파벳
      strcpy(syll_type, "alpha");
    }
    else if (isdigit(syll[1])) { // 숫자
      strcpy(syll_type, "digit");
    }
    else if (isascii(syll[1])) { // 기호
    strcpy(syll_type, "1symb");
    }
    else { // etc
      strcpy(syll_type, "1etc");
    }
  }
  
  else {
    if (isHanja(syll[0], syll[1])) { // 한자
      strcpy(syll_type, "hanja");
    }
    else if (isHangul(syll[0], syll[1])) { // 한글
      strcpy(syll_type, "hangul");
    }
    else if (is2Byte(syll[0], syll[1])) { // 2byte 기호
      strcpy(syll_type, "2symb");
    }
    else { // etc
      strcpy(syll_type, "2etc");
    }
  }

  PROB_MAP::iterator itr;
  NUM_PROB::iterator itr2;

  itr = syllable_dic.find(syll_type); /* 사전에서 찾는다. */

  if ( itr != syllable_dic.end() ) { /* 있으면 */

    for (itr2 = syllable_dic[syll_type].begin(); itr2 != syllable_dic[syll_type].end(); ++itr2) {
      ///**/fprintf(stderr, "%s %s %s\n", syll, syll_type, itr2->first.c_str() /* 음절 태그열 */); 
      tag_seq.push_back(itr2->first);
    }
  }

  return 1;
}

/******************************************************************************/
/* 음운 현상 복원 */
int recover_phonetic_change(PROB_MAP &phonetic_prob, PROB_MAP &phonetic_info, PROB_MAP &syllable_dic,
                            char *ej_by_2byte,
                            SUB_STRING &sub_str, int num, int len,
                            RESTORED_RESULT &restored_ej, RESTORED_STAGS &str_syl_tag_seq,
                            int &syllable_only) {

  PROB_MAP::iterator itr;
  NUM_PROB::iterator itr2;

  t_TAB pos;
  t_TAB head, tail;

  char restored_str[MAX_WORD];
  char *restored_str_ptr = restored_str;

  int num_result = 0; /* 복원된 어절의 수 */

  int i, j, k;
  double prob_all; /* 복원된 어절의 최종 확률 */

  /* 한음절의 복원 전과 후가 같은 음절에 대한 확률 */
  /* 예) P(가|가) */
  double one_syll_probs[MAX_WORD] = {0.0, };  /* 초기 확률 log(1) */

  /* 전체 확률 */
  /* 예) P(학교는->학교는) : P(학->학)P(교->교)P(는->는) */
  double total_prob = 0.0; 
  
  SEQ_STAGS origin_syl_tag_seq; /* (원 어절에 대한) 음절 태그 열 */
  SEQ_STAGS syl_tag_seq; /* 음절 태그 열 */
  SEQ_STAGS restored_syl_tag_seq; /* 음절 태그 열 */

  int is_there_empty_syllable = 0; /* 태그가 할당되지 않은 음절이 존재하는가? */
  vector<int> empty_syllables; /* 빈 음절들 */

  syllable_only = 0; /* 초기화 */
  /****************************************************************/
  /* 단음절 처리 */
  /* one_syll_probs에 각 단음절이 자신으로 유지될 확률을 구한다. */
  for (j = 0; j < len; j++) {
    //char *one_syllable;
    string one_syllable;
    double prob;
  
    one_syllable = sub_str[TabPos2to1(j, j+1, len)]; /* 단음절 부분 문자열만 */
  
    itr = phonetic_prob.find(one_syllable); /* 사전에서 찾는다. */

    if ( itr != phonetic_prob.end() ) { /* 있으면 */

      prob = phonetic_prob[one_syllable][one_syllable];

      if (prob > 0) { /* 확률이 있는 경우 */
        one_syll_probs[j] = log(prob);
      }

      /* 없는 경우 */
      /* 예) "났 -> 나았"은 있으나 "났 -> 났"은 없다 */
      else { 
        one_syll_probs[j] = -LONG_MAX; /* 확률 0 */
      }
    } 

    total_prob += one_syll_probs[j];  /* 원 어절 확률 */

    /**************************/
    /* 한 음절에 대한 가능한 태그를 모두 알아냄 */
    STAGS tag_seq;

    char denom[MAX_WORD];
    sprintf(denom, "%s%s%s", one_syllable.c_str(), DELIM, one_syllable.c_str());
    itr = phonetic_info.find(denom); /* 사전에서 찾는다. */
  
    if ( itr != phonetic_info.end() ) { /* 있으면 */

      for (itr2 = phonetic_info[denom].begin(); itr2 != phonetic_info[denom].end(); ++itr2) {
        ///**/fprintf(stderr, "%s %s %s\n", ej_by_2byte, denom, itr2->first.c_str() /* 음절 태그열 */); 
        tag_seq.push_back(itr2->first);
      }
    }
    else {
      tag_seq.push_back(""); /* 음절에 대응되는 태그가 없을 때 */
      is_there_empty_syllable = 1;
      empty_syllables.push_back(j);
    }

    origin_syl_tag_seq.push_back(tag_seq);

  } /* end of for */

  /*****************************/
  ///**/print_syl_tag_seq(origin_syl_tag_seq); /* 출력 */

  /* 원어절 음절태그열 */
  str_syl_tag_seq.insert(make_pair(ej_by_2byte, origin_syl_tag_seq));

  /* 복원된 어절과 확률 저장 */
  restored_ej.insert(make_pair(total_prob, ej_by_2byte));
  num_result++;
  
  /******************************************************************************/
  /* 모든 부분 문자열에 대해 */
  for (i = 0; i < num; i++) {

  	itr = phonetic_prob.find(sub_str[i]); /* 부분 문자열을 사전에서 찾는다. */
    if ( itr == phonetic_prob.end() ) { /* 없으면 */
      continue; /* do nothing */
    }

    /* 취할 수 있는 모든 음운 변화(어휘층 음절열)를 찾는다. */
    for (itr2 = phonetic_prob[itr->first].begin(); 
         itr2 != phonetic_prob[itr->first].end(); ++itr2) {

      prob_all = total_prob; /* 초기화 (중요함) */

      ///**/fprintf(stdout, "[%d] %s -> %s : %lf\n", i, itr->first.c_str(), itr2->first.c_str(), log(itr2->second));

      /* 같으면 : 음운 복원 대상과 같으면 */
      /* 단음절 */
      if (itr->first == itr2->first && strlen(sub_str[i].c_str()) == 2) {
        continue; /* do nothing */
      }

      /* 다르면 */
      TabPos1to2(i, len, &pos);

      /*******************************/
      /* 어절 전체를 복원 */
      if (pos.x == 0 && pos.y == len) {
        
        /* 복원된 어절과 확률 저장 */
        ///**/fprintf(stderr, "전체 : %s -> %s\n", itr->first.c_str(), itr2->first.c_str());
        double prob_restored_str = log(itr2->second);
        restored_ej.insert(make_pair(prob_restored_str, itr2->first.c_str()));
        num_result++;

        char denom[MAX_WORD];
        sprintf(denom, "%s%s%s", itr->first.c_str(), DELIM, itr2->first.c_str());

        /* 복원된 부분 음절열에 대한 모든 가능한 품사를 얻어낸다. */
        get_syllable_tag_sequence(phonetic_info, denom, syl_tag_seq);

        ///**/print_syl_tag_seq(syl_tag_seq); /* 출력 */
        
        /* 복원된어절 음절태그열 */
        str_syl_tag_seq.insert(make_pair(itr2->first, syl_tag_seq));
      }

      /*******************************/
      /* 뒷부분을 복원 */
      else if (pos.y == len) {
        head.x = 0;
        head.y = pos.x;
        
        for (k = pos.x; k < len; k++) {
          prob_all -= one_syll_probs[k];
        }

        {
          int check = 0;
          /* 복원되지 않은 부분에서 빈(미등록) 음절이 있으면 */
          for (int m = 0; m < (int) empty_syllables.size(); m++) {

            if (empty_syllables[m] >= head.x && empty_syllables[m] < head.y) {
              check++;
              break;
            }
          }
          if (check) continue;
        }

        sprintf(restored_str, "%s%s", sub_str[TabPos2to1(head.x, head.y, len)].c_str(), itr2->first.c_str());

        /* 복원된 어절과 확률 저장 */
        ///**/fprintf(stderr, "뒤 : %s -> %s (%s)\n", itr->first.c_str(), itr2->first.c_str(), restored_str);

        restored_ej.insert(make_pair(prob_all + log(itr2->second), restored_str_ptr));
        num_result++;

        char denom[MAX_WORD];
        sprintf(denom, "%s%s%s", itr->first.c_str(), DELIM, itr2->first.c_str());

        /* 복원된 부분 음절열에 대한 모든 가능한 품사를 얻어낸다. */
        get_syllable_tag_sequence(phonetic_info, denom, syl_tag_seq);

        restored_syl_tag_seq.clear(); /* 초기화 */

        /* 앞부분 */
        for (int l = head.x; l < head.y; l++) {
          restored_syl_tag_seq.push_back(origin_syl_tag_seq[l]);
        }

        /* 복원된 부분 */
        for (int n = 0; n < (int) syl_tag_seq.size(); n++) {
          restored_syl_tag_seq.push_back(syl_tag_seq[n]);
        }

        ///**/print_syl_tag_seq(restored_syl_tag_seq); /* 출력 */

        /* 복원된어절 음절태그열 */
        str_syl_tag_seq.insert(make_pair(restored_str_ptr, restored_syl_tag_seq));
      }

      /*******************************/
      /* 앞부분을 복원 */
      else if (pos.x == 0) {
        
        tail.x = pos.y;
        tail.y = len;

        for (k = 0; k < pos.y; k++) {
          prob_all -= one_syll_probs[k];
        }

        {
          int check = 0;

          /* 복원되지 않은 부분에서 빈(미등록) 음절이 있으면 */
          for (int m = 0; m < (int) empty_syllables.size(); m++) {
            if (empty_syllables[m] >= tail.x && empty_syllables[m] < tail.y) {
              check++;
              break;
            }
          }
          if (check) continue;
        }

        sprintf(restored_str, "%s%s", itr2->first.c_str(), sub_str[TabPos2to1(tail.x, tail.y, len)].c_str());

        /* 복원된 어절과 확률 저장 */
        ///**/fprintf(stderr, "앞 : %s -> %s (%s)\n", itr->first.c_str(), itr2->first.c_str(), restored_str);

        restored_ej.insert(make_pair(prob_all + log(itr2->second), restored_str_ptr));
        num_result++;

        char denom[MAX_WORD];
        sprintf(denom, "%s%s%s", itr->first.c_str(), DELIM, itr2->first.c_str());

        /* 복원된 부분 음절열에 대한 모든 가능한 품사를 얻어낸다. */
        get_syllable_tag_sequence(phonetic_info, denom, syl_tag_seq);

        restored_syl_tag_seq.clear(); /* 초기화 */

        /* 복원된 부분 */
        for (int n = 0; n < (int) syl_tag_seq.size(); n++) {
          restored_syl_tag_seq.push_back(syl_tag_seq[n]);
        }
        /* 뒷부분 */
        for (int l = tail.x; l < tail.y; l++) {
          restored_syl_tag_seq.push_back(origin_syl_tag_seq[l]);
        }

        ///**/print_syl_tag_seq(restored_syl_tag_seq); /* 출력 */
        
        /* 복원된어절 음절태그열 */
        str_syl_tag_seq.insert(make_pair(restored_str_ptr, restored_syl_tag_seq));
      }

      /*******************************/
      /* 중간부분을 복원 */
      else {
        head.x = 0;
        head.y = pos.x;
        tail.x = pos.y;
        tail.y = len;

        for (k = pos.x; k < pos.y; k++) {
          prob_all -= one_syll_probs[k];
        }

        {
          int check = 0;

          /* 복원되지 않은 부분에서 빈(미등록) 음절이 있으면 */
          for (int m = 0; m < (int) empty_syllables.size(); m++) {
            if ( (empty_syllables[m] >= head.x && empty_syllables[m] < head.y) ||
                 (empty_syllables[m] >= tail.x && empty_syllables[m] < tail.y) ) {

              check++;
              break;
            }
          }
          if (check) continue;
        }

        sprintf(restored_str, "%s%s%s", sub_str[TabPos2to1(head.x, head.y, len)].c_str(), itr2->first.c_str(), sub_str[TabPos2to1(tail.x, tail.y, len)].c_str());

        /* 복원된 어절과 확률 저장 */
        ///**/fprintf(stderr, "중 : %s -> %s (%s)\n", itr->first.c_str(), itr2->first.c_str(), restored_str);

        restored_ej.insert(make_pair(prob_all + log(itr2->second), restored_str_ptr));
        num_result++;

        char denom[MAX_WORD];
        sprintf(denom, "%s%s%s", itr->first.c_str(), DELIM, itr2->first.c_str());

        /* 복원된 부분 음절열에 대한 모든 가능한 품사를 얻어낸다. */
        get_syllable_tag_sequence(phonetic_info, denom, syl_tag_seq);

        restored_syl_tag_seq.clear(); /* 초기화 */

        /* 앞부분 */
        for (int l = head.x; l < head.y; l++) {
          restored_syl_tag_seq.push_back(origin_syl_tag_seq[l]);
        }

        /* 복원된 부분 */
        for (int j = 0; j < (int) syl_tag_seq.size(); j++) {
          restored_syl_tag_seq.push_back(syl_tag_seq[j]);
        }

        /* 뒷부분 */
        for (int n = tail.x; n < tail.y; n++) {
          restored_syl_tag_seq.push_back(origin_syl_tag_seq[n]);
        }

        ///**/print_syl_tag_seq(restored_syl_tag_seq); /* 출력 */
        
        /* 복원된어절 음절태그열 */
        str_syl_tag_seq.insert(make_pair(restored_str_ptr, restored_syl_tag_seq));

      } /* end of else */
    } /* end of for */
  } /* end of for */

  /* 원어절 외에 복원된 어절이 없고, 태그 후보가 할당되지 않은 빈 음절이 있을 때 */
  if (num_result == 1 && is_there_empty_syllable) {
    ///**/fprintf(stderr, "only one result and empty syllable\n");

    syllable_only = 1; /* 음절 단위 분석만 하도록 함 */

    str_syl_tag_seq.clear(); /* 초기화 */

    /* 미등록 음절에 대해 */
    for (int m = 0; m < (int) empty_syllables.size(); m++) {

      /* 단음절 */
      string one_syllable;
      one_syllable = sub_str[TabPos2to1(empty_syllables[m], empty_syllables[m]+1, len)];

      STAGS tag_seq;
      /* 주어진 음절이 취할 수 있는 태그 후보들을 얻어낸다. */
      get_tag_candidate(syllable_dic, (unsigned char *) one_syllable.c_str(), tag_seq);
      origin_syl_tag_seq[empty_syllables[m]]  = tag_seq;
    }

    /* 원어절 음절태그열 */
    str_syl_tag_seq.insert(make_pair(ej_by_2byte, origin_syl_tag_seq));

  } // end of if

  return num_result;
}

/******************************************************************************/
/* 음운 현상 복원 */
/* 리턴값 : 복원된 어절의 수, 복원된 어절이 없으면 원어절을 입력후 1을 리턴, 0은 오류 */
/* 입력 : ej_by_2byte */
/* 출력 : restored_ej */
int phonetic_recovery(PROB_MAP &phonetic_prob, PROB_MAP &phonetic_info, PROB_MAP &syllable_dic,
                      const char *input_ej, 
                      RESTORED_RESULT &restored_ej, RESTORED_STAGS &str_syl_tag_seq,
                      int &syllable_only) {
  int len;
  int num;
  int num_restored_ej = 0;   /* 음운 복원된 어절의 수 */

  SUB_STRING sub_str;  // 부분 문자열 저장 // vector<string>

  char ej_by_2byte[MAX_WORD]; /* 2바이트(음절) 단위의 어절 */

  /* FIL을 추가하여 각 음절을 2byte로 만든다. */
  split_by_char_array((char *)input_ej, ej_by_2byte);

  len = (int) strlen(ej_by_2byte) / 2; /* 문자열 길이 */
  num = TabNum(len);
 
  /* 부분 문자열 저장 */
  /* 출력 : sub_str */
  save_partial_str(len, ej_by_2byte, sub_str);

  /* 음운 현상 복원 */
  /* 출력 : restored_ej */
  num_restored_ej = 
      recover_phonetic_change(phonetic_prob, phonetic_info, syllable_dic,
                              ej_by_2byte, sub_str, num, len, 
                              restored_ej, str_syl_tag_seq, syllable_only);

  /* 복원된 어절이 하나도 없을 경우 */
  if (num_restored_ej == 0) {
    //fprintf(stderr, "no restored ej in [%s]\n", ej_by_2byte);
    
    /* 원어절을 사용한다. */
    num_restored_ej = 1;
    string ej_ptr = ej_by_2byte;
    restored_ej.insert(make_pair((double) 0.0, ej_ptr)); /* 확률 1 */
  }

  //#define DEBUG_PHONETIC
  #ifdef DEBUG_PHONETIC /***********************************/
  fprintf(stdout, "\n");

  /* 복원된 어절 출력 */
  fprintf(stderr, "# of restored ej. = %d\n", num_restored_ej);

  RESTORED_RESULT::iterator it;

  for (it = restored_ej.begin(); it != restored_ej.end(); ++it) {
    fprintf(stderr, "%12.11e\t%s\n", exp(it->first), it->second.c_str());
  }
  #endif /***********************************/
  
  return num_restored_ej;
}

