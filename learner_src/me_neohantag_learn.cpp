#include <stdio.h>
#include <stdlib.h>

#include "definitions.h"
#include "report.h"
#include "global_option.h"
#include "hsplit.h"
#include "tool_pos_tagged_corpus.h"
#include "get_sentence.h"

#define HEAD_MODEL "head"
#define DUMMY_HEAD_MODEL "dummy_head"
#define TAIL_MODEL "tail"

#define FEATURE_SELECTION_CUTOFF 5


#define FULLMORPHEME "fullmorpheme.def"

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

  //for (i = morph_num-1; i >= 0; i--) { // �ڿ�������
  for (i = 0; i < morph_num; i++) { // �տ�������
    it = full_morpheme_map.find(tags[i]); // ã�ƺ���
    
    if (it != full_morpheme_map.end()) { // ������
      *head_pos = i;
      break; // ã�ڸ��� ����
    }
  }

//  if (*head_pos == *tail_pos)  // �Ӹ��� ������ ��ġ�� ������ ������ �ʿ��������...
  //*tail_pos = -1; // ����.

  return 1;
}

/*****************************************************************************/
// ���� ���� (cutoff �̸��̸� ������� ����)
// str : ���� �̸�
// target : ���� ��
// map_to_find : �������� �󵵰� ����� ��
// return value : ����ϸ� 1, �׷��� ������ 0
int select_feature(FILE *outfp, char *str, string target, WORD_FREQ &map_to_find, int cutoff) {
  
  WORD_FREQ::iterator it;

  it = map_to_find.find(target); // ã�ƺ���

  //fprintf(outfp, " >> %s", target.c_str());

  if (it != map_to_find.end()) { // ������
    if (it->second >= cutoff) { // cutoff �̻��̸� ���
      fprintf(outfp, " %s=%s", str, target.c_str());
      return 1;
    }
  }
  
  return 0;
}

/*****************************************************************************/
// filename : ǰ�� ������ ����ġ
int me_neohantag_learn(char *filename, char delimiter) {

  FILE *fp; /* �Է� ȭ�� */

  FILE *head_fp;
  FILE *dummy_head_fp;
//  FILE *tail_fp;

  WORD_FREQ full_morpheme_map;

  /////////////////////////////////////////////////////////////////////////////
  // ���� ���¼� ���� ���� �б�
  // ȭ�� ����
  if ((fp = fopen (FULLMORPHEME, "rt")) == NULL) {
    error("File open error : %s\n", FULLMORPHEME);
    return 0;
  }

  char tagname[10];

  while (fscanf(fp, "%s", tagname) != EOF) {
    full_morpheme_map[tagname]++;
  }
  
  fclose(fp);

  /////////////////////////////////////////////////////////////////////////////

  /* ȭ�� ���� */
  if ((fp = fopen (filename, "rt")) == NULL) {
    error("File open error : %s\n", filename);
    return 0;
  }

  // �� ���� ����
  if ((head_fp = fopen (HEAD_MODEL".trn", "wt")) == NULL) {
    error("File open error : %s\n", HEAD_MODEL".trn");
    return 0;
  }

  if ((dummy_head_fp = fopen (DUMMY_HEAD_MODEL".trn", "wt")) == NULL) {
    error("File open error : %s\n", DUMMY_HEAD_MODEL".trn");
    return 0;
  }

  /*if ((tail_fp = fopen (TAIL_MODEL".trn", "wt")) == NULL) {
    error("File open error : %s\n", TAIL_MODEL".trn");
    return 0;
  }*/

  /////////////////////////////////////////////////////////////////////////////
  report(2, "Extracting information from %s\n", filename);

  static char words[MAX_LINE][MAX_WORD_LEN];
  static char result[MAX_LINE][MAX_RESULT_LEN];
  
  int num_word; /* ���峻�� �ܾ��� �� */
  int num_sentence = 0; /* ������ �� */
  int start = 2;
  int end;

  /* ������ ���� ��� �����ϱ� ���ؼ��� �ƴ϶�  */
  /* �Լ� ȣ��� �Ź� ���� ������ �ɸ��� �ð������� static���� ������ */
  static int morph_num = 0; /* ���� ���� ���¼� �� */
  static char morphs[MAX_WORD][MAX_WORD] = {0}; /* ���¼� �� */
  static char tags[MAX_WORD][MAX_WORD] = {0}; /* ǰ�� �±� �� */
  static int spacing_tags[MAX_WORD] = {0}; /* ���� �±� �� */

  /////////////////////////////////////////////////////////////////////////////
//  string prev_head_morph; // ���� �Ӹ� ���¼�
  string prev_head_tag;   // ���� �Ӹ� �±�
//  string prev_tail_morph; // ���� ���� ���¼�
//  string prev_tail_tag;   // ���� ���� �±�

  string prev_first_tag;  // ���� ù �±�
  string prev_last_tag;   // ���� ������ �±�
  string prev_second_last_tag; // ���� ������-1 �±�
  string prev_last_two_tag; // ���� ������ �� �±�

  /////////////////////////////////////////////////////////////////////////////

//  string cur_head_morph; // ���� �Ӹ� ���¼�
  string cur_head_tag;   // ���� �Ӹ� �±�
//  string cur_tail_morph; // ���� ���� ���¼�
//  string cur_tail_tag;   // ���� ���� �±�

  string cur_first_tag;  // ���� ù �±�
  string cur_last_tag;   // ���� ������ �±�
  string cur_second_last_tag; // ���� ������-1 �±�
  string cur_last_two_tag; // ���� ������ �� �±�

  /////////////////////////////////////////////////////////////////////////////

  WORD_FREQ head_morph_map; // �Ӹ� ���¼� ��
  WORD_FREQ head_tag_map;   // �Ӹ� �±� ��
  WORD_FREQ tail_morph_map; // ���� ���¼� ��
  WORD_FREQ tail_tag_map;   // ���� �±� ��

  //WORD_FREQ head_tag_tail_tag_map; // �Ӹ� �±�+���� �±�

  WORD_FREQ last_tag_map; // ������ �±� ��
  WORD_FREQ second_last_tag_map; // ������-1 �±� ��
  WORD_FREQ last_two_tag_map; // ������ �� �±� ��

  /////////////////////////////////////////////////////////////////////////////
  // first pass
  fprintf(stderr, "\tfirst pass\n");

  while (1) {

    // ���� ���ۿ� ���� �ʱ�ȭ : r0
    //head_morph_map[BOSTAG_1]++;
    head_tag_map[BOSTAG_1]++;
    //tail_morph_map[BOSTAG_1]++;
    //tail_tag_map[BOSTAG_1]++;

    last_tag_map[BOSTAG_1]++;
    second_last_tag_map[BOSTAG_1]++;
    last_two_tag_map[BOSTAG_1]++;

    /* ������ �Է� */
    num_word = get_sentence(fp, words, result);

    if (num_word <= 0) {
      report(3, "\n");
      break; // while���� ��������
    }

    start = 2;
    end = num_word + 1;

    for (int i = start; i <= end; i++) {

      /* ���� ���� ���¼ҿ��� �±׿��� �˾Ƴ���. */
      get_morphs_tags(result[i], &morph_num, morphs, tags, spacing_tags, delimiter);

      // �Ӹ� ���¼ҿ� ���� ���¼� ã��
      int head_pos;
      int tail_pos;

      find_head_tail_morph(full_morpheme_map, morph_num, morphs, tags, &head_pos, &tail_pos);

      // �Ӹ�
      //cur_head_morph = morphs[head_pos];
      cur_head_tag = tags[head_pos];

      // ����
      //cur_tail_morph = morphs[tail_pos];
      //cur_tail_tag = tags[tail_pos];

      cur_first_tag = tags[0]; // ù��° �±�
      cur_last_tag = tags[morph_num-1]; // ������ �±�
      cur_second_last_tag = (morph_num == 1) ? BOW_TAG_1 : tags[morph_num-2]; // ������-1 �±�

      cur_last_two_tag = cur_second_last_tag + cur_last_tag; // ������ �� �±�
      
      // map�� ���� ����
      //head_morph_map[cur_head_morph]++;
      head_tag_map[cur_head_tag]++;
      
      //tail_morph_map[cur_tail_morph]++;
      //tail_tag_map[cur_tail_tag]++;

      last_tag_map[cur_last_tag]++;
      second_last_tag_map[cur_second_last_tag]++;

      last_two_tag_map[cur_last_two_tag]++;

    } // end of for

  } // end of while

  rewind(fp); // ó������

  /////////////////////////////////////////////////////////////////////////////
  // second pass
  fprintf(stderr, "\tsecond pass\n");
  fprintf(stderr, "\tcutoff for feature selection = %d\n", FEATURE_SELECTION_CUTOFF);

  // ��� ���忡 ����
  while (1) {

    // ���� ���ۿ� ���� �ʱ�ȭ : r0
    //prev_head_morph = BOSTAG_1;
    prev_head_tag = BOSTAG_1;
    //prev_tail_morph = BOSTAG_1;
    //prev_tail_tag = BOSTAG_1;

    prev_last_tag = BOSTAG_1;
    prev_second_last_tag = BOSTAG_1;

    /* ������ �Է� */
    num_word = get_sentence(fp, words, result);

    num_sentence++;

    if (num_word <= 0) {
      report(3, "\n");
      break; // while���� ��������
    }

    // ��� ������ ����
    start = 2;
    end = num_word + 1;
    for (int i = start; i <= end; i++) {

      /* ���� ���� ���¼ҿ��� �±׿��� �˾Ƴ���. */
      get_morphs_tags(result[i], &morph_num, morphs, tags, spacing_tags, delimiter);

      // �Ӹ� ���¼ҿ� ���� ���¼� ã��
      int head_pos;
      int tail_pos;

      find_head_tail_morph(full_morpheme_map, morph_num, morphs, tags, &head_pos, &tail_pos);

      //cur_head_morph = morphs[head_pos];
      cur_head_tag = tags[head_pos];
      //cur_tail_morph = morphs[tail_pos];
      //cur_tail_tag = tags[tail_pos];

      cur_first_tag = tags[0];
      cur_last_tag = tags[morph_num-1];
      cur_second_last_tag = (morph_num == 1) ? BOW_TAG_1 : tags[morph_num-2];

      cur_last_two_tag = cur_second_last_tag+cur_last_tag; // ������ �� �±�

      WORD_FREQ::iterator it;

      ///////////////////////////////////////////////////////////////////////////////
      // �Ӹ� ��
      // label ////////////////////
      /*it = head_morph_map.find(cur_head_morph); // ã�ƺ���
      if (it != head_morph_map.end()) { // ������
        if (it->second >= FEATURE_SELECTION_CUTOFF) { // cutoff �̻��̸� ���
          fprintf(head_fp, "%s/%s", cur_head_morph, cur_head_tag); // ���¼�/ǰ��
        }
        else fprintf(head_fp, "%s", cur_head_tag); // ǰ�縸
      }
      else fprintf(head_fp, "%s", cur_head_tag); // ǰ�縸
      */

      fprintf(head_fp, "%s", cur_first_tag.c_str()); // ���� ������ ù��° ǰ�縸
      fprintf(dummy_head_fp, "%s", cur_first_tag.c_str()); // ���� ������ ù��° ǰ�縸

      // ���� ////////////////////

      // �Ӹ� ���¼�
      //select_feature(head_fp, "hm", prev_head_morph, head_morph_map, FEATURE_SELECTION_CUTOFF); 

      // �Ӹ� �±�
      select_feature(head_fp, "ht", prev_head_tag,   head_tag_map,   FEATURE_SELECTION_CUTOFF);
      fprintf(dummy_head_fp, " ht=%s", BOW_TAG_1);

      // ���� ���¼�
      //select_feature(head_fp, "tm", prev_tail_morph, tail_morph_map, FEATURE_SELECTION_CUTOFF); 

      // ���� �±�
      //select_feature(head_fp, "tt", prev_tail_tag,   tail_tag_map,   FEATURE_SELECTION_CUTOFF); 

      // ������ �±�
      select_feature(head_fp, "lt", prev_last_tag,   last_tag_map,   FEATURE_SELECTION_CUTOFF); 
      fprintf(dummy_head_fp, " lt=%s", BOW_TAG_1);

      // ������-1 �±�
      select_feature(head_fp, "lt-1", prev_second_last_tag,   second_last_tag_map,   FEATURE_SELECTION_CUTOFF);
      fprintf(dummy_head_fp, " lt-1=%s", BOW_TAG_1);

      // ������ �� �±�
      select_feature(head_fp, "l2t", prev_last_two_tag,   last_two_tag_map,   FEATURE_SELECTION_CUTOFF);
      fprintf(dummy_head_fp, " l2t=%s", BOW_TAG_1);

      fprintf(head_fp, "\n"); // ���๮��
      fprintf(dummy_head_fp, "\n"); // ���๮��

      ///////////////////////////////////////////////////////////////////////////////
      // ���� ��
      // label
      /*it = tail_morph_map.find(cur_tail_morph); // ã�ƺ���
      if (it != tail_morph_map.end()) { // ������
        if (it->second >= FEATURE_SELECTION_CUTOFF) { // cutoff �̻��̸� ���
          fprintf(tail_fp, "%s/%s", cur_tail_morph, cur_tail_tag);
        }
        else fprintf(tail_fp, "%s", cur_tail_tag); // ǰ�縸
      }
      else fprintf(tail_fp, "%s", cur_tail_tag); // ǰ�縸
      */
      
      //fprintf(tail_fp, "%s", cur_tail_tag.c_str()); // ǰ�縸

      // ����
      //select_feature(tail_fp, "hm", prev_head_morph, head_morph_map, FEATURE_SELECTION_CUTOFF); // �Ӹ� ���¼�
      //select_feature(tail_fp, "ht", prev_head_tag,   head_tag_map,   FEATURE_SELECTION_CUTOFF); // �Ӹ� �±�
      //select_feature(tail_fp, "tm", prev_tail_morph, tail_morph_map, FEATURE_SELECTION_CUTOFF); // ���� ���¼�
      //select_feature(tail_fp, "tt", prev_tail_tag,   tail_tag_map,   FEATURE_SELECTION_CUTOFF); // ���� �±�

      //fprintf(tail_fp, "\n"); // ���๮��

      ///////////////////////////////////////////////////////////////////////////////
      // ���� ������ ���� ������ ���� ������ �����.
      // ����
      //prev_head_morph = cur_head_morph;
      prev_head_tag = cur_head_tag;
      //prev_tail_morph = cur_tail_morph;
      //prev_tail_tag = cur_tail_tag;

      prev_last_tag = cur_last_tag;
      prev_second_last_tag = cur_second_last_tag;
      prev_last_two_tag = cur_last_two_tag;

    } // end of for
  } // end of while

  report(1, "Total %d of sentences are processed!\n", num_sentence);

  fclose(fp); /* ȭ�� �ݱ� */
  fclose(head_fp);
  fclose(dummy_head_fp);
  //fclose(tail_fp);


  /////////////////////////////////////////////////////////////////////////////
  // �ִ뿣Ʈ���� Ȯ�� ���� (�н�)
  fprintf(stderr, "Learning Maximum entropy models..\n");
  char command[MAX_WORD];

  // �Ӹ� ��
  sprintf(command, "max-ent-train %s.trn %s", HEAD_MODEL, HEAD_MODEL);
  report(1, "%s\n", command);
  system(command);

  // dummy �Ӹ� ��
  sprintf(command, "max-ent-train %s.trn %s", DUMMY_HEAD_MODEL, DUMMY_HEAD_MODEL);
  report(1, "%s\n", command);
  system(command);

  // ���� ��
  //sprintf(command, "max-ent-train %s.trn %s", TAIL_MODEL, TAIL_MODEL);
  //report(1, "%s\n", command);
  //system(command);

  /////////////////////////////////////////////////////////////////////////////

  return 1;
}

