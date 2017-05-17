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

  /* ���� ���� Ȯ�� */
  fprintf(stderr, "\tReading phonetic probabilities.. [%s]", PHONETIC_PRB_Path);
  if (!map_scan_probability(PHONETIC_PRB_Path, phonetic_prob, "t")) {
    fprintf(stderr, "\t[ERROR] can't read the phonetic probabilities!\n");
    return 0;
  }
  fprintf(stderr, "\t[done]\n");

  /* ���� ���� ���� */
  fprintf(stderr, "\tReading phonetic information.. [%s]", PHONETIC_INFO_Path);
  if (!map_scan_probability(PHONETIC_INFO_Path, phonetic_info, "t")) {
    fprintf(stderr, "\t[ERROR] can't read the phonetic information!\n");
    return 0;
  }
  fprintf(stderr, "\t[done]\n");

  /* �̵�� ���� Ȯ�� */
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

  words.clear(); /* �ʱ�ȭ */

  strcpy(text, concatenated_str);

  pch = strtok (text, "|");

  while (pch != NULL) {
    words.push_back(pch);
    pch = strtok (NULL, "|");
  }
  return 1;
}

/******************************************************************************/
/* str : ��) �ѷ�����_|�θ��������� */
/* itr2->first : ��) I-pv|I-pv|B-ep|B-ec|I-ec|I-ec */
/* ����� syl_tag_seq�� ���� */
int get_syllable_tag_sequence(PROB_MAP &phonetic_info, char *str, SEQ_STAGS &syl_tag_seq) {

  PROB_MAP::iterator itr;
  NUM_PROB::iterator itr2;

  STAGS tags;

  STAGS tag_seq[100];

  syl_tag_seq.clear(); /* �ʱ�ȭ */

  itr = phonetic_info.find(str); /* �������� ã�´�. */
  
  if ( itr != phonetic_info.end() ) { /* ������ */
    
    int str_len;

    for (//itr2 = phonetic_info[str].begin(); 
         itr2 = itr->second.begin(); 
         itr2 != itr->second.end(); ++itr2) {

      ///**/fprintf(stderr, "%s %s\n", str, itr2->first.c_str() /* ���� �±׿� */); 

      /*  I-pv|I-pv|B-ep|B-ec|I-ec|I-ec �κ��� I-pv I-pv B-ep B-ec I-ec I-ec�� ��´�. */
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
/* ��� */
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
/* �־��� ����(syll)�� ���� �� �ִ� �±� �ĺ����� ����. */
/* ����� tag_seq�� ���� */
int get_tag_candidate(PROB_MAP &syllable_dic, const unsigned char *syll, STAGS &tag_seq) {
  
  char syll_type[MAX_WORD];

  if (syll[0] == FIL) {
    if (isalpha(syll[1])) { // ���ĺ�
      strcpy(syll_type, "alpha");
    }
    else if (isdigit(syll[1])) { // ����
      strcpy(syll_type, "digit");
    }
    else if (isascii(syll[1])) { // ��ȣ
    strcpy(syll_type, "1symb");
    }
    else { // etc
      strcpy(syll_type, "1etc");
    }
  }
  
  else {
    if (isHanja(syll[0], syll[1])) { // ����
      strcpy(syll_type, "hanja");
    }
    else if (isHangul(syll[0], syll[1])) { // �ѱ�
      strcpy(syll_type, "hangul");
    }
    else if (is2Byte(syll[0], syll[1])) { // 2byte ��ȣ
      strcpy(syll_type, "2symb");
    }
    else { // etc
      strcpy(syll_type, "2etc");
    }
  }

  PROB_MAP::iterator itr;
  NUM_PROB::iterator itr2;

  itr = syllable_dic.find(syll_type); /* �������� ã�´�. */

  if ( itr != syllable_dic.end() ) { /* ������ */

    for (itr2 = syllable_dic[syll_type].begin(); itr2 != syllable_dic[syll_type].end(); ++itr2) {
      ///**/fprintf(stderr, "%s %s %s\n", syll, syll_type, itr2->first.c_str() /* ���� �±׿� */); 
      tag_seq.push_back(itr2->first);
    }
  }

  return 1;
}

/******************************************************************************/
/* ���� ���� ���� */
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

  int num_result = 0; /* ������ ������ �� */

  int i, j, k;
  double prob_all; /* ������ ������ ���� Ȯ�� */

  /* �������� ���� ���� �İ� ���� ������ ���� Ȯ�� */
  /* ��) P(��|��) */
  double one_syll_probs[MAX_WORD] = {0.0, };  /* �ʱ� Ȯ�� log(1) */

  /* ��ü Ȯ�� */
  /* ��) P(�б���->�б���) : P(��->��)P(��->��)P(��->��) */
  double total_prob = 0.0; 
  
  SEQ_STAGS origin_syl_tag_seq; /* (�� ������ ����) ���� �±� �� */
  SEQ_STAGS syl_tag_seq; /* ���� �±� �� */
  SEQ_STAGS restored_syl_tag_seq; /* ���� �±� �� */

  int is_there_empty_syllable = 0; /* �±װ� �Ҵ���� ���� ������ �����ϴ°�? */
  vector<int> empty_syllables; /* �� ������ */

  syllable_only = 0; /* �ʱ�ȭ */
  /****************************************************************/
  /* ������ ó�� */
  /* one_syll_probs�� �� �������� �ڽ����� ������ Ȯ���� ���Ѵ�. */
  for (j = 0; j < len; j++) {
    //char *one_syllable;
    string one_syllable;
    double prob;
  
    one_syllable = sub_str[TabPos2to1(j, j+1, len)]; /* ������ �κ� ���ڿ��� */
  
    itr = phonetic_prob.find(one_syllable); /* �������� ã�´�. */

    if ( itr != phonetic_prob.end() ) { /* ������ */

      prob = phonetic_prob[one_syllable][one_syllable];

      if (prob > 0) { /* Ȯ���� �ִ� ��� */
        one_syll_probs[j] = log(prob);
      }

      /* ���� ��� */
      /* ��) "�� -> ����"�� ������ "�� -> ��"�� ���� */
      else { 
        one_syll_probs[j] = -LONG_MAX; /* Ȯ�� 0 */
      }
    } 

    total_prob += one_syll_probs[j];  /* �� ���� Ȯ�� */

    /**************************/
    /* �� ������ ���� ������ �±׸� ��� �˾Ƴ� */
    STAGS tag_seq;

    char denom[MAX_WORD];
    sprintf(denom, "%s%s%s", one_syllable.c_str(), DELIM, one_syllable.c_str());
    itr = phonetic_info.find(denom); /* �������� ã�´�. */
  
    if ( itr != phonetic_info.end() ) { /* ������ */

      for (itr2 = phonetic_info[denom].begin(); itr2 != phonetic_info[denom].end(); ++itr2) {
        ///**/fprintf(stderr, "%s %s %s\n", ej_by_2byte, denom, itr2->first.c_str() /* ���� �±׿� */); 
        tag_seq.push_back(itr2->first);
      }
    }
    else {
      tag_seq.push_back(""); /* ������ �����Ǵ� �±װ� ���� �� */
      is_there_empty_syllable = 1;
      empty_syllables.push_back(j);
    }

    origin_syl_tag_seq.push_back(tag_seq);

  } /* end of for */

  /*****************************/
  ///**/print_syl_tag_seq(origin_syl_tag_seq); /* ��� */

  /* ������ �����±׿� */
  str_syl_tag_seq.insert(make_pair(ej_by_2byte, origin_syl_tag_seq));

  /* ������ ������ Ȯ�� ���� */
  restored_ej.insert(make_pair(total_prob, ej_by_2byte));
  num_result++;
  
  /******************************************************************************/
  /* ��� �κ� ���ڿ��� ���� */
  for (i = 0; i < num; i++) {

  	itr = phonetic_prob.find(sub_str[i]); /* �κ� ���ڿ��� �������� ã�´�. */
    if ( itr == phonetic_prob.end() ) { /* ������ */
      continue; /* do nothing */
    }

    /* ���� �� �ִ� ��� ���� ��ȭ(������ ������)�� ã�´�. */
    for (itr2 = phonetic_prob[itr->first].begin(); 
         itr2 != phonetic_prob[itr->first].end(); ++itr2) {

      prob_all = total_prob; /* �ʱ�ȭ (�߿���) */

      ///**/fprintf(stdout, "[%d] %s -> %s : %lf\n", i, itr->first.c_str(), itr2->first.c_str(), log(itr2->second));

      /* ������ : ���� ���� ���� ������ */
      /* ������ */
      if (itr->first == itr2->first && strlen(sub_str[i].c_str()) == 2) {
        continue; /* do nothing */
      }

      /* �ٸ��� */
      TabPos1to2(i, len, &pos);

      /*******************************/
      /* ���� ��ü�� ���� */
      if (pos.x == 0 && pos.y == len) {
        
        /* ������ ������ Ȯ�� ���� */
        ///**/fprintf(stderr, "��ü : %s -> %s\n", itr->first.c_str(), itr2->first.c_str());
        double prob_restored_str = log(itr2->second);
        restored_ej.insert(make_pair(prob_restored_str, itr2->first.c_str()));
        num_result++;

        char denom[MAX_WORD];
        sprintf(denom, "%s%s%s", itr->first.c_str(), DELIM, itr2->first.c_str());

        /* ������ �κ� �������� ���� ��� ������ ǰ�縦 ����. */
        get_syllable_tag_sequence(phonetic_info, denom, syl_tag_seq);

        ///**/print_syl_tag_seq(syl_tag_seq); /* ��� */
        
        /* �����Ⱦ��� �����±׿� */
        str_syl_tag_seq.insert(make_pair(itr2->first, syl_tag_seq));
      }

      /*******************************/
      /* �޺κ��� ���� */
      else if (pos.y == len) {
        head.x = 0;
        head.y = pos.x;
        
        for (k = pos.x; k < len; k++) {
          prob_all -= one_syll_probs[k];
        }

        {
          int check = 0;
          /* �������� ���� �κп��� ��(�̵��) ������ ������ */
          for (int m = 0; m < (int) empty_syllables.size(); m++) {

            if (empty_syllables[m] >= head.x && empty_syllables[m] < head.y) {
              check++;
              break;
            }
          }
          if (check) continue;
        }

        sprintf(restored_str, "%s%s", sub_str[TabPos2to1(head.x, head.y, len)].c_str(), itr2->first.c_str());

        /* ������ ������ Ȯ�� ���� */
        ///**/fprintf(stderr, "�� : %s -> %s (%s)\n", itr->first.c_str(), itr2->first.c_str(), restored_str);

        restored_ej.insert(make_pair(prob_all + log(itr2->second), restored_str_ptr));
        num_result++;

        char denom[MAX_WORD];
        sprintf(denom, "%s%s%s", itr->first.c_str(), DELIM, itr2->first.c_str());

        /* ������ �κ� �������� ���� ��� ������ ǰ�縦 ����. */
        get_syllable_tag_sequence(phonetic_info, denom, syl_tag_seq);

        restored_syl_tag_seq.clear(); /* �ʱ�ȭ */

        /* �պκ� */
        for (int l = head.x; l < head.y; l++) {
          restored_syl_tag_seq.push_back(origin_syl_tag_seq[l]);
        }

        /* ������ �κ� */
        for (int n = 0; n < (int) syl_tag_seq.size(); n++) {
          restored_syl_tag_seq.push_back(syl_tag_seq[n]);
        }

        ///**/print_syl_tag_seq(restored_syl_tag_seq); /* ��� */

        /* �����Ⱦ��� �����±׿� */
        str_syl_tag_seq.insert(make_pair(restored_str_ptr, restored_syl_tag_seq));
      }

      /*******************************/
      /* �պκ��� ���� */
      else if (pos.x == 0) {
        
        tail.x = pos.y;
        tail.y = len;

        for (k = 0; k < pos.y; k++) {
          prob_all -= one_syll_probs[k];
        }

        {
          int check = 0;

          /* �������� ���� �κп��� ��(�̵��) ������ ������ */
          for (int m = 0; m < (int) empty_syllables.size(); m++) {
            if (empty_syllables[m] >= tail.x && empty_syllables[m] < tail.y) {
              check++;
              break;
            }
          }
          if (check) continue;
        }

        sprintf(restored_str, "%s%s", itr2->first.c_str(), sub_str[TabPos2to1(tail.x, tail.y, len)].c_str());

        /* ������ ������ Ȯ�� ���� */
        ///**/fprintf(stderr, "�� : %s -> %s (%s)\n", itr->first.c_str(), itr2->first.c_str(), restored_str);

        restored_ej.insert(make_pair(prob_all + log(itr2->second), restored_str_ptr));
        num_result++;

        char denom[MAX_WORD];
        sprintf(denom, "%s%s%s", itr->first.c_str(), DELIM, itr2->first.c_str());

        /* ������ �κ� �������� ���� ��� ������ ǰ�縦 ����. */
        get_syllable_tag_sequence(phonetic_info, denom, syl_tag_seq);

        restored_syl_tag_seq.clear(); /* �ʱ�ȭ */

        /* ������ �κ� */
        for (int n = 0; n < (int) syl_tag_seq.size(); n++) {
          restored_syl_tag_seq.push_back(syl_tag_seq[n]);
        }
        /* �޺κ� */
        for (int l = tail.x; l < tail.y; l++) {
          restored_syl_tag_seq.push_back(origin_syl_tag_seq[l]);
        }

        ///**/print_syl_tag_seq(restored_syl_tag_seq); /* ��� */
        
        /* �����Ⱦ��� �����±׿� */
        str_syl_tag_seq.insert(make_pair(restored_str_ptr, restored_syl_tag_seq));
      }

      /*******************************/
      /* �߰��κ��� ���� */
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

          /* �������� ���� �κп��� ��(�̵��) ������ ������ */
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

        /* ������ ������ Ȯ�� ���� */
        ///**/fprintf(stderr, "�� : %s -> %s (%s)\n", itr->first.c_str(), itr2->first.c_str(), restored_str);

        restored_ej.insert(make_pair(prob_all + log(itr2->second), restored_str_ptr));
        num_result++;

        char denom[MAX_WORD];
        sprintf(denom, "%s%s%s", itr->first.c_str(), DELIM, itr2->first.c_str());

        /* ������ �κ� �������� ���� ��� ������ ǰ�縦 ����. */
        get_syllable_tag_sequence(phonetic_info, denom, syl_tag_seq);

        restored_syl_tag_seq.clear(); /* �ʱ�ȭ */

        /* �պκ� */
        for (int l = head.x; l < head.y; l++) {
          restored_syl_tag_seq.push_back(origin_syl_tag_seq[l]);
        }

        /* ������ �κ� */
        for (int j = 0; j < (int) syl_tag_seq.size(); j++) {
          restored_syl_tag_seq.push_back(syl_tag_seq[j]);
        }

        /* �޺κ� */
        for (int n = tail.x; n < tail.y; n++) {
          restored_syl_tag_seq.push_back(origin_syl_tag_seq[n]);
        }

        ///**/print_syl_tag_seq(restored_syl_tag_seq); /* ��� */
        
        /* �����Ⱦ��� �����±׿� */
        str_syl_tag_seq.insert(make_pair(restored_str_ptr, restored_syl_tag_seq));

      } /* end of else */
    } /* end of for */
  } /* end of for */

  /* ������ �ܿ� ������ ������ ����, �±� �ĺ��� �Ҵ���� ���� �� ������ ���� �� */
  if (num_result == 1 && is_there_empty_syllable) {
    ///**/fprintf(stderr, "only one result and empty syllable\n");

    syllable_only = 1; /* ���� ���� �м��� �ϵ��� �� */

    str_syl_tag_seq.clear(); /* �ʱ�ȭ */

    /* �̵�� ������ ���� */
    for (int m = 0; m < (int) empty_syllables.size(); m++) {

      /* ������ */
      string one_syllable;
      one_syllable = sub_str[TabPos2to1(empty_syllables[m], empty_syllables[m]+1, len)];

      STAGS tag_seq;
      /* �־��� ������ ���� �� �ִ� �±� �ĺ����� ����. */
      get_tag_candidate(syllable_dic, (unsigned char *) one_syllable.c_str(), tag_seq);
      origin_syl_tag_seq[empty_syllables[m]]  = tag_seq;
    }

    /* ������ �����±׿� */
    str_syl_tag_seq.insert(make_pair(ej_by_2byte, origin_syl_tag_seq));

  } // end of if

  return num_result;
}

/******************************************************************************/
/* ���� ���� ���� */
/* ���ϰ� : ������ ������ ��, ������ ������ ������ �������� �Է��� 1�� ����, 0�� ���� */
/* �Է� : ej_by_2byte */
/* ��� : restored_ej */
int phonetic_recovery(PROB_MAP &phonetic_prob, PROB_MAP &phonetic_info, PROB_MAP &syllable_dic,
                      const char *input_ej, 
                      RESTORED_RESULT &restored_ej, RESTORED_STAGS &str_syl_tag_seq,
                      int &syllable_only) {
  int len;
  int num;
  int num_restored_ej = 0;   /* ���� ������ ������ �� */

  SUB_STRING sub_str;  // �κ� ���ڿ� ���� // vector<string>

  char ej_by_2byte[MAX_WORD]; /* 2����Ʈ(����) ������ ���� */

  /* FIL�� �߰��Ͽ� �� ������ 2byte�� �����. */
  split_by_char_array((char *)input_ej, ej_by_2byte);

  len = (int) strlen(ej_by_2byte) / 2; /* ���ڿ� ���� */
  num = TabNum(len);
 
  /* �κ� ���ڿ� ���� */
  /* ��� : sub_str */
  save_partial_str(len, ej_by_2byte, sub_str);

  /* ���� ���� ���� */
  /* ��� : restored_ej */
  num_restored_ej = 
      recover_phonetic_change(phonetic_prob, phonetic_info, syllable_dic,
                              ej_by_2byte, sub_str, num, len, 
                              restored_ej, str_syl_tag_seq, syllable_only);

  /* ������ ������ �ϳ��� ���� ��� */
  if (num_restored_ej == 0) {
    //fprintf(stderr, "no restored ej in [%s]\n", ej_by_2byte);
    
    /* �������� ����Ѵ�. */
    num_restored_ej = 1;
    string ej_ptr = ej_by_2byte;
    restored_ej.insert(make_pair((double) 0.0, ej_ptr)); /* Ȯ�� 1 */
  }

  //#define DEBUG_PHONETIC
  #ifdef DEBUG_PHONETIC /***********************************/
  fprintf(stdout, "\n");

  /* ������ ���� ��� */
  fprintf(stderr, "# of restored ej. = %d\n", num_restored_ej);

  RESTORED_RESULT::iterator it;

  for (it = restored_ej.begin(); it != restored_ej.end(); ++it) {
    fprintf(stderr, "%12.11e\t%s\n", exp(it->first), it->second.c_str());
  }
  #endif /***********************************/
  
  return num_restored_ej;
}

