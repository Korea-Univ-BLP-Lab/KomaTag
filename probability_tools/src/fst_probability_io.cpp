#include <stdio.h>
#include <vector>
#include <string>
#include "probability_tool.h"
//#include "FST.h"

using namespace std;
/******************************************************************************/
/* 확률을 파일에 출력 */
int fst_print_probability(char *fst_filename, char *hash_filename, char *prob_filename, PROB_MAP &probs) {
  
  double prob;
  
  char *TEMP_FILENAME = "$$$.$$";
  
  FILE *list_fp, *prob_fp;
    
  if ((list_fp = fopen(TEMP_FILENAME, "wt")) == NULL) {
    fprintf(stderr, "Error: cannot open a temporary file\n");
    return 0;
  }
  
  if ((prob_fp = fopen(prob_filename, "wb")) == NULL) {
    fprintf(stderr, "Error: cannot open file [%s]\n", prob_filename);
    return 0;
  }


  // 저장된 모든 확률값들에 대해
  
  // 분모
  for (PROB_MAP::iterator denom = probs.begin(); denom != probs.end(); ++denom)  {

    // 분자
    for (NUM_PROB::iterator num = probs[denom->first].begin(); 
      num != probs[denom->first].end(); ++num) {

      prob = probs[denom->first][num->first]; // 확률값
      
      fprintf(list_fp, "%s%s%s\n", denom->first.c_str(), /* 분모 */ 
                                   DELIM_PROB,
                                   num->first.c_str() /* 분자 */);
      
      fwrite(&prob, sizeof(double), 1, prob_fp); // 확률값
      
    }
  }

  fclose(list_fp);
  fclose(prob_fp);
  
  // fst 만들기
  build_fst(TEMP_FILENAME, fst_filename, hash_filename);
  
  remove(TEMP_FILENAME); // 파일 삭제
  
  return 1;
}

/******************************************************************************/
// 확률을 파일에 출력
// P(분자|분모) = freq(분자"|분모) / freq(분모) = 확률

// 입력 : probs (확률 구조)

// denom_fst_filename : 분모에 대한 FST 파일 (hash 파일은 끝에 .hash가 붙어서 자동 생성됨)
// fst_filename : 분모-분자에 대한 FST 파일      (hash 파일은 끝에 .hash가 붙어서 자동 생성됨)
// numerator_filename : 분자에 대한 파일 (문자열)
// prob_filename : 확률 파일 (double)
int fst_print_probability2(char *denom_fst_filename, char *numerator_filename, char *prob_filename, 
                           PROB_MAP &probs) {
  
  double prob;
  
  char *DENOM_FILENAME = "$$$.DENOM.$$$";
  
  
  FILE *denom_fp, *num_fp, *prob_fp;
    
  if ((denom_fp = fopen(DENOM_FILENAME, "wt")) == NULL) {
    fprintf(stderr, "Error: cannot open a temporary file\n");
    return 0;
  }

  if ((num_fp = fopen(numerator_filename, "wt")) == NULL) {
    fprintf(stderr, "Error: cannot open a temporary file\n");
    return 0;
  }
  
  if ((prob_fp = fopen(prob_filename, "wb")) == NULL) {
    fprintf(stderr, "Error: cannot open file [%s]\n", prob_filename);
    return 0;
  }


  // 저장된 모든 확률값들에 대해
  
  // 분모
  for (PROB_MAP::iterator denom = probs.begin(); denom != probs.end(); ++denom)  {

    // 분자
    for (NUM_PROB::iterator num = probs[denom->first].begin(); 
      num != probs[denom->first].end(); ++num) {

      prob = probs[denom->first][num->first]; // 확률값
      
      // 분모
      fprintf(denom_fp, "%s\n", denom->first.c_str());

      // 분자
      fprintf(num_fp, "%s\n", num->first.c_str());
      
      // 확률값
      fwrite(&prob, sizeof(double), 1, prob_fp); // 확률값
      
    }
  }

  fclose(denom_fp);
  fclose(num_fp);
  fclose(prob_fp);
  
  string hash_filename;

  hash_filename = denom_fst_filename;
  hash_filename += ".hash";

  // fst 만들기
  fprintf(stderr, "분모에 대한 FST 파일을 만드고 있습니다.\n");
  build_fst(DENOM_FILENAME, denom_fst_filename, (char *) hash_filename.c_str());

  remove(DENOM_FILENAME); // 파일 삭제
  
  return 1;
}

/*****************************************************************************/
// 파일로부터 double값들을 읽어들인다.
static double *Load_double(char *filename) {
  FILE *infofp;
  long FileSize = 0;
  double *fst_info;

  if ((infofp = fopen (filename, "rb")) == NULL) {
    fprintf (stderr, "ERROR: Can't open information file [%s]!\n", filename);
    return NULL;
  }

  fseek (infofp, 0, SEEK_END);
  FileSize = ftell (infofp);
  fseek (infofp, 0, SEEK_SET);
  fst_info = (double *) malloc (FileSize + 1);

  if (!fst_info) {
    fprintf (stderr, "ERROR: Not enough memory!\n");
    return NULL;
  }
  
  fread (fst_info, sizeof (double), FileSize / sizeof (double), infofp); // 읽기
  
  fclose (infofp);
  
  return fst_info;
}

/*****************************************************************************/
// 파일로부터 문자열을 읽어서 string 벡터에 삽입
static int Load_string(char *filename, vector<string> &list) {
  char temp_str[1024];
  FILE *fp;

  if ((fp = fopen (filename, "rt")) == NULL) {
    fprintf (stderr, "ERROR: Can't open file [%s]!\n", filename);
    return 0;
  }

  while (fgets(temp_str, 1024, fp) != NULL) {
    temp_str[strlen(temp_str)-1] = 0; // 엔터 제거

    list.push_back(temp_str);
  }
  return 1;
}

/*****************************************************************************/
// fst 파일과 확률값 파일을 읽어들인다.
int fst_probability_open(char *fst_filename, char *hash_filename, char *fst_PROB_filename, 
                   void **fst_fst, double **fst_prob) {

  // fst 열기
  fprintf(stderr, "\tReading FST file.. [%s]", fst_filename);
  if (!(*fst_fst = LoadTransducer (fst_filename, hash_filename))) {
    fprintf (stderr, "[ERROR] Cannot open fst! [%s]\n", fst_filename);
    return 0;
  }
  fprintf(stderr, "\t[done]\n");
  
  // 확률 화일 열기
  fprintf(stderr, "\tReading probability file.. [%s]", fst_PROB_filename);
  if ((*fst_prob = Load_double (fst_PROB_filename)) == NULL) { // 확률 읽기
    fprintf (stderr, "[ERROR] Cannot open FST! [%s]\n", fst_PROB_filename);
    return 0;
  }
  fprintf(stderr, "\t[done]\n");

  return 1;
}

/*****************************************************************************/
// fst 파일과 확률값 파일을 읽어들인다.
int fst_probability_open2(char *denom_fst_filename, char *numerator_filename, char *fst_PROB_filename, 
                         void **denom_fst, vector<string> &numerator, double **fst_prob) {

  // fst 열기
  fprintf(stderr, "\tReading FST file.. [%s]", denom_fst_filename);
  if (!(*denom_fst = LoadTransducer (denom_fst_filename, NULL))) {
    fprintf (stderr, "[ERROR] Cannot open fst! [%s]\n", denom_fst_filename);
    return 0;
  }
  fprintf(stderr, "\t[done]\n");
  
  // 분자 화일 열기
  fprintf(stderr, "\tReading numerator file.. [%s]", numerator_filename);
  if (!Load_string (numerator_filename, numerator)) { // 문자열 읽기
    fprintf (stderr, "[ERROR] Cannot open numerator file! [%s]\n", numerator_filename);
    return 0;
  }
  fprintf(stderr, "\t[done]\n");

  // 확률 화일 열기
  fprintf(stderr, "\tReading probability file.. [%s]", fst_PROB_filename);
  if ((*fst_prob = Load_double (fst_PROB_filename)) == NULL) { // 확률 읽기
    fprintf (stderr, "[ERROR] Cannot open probability file! [%s]\n", fst_PROB_filename);
    return 0;
  }
  fprintf(stderr, "\t[done]\n");


  ///**/for (unsigned int i = 0; i < numerator.size(); i++) {
  //  fprintf(stderr, "%d) %s\n", i, numerator[i].c_str());
  //}

  return 1;
}

/*****************************************************************************/
// 메모리 해제
/* 이 함수의 호출은 : fst_close(fst_fst, fst_freq); 와 같이 한다. */
void fst_probability_close(void *fst_fst, double *fst_prob) {

  if (fst_fst) FreeTransducer(fst_fst);      /* FST */

  /* 메모리 해제 */
  if (fst_prob) free (fst_prob);
  fst_prob = NULL;

}

