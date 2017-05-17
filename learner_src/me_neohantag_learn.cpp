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

  //for (i = morph_num-1; i >= 0; i--) { // 뒤에서부터
  for (i = 0; i < morph_num; i++) { // 앞에서부터
    it = full_morpheme_map.find(tags[i]); // 찾아보고
    
    if (it != full_morpheme_map.end()) { // 있으면
      *head_pos = i;
      break; // 찾자마자 종료
    }
  }

//  if (*head_pos == *tail_pos)  // 머리와 꼬리의 위치가 같으면 꼬리는 필요없을지도...
  //*tail_pos = -1; // 없다.

  return 1;
}

/*****************************************************************************/
// 자질 선택 (cutoff 미만이면 출력하지 않음)
// str : 자질 이름
// target : 자질 값
// map_to_find : 자질값과 빈도가 저장된 맵
// return value : 출력하면 1, 그렇지 않으면 0
int select_feature(FILE *outfp, char *str, string target, WORD_FREQ &map_to_find, int cutoff) {
  
  WORD_FREQ::iterator it;

  it = map_to_find.find(target); // 찾아보고

  //fprintf(outfp, " >> %s", target.c_str());

  if (it != map_to_find.end()) { // 있으면
    if (it->second >= cutoff) { // cutoff 이상이면 출력
      fprintf(outfp, " %s=%s", str, target.c_str());
      return 1;
    }
  }
  
  return 0;
}

/*****************************************************************************/
// filename : 품사 부착된 말뭉치
int me_neohantag_learn(char *filename, char delimiter) {

  FILE *fp; /* 입력 화일 */

  FILE *head_fp;
  FILE *dummy_head_fp;
//  FILE *tail_fp;

  WORD_FREQ full_morpheme_map;

  /////////////////////////////////////////////////////////////////////////////
  // 실질 형태소 정의 파일 읽기
  // 화일 열기
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

  /* 화일 열기 */
  if ((fp = fopen (filename, "rt")) == NULL) {
    error("File open error : %s\n", filename);
    return 0;
  }

  // 모델 파일 열기
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
  
  int num_word; /* 문장내의 단어의 수 */
  int num_sentence = 0; /* 문장의 수 */
  int start = 2;
  int end;

  /* 변수의 값을 계속 유지하기 위해서가 아니라  */
  /* 함수 호출시 매번 변수 생성에 걸리는 시간때문에 static으로 선언함 */
  static int morph_num = 0; /* 어절 내의 형태소 수 */
  static char morphs[MAX_WORD][MAX_WORD] = {0}; /* 형태소 열 */
  static char tags[MAX_WORD][MAX_WORD] = {0}; /* 품사 태그 열 */
  static int spacing_tags[MAX_WORD] = {0}; /* 띄어쓰기 태그 열 */

  /////////////////////////////////////////////////////////////////////////////
//  string prev_head_morph; // 이전 머리 형태소
  string prev_head_tag;   // 이전 머리 태그
//  string prev_tail_morph; // 이전 꼬리 형태소
//  string prev_tail_tag;   // 이전 꼬리 태그

  string prev_first_tag;  // 이전 첫 태그
  string prev_last_tag;   // 이전 마지막 태그
  string prev_second_last_tag; // 이전 마지막-1 태그
  string prev_last_two_tag; // 이전 마지막 두 태그

  /////////////////////////////////////////////////////////////////////////////

//  string cur_head_morph; // 현재 머리 형태소
  string cur_head_tag;   // 현재 머리 태그
//  string cur_tail_morph; // 현재 꼬리 형태소
//  string cur_tail_tag;   // 현재 꼬리 태그

  string cur_first_tag;  // 현재 첫 태그
  string cur_last_tag;   // 현재 마지막 태그
  string cur_second_last_tag; // 현재 마지막-1 태그
  string cur_last_two_tag; // 현재 마지막 두 태그

  /////////////////////////////////////////////////////////////////////////////

  WORD_FREQ head_morph_map; // 머리 형태소 맵
  WORD_FREQ head_tag_map;   // 머리 태그 맵
  WORD_FREQ tail_morph_map; // 꼬리 형태소 맵
  WORD_FREQ tail_tag_map;   // 꼬리 태그 맵

  //WORD_FREQ head_tag_tail_tag_map; // 머리 태그+꼬리 태그

  WORD_FREQ last_tag_map; // 마지막 태그 맵
  WORD_FREQ second_last_tag_map; // 마지막-1 태그 맵
  WORD_FREQ last_two_tag_map; // 마지막 두 태그 맵

  /////////////////////////////////////////////////////////////////////////////
  // first pass
  fprintf(stderr, "\tfirst pass\n");

  while (1) {

    // 문장 시작에 대한 초기화 : r0
    //head_morph_map[BOSTAG_1]++;
    head_tag_map[BOSTAG_1]++;
    //tail_morph_map[BOSTAG_1]++;
    //tail_tag_map[BOSTAG_1]++;

    last_tag_map[BOSTAG_1]++;
    second_last_tag_map[BOSTAG_1]++;
    last_two_tag_map[BOSTAG_1]++;

    /* 문장을 입력 */
    num_word = get_sentence(fp, words, result);

    if (num_word <= 0) {
      report(3, "\n");
      break; // while문을 빠져나감
    }

    start = 2;
    end = num_word + 1;

    for (int i = start; i <= end; i++) {

      /* 어절 내의 형태소열과 태그열을 알아낸다. */
      get_morphs_tags(result[i], &morph_num, morphs, tags, spacing_tags, delimiter);

      // 머리 형태소와 꼬리 형태소 찾기
      int head_pos;
      int tail_pos;

      find_head_tail_morph(full_morpheme_map, morph_num, morphs, tags, &head_pos, &tail_pos);

      // 머리
      //cur_head_morph = morphs[head_pos];
      cur_head_tag = tags[head_pos];

      // 꼬리
      //cur_tail_morph = morphs[tail_pos];
      //cur_tail_tag = tags[tail_pos];

      cur_first_tag = tags[0]; // 첫번째 태그
      cur_last_tag = tags[morph_num-1]; // 마지막 태그
      cur_second_last_tag = (morph_num == 1) ? BOW_TAG_1 : tags[morph_num-2]; // 마지막-1 태그

      cur_last_two_tag = cur_second_last_tag + cur_last_tag; // 마지막 두 태그
      
      // map에 전부 저장
      //head_morph_map[cur_head_morph]++;
      head_tag_map[cur_head_tag]++;
      
      //tail_morph_map[cur_tail_morph]++;
      //tail_tag_map[cur_tail_tag]++;

      last_tag_map[cur_last_tag]++;
      second_last_tag_map[cur_second_last_tag]++;

      last_two_tag_map[cur_last_two_tag]++;

    } // end of for

  } // end of while

  rewind(fp); // 처음으로

  /////////////////////////////////////////////////////////////////////////////
  // second pass
  fprintf(stderr, "\tsecond pass\n");
  fprintf(stderr, "\tcutoff for feature selection = %d\n", FEATURE_SELECTION_CUTOFF);

  // 모든 문장에 대해
  while (1) {

    // 문장 시작에 대한 초기화 : r0
    //prev_head_morph = BOSTAG_1;
    prev_head_tag = BOSTAG_1;
    //prev_tail_morph = BOSTAG_1;
    //prev_tail_tag = BOSTAG_1;

    prev_last_tag = BOSTAG_1;
    prev_second_last_tag = BOSTAG_1;

    /* 문장을 입력 */
    num_word = get_sentence(fp, words, result);

    num_sentence++;

    if (num_word <= 0) {
      report(3, "\n");
      break; // while문을 빠져나감
    }

    // 모든 어절에 대해
    start = 2;
    end = num_word + 1;
    for (int i = start; i <= end; i++) {

      /* 어절 내의 형태소열과 태그열을 알아낸다. */
      get_morphs_tags(result[i], &morph_num, morphs, tags, spacing_tags, delimiter);

      // 머리 형태소와 꼬리 형태소 찾기
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

      cur_last_two_tag = cur_second_last_tag+cur_last_tag; // 마지막 두 태그

      WORD_FREQ::iterator it;

      ///////////////////////////////////////////////////////////////////////////////
      // 머리 모델
      // label ////////////////////
      /*it = head_morph_map.find(cur_head_morph); // 찾아보고
      if (it != head_morph_map.end()) { // 있으면
        if (it->second >= FEATURE_SELECTION_CUTOFF) { // cutoff 이상이면 출력
          fprintf(head_fp, "%s/%s", cur_head_morph, cur_head_tag); // 형태소/품사
        }
        else fprintf(head_fp, "%s", cur_head_tag); // 품사만
      }
      else fprintf(head_fp, "%s", cur_head_tag); // 품사만
      */

      fprintf(head_fp, "%s", cur_first_tag.c_str()); // 현재 어절의 첫번째 품사만
      fprintf(dummy_head_fp, "%s", cur_first_tag.c_str()); // 현재 어절의 첫번째 품사만

      // 문맥 ////////////////////

      // 머리 형태소
      //select_feature(head_fp, "hm", prev_head_morph, head_morph_map, FEATURE_SELECTION_CUTOFF); 

      // 머리 태그
      select_feature(head_fp, "ht", prev_head_tag,   head_tag_map,   FEATURE_SELECTION_CUTOFF);
      fprintf(dummy_head_fp, " ht=%s", BOW_TAG_1);

      // 꼬리 형태소
      //select_feature(head_fp, "tm", prev_tail_morph, tail_morph_map, FEATURE_SELECTION_CUTOFF); 

      // 꼬리 태그
      //select_feature(head_fp, "tt", prev_tail_tag,   tail_tag_map,   FEATURE_SELECTION_CUTOFF); 

      // 마지막 태그
      select_feature(head_fp, "lt", prev_last_tag,   last_tag_map,   FEATURE_SELECTION_CUTOFF); 
      fprintf(dummy_head_fp, " lt=%s", BOW_TAG_1);

      // 마지막-1 태그
      select_feature(head_fp, "lt-1", prev_second_last_tag,   second_last_tag_map,   FEATURE_SELECTION_CUTOFF);
      fprintf(dummy_head_fp, " lt-1=%s", BOW_TAG_1);

      // 마지막 두 태그
      select_feature(head_fp, "l2t", prev_last_two_tag,   last_two_tag_map,   FEATURE_SELECTION_CUTOFF);
      fprintf(dummy_head_fp, " l2t=%s", BOW_TAG_1);

      fprintf(head_fp, "\n"); // 개행문자
      fprintf(dummy_head_fp, "\n"); // 개행문자

      ///////////////////////////////////////////////////////////////////////////////
      // 꼬리 모델
      // label
      /*it = tail_morph_map.find(cur_tail_morph); // 찾아보고
      if (it != tail_morph_map.end()) { // 있으면
        if (it->second >= FEATURE_SELECTION_CUTOFF) { // cutoff 이상이면 출력
          fprintf(tail_fp, "%s/%s", cur_tail_morph, cur_tail_tag);
        }
        else fprintf(tail_fp, "%s", cur_tail_tag); // 품사만
      }
      else fprintf(tail_fp, "%s", cur_tail_tag); // 품사만
      */
      
      //fprintf(tail_fp, "%s", cur_tail_tag.c_str()); // 품사만

      // 문맥
      //select_feature(tail_fp, "hm", prev_head_morph, head_morph_map, FEATURE_SELECTION_CUTOFF); // 머리 형태소
      //select_feature(tail_fp, "ht", prev_head_tag,   head_tag_map,   FEATURE_SELECTION_CUTOFF); // 머리 태그
      //select_feature(tail_fp, "tm", prev_tail_morph, tail_morph_map, FEATURE_SELECTION_CUTOFF); // 꼬리 형태소
      //select_feature(tail_fp, "tt", prev_tail_tag,   tail_tag_map,   FEATURE_SELECTION_CUTOFF); // 꼬리 태그

      //fprintf(tail_fp, "\n"); // 개행문자

      ///////////////////////////////////////////////////////////////////////////////
      // 현재 어절에 대한 정보를 이전 정보로 만든다.
      // 갱신
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

  fclose(fp); /* 화일 닫기 */
  fclose(head_fp);
  fclose(dummy_head_fp);
  //fclose(tail_fp);


  /////////////////////////////////////////////////////////////////////////////
  // 최대엔트로피 확률 추정 (학습)
  fprintf(stderr, "Learning Maximum entropy models..\n");
  char command[MAX_WORD];

  // 머리 모델
  sprintf(command, "max-ent-train %s.trn %s", HEAD_MODEL, HEAD_MODEL);
  report(1, "%s\n", command);
  system(command);

  // dummy 머리 모델
  sprintf(command, "max-ent-train %s.trn %s", DUMMY_HEAD_MODEL, DUMMY_HEAD_MODEL);
  report(1, "%s\n", command);
  system(command);

  // 꼬리 모델
  //sprintf(command, "max-ent-train %s.trn %s", TAIL_MODEL, TAIL_MODEL);
  //report(1, "%s\n", command);
  //system(command);

  /////////////////////////////////////////////////////////////////////////////

  return 1;
}

