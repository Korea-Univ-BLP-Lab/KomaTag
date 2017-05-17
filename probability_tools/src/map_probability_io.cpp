#include <stdio.h>
#include "probability_tool.h"

/******************************************************************************/
/* 확률을 출력 (text 모드) */
// mode : "t" = text file, "b" = binary file
int map_print_probability(char *filename, PROB_MAP &probs, char *mode) {
  
  FILE *fp;

  // text 모드이면
  if (strcmp(mode, "t") == 0) {

    if ((fp = fopen(filename, "wt")) == NULL) {
      fprintf(stderr, "Error: cannot open file [%s]\n", filename);
      return 0;
    }
  
    for (PROB_MAP::iterator denom = probs.begin(); denom != probs.end(); ++denom)  {

      for (NUM_PROB::iterator num = probs[denom->first].begin(); 
           num != probs[denom->first].end(); ++num) {

        fprintf(fp, "%s\t%s\t%12.11e\n", denom->first.c_str(), /* 분모 */
                num->first.c_str(), /* 분자-분모 */
                probs[denom->first][num->first]); /* 확률 */
      }
    }
  } // end of if
  
  // binary 모드이면
  else {

    double prob;
    int length;

    if ((fp = fopen(filename, "wb")) == NULL) {
      fprintf(stderr, "Error: cannot open file [%s]\n", filename);
      return 0;
    }

    for (PROB_MAP::iterator denom = probs.begin(); denom != probs.end(); ++denom)  {

      for (NUM_PROB::iterator num = probs[denom->first].begin(); 
        num != probs[denom->first].end(); ++num) {

        length = (int) strlen(denom->first.c_str()) + 1; 

        fwrite(denom->first.c_str(), length, 1, fp); /* 문자열 (분모) */

        length = (int) strlen(num->first.c_str()) + 1; 

        fwrite(num->first.c_str(), length, 1, fp); /* 문자열 (분자 - 분모) */

        prob = probs[denom->first][num->first]; 
        fwrite(&prob, sizeof(double), 1, fp); /* 확률 */
      }
    }

  } // end of else

  fclose(fp);
  return 1;
}

/******************************************************************************/
/* 확률을 입력 (text 모드) */
/* 리턴값 : 실패 = 0, 확률의 수 */
// mode : "t" = text file, "b" = binary file
int map_scan_probability(char *filename, PROB_MAP &probs, char *mode) {

  FILE *fp;

  int count = 0;
  int line_num = 0;

  // text 모드이면
  if (strcmp(mode, "t") == 0) {
  
    char denom[1000];
    char num[1000];
    double prob;

    int num_item;

    if ((fp = fopen(filename, "rt")) == NULL) {
      fprintf(stderr, "Error: cannot open file [%s]\n", filename);
      return 0;
    }

    while ((num_item = fscanf(fp, "%s%s%lf", 
                  denom, /* 분모 */
                  num,   /* 분자-분모 */
                  &prob)) != EOF) /* log(확률) */  {

      line_num++;

      if (num_item != 3) {
        fprintf(stderr, "line number = %d\n", line_num);
        return 0;
      }
      probs[denom][num] = prob;
      count++;
    }
  } // end of if


  // binary 모드이면
  else {
    char *denom;
    char *num;

    double prob;
    char *file_contents = NULL;

    if ((fp = fopen(filename, "rb")) == NULL) {
      fprintf(stderr, "Error: cannot open file [%s]\n", filename);
      return 0;
    }

    /* 화일의 크기를 알아냄 */
    fseek(fp, 0, SEEK_END); /* 화일의 끝 */
    long FileSize = ftell(fp);   /* 화일의 위치 */
    fseek(fp, 0, SEEK_SET); /* 화일의 처음 */

    file_contents = (char *) malloc(FileSize); /* 메모리 할당 */
    if (file_contents == NULL) {
      fprintf(stderr, "Not enough memory\n");
      return 0;
    }

    fread(file_contents, FileSize, 1, fp); /* 화일 전체를 읽어들임 */

    //file_contents[FileSize] = 0; /* NULL */
    
    char *ptr;
    ptr = file_contents;

    while (1) {

      if (ptr - file_contents >= FileSize) break; /* 화일 내용을 다 읽었으면 중지 */

      denom = ptr; /* 분모 */
      ptr += (strlen(denom) + 1);

      num = ptr; /* 분자 */
      ptr += (strlen(num) + 1);

      memcpy(&prob, ptr, sizeof(double)); /* 확률 */
      ptr += sizeof(double);
      
      probs[denom][num] = prob;

      count++;
    }
   
    free(file_contents); /* 메모리 해제 */
    
  } // end of else

  fclose(fp);

  return count;
}

/******************************************************************************/
/* 태그 집합을 알아냄 */
void get_tagset(PROB_MAP &probs, TAGSET &tagset) {

  /* word */
  for (PROB_MAP::iterator denom = probs.begin(); denom != probs.end(); ++denom)  {
    /* tag */
    for (NUM_PROB::iterator num = probs[denom->first].begin(); 
      num != probs[denom->first].end(); ++num) {

        tagset[num->first]++;
    }
  }
}
