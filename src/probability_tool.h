#ifndef __fst_probability_H__
#define __fst_probability_H__

#pragma warning(disable: 4786)
#pragma warning(disable: 4503)

#include <map>
#include <vector>
#include <string>
#include "FST.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
#define ALMOST_ZERO 1.0e-100 /* 완전히 0을 쓰게 되면 log(0) = inf가 되어서 문제 발생 가능 */
#define LOG_ALMOST_ZERO -100

//#define DELIM "_|"
#define DELIM_PROB "\"|" // "를 쓰는 이유 : 정렬 문제때문 (출력가능한 문자보다 더 빠른 문자를 써야함)
#define NULLSTRING "***"

////////////////////////////////////////////////////////////////////////////////
typedef map<string, int>       C_FREQ;     /* 문자 빈도 */
typedef map<string, C_FREQ>    CC_FREQ;    /* 문자 문자 빈도 */
typedef map<string, CC_FREQ>   CCC_FREQ;   /* 문자 문자 문자 빈도 */
typedef map<string, CCC_FREQ>  CCCC_FREQ;  /* 문자 문자 문자 문자 빈도 */
typedef map<string, CCCC_FREQ> CCCCC_FREQ; /* 문자 문자 문자 문자 문자 빈도 */


typedef map<string, double>           NUM_PROB; /* 분자 확률값 */
typedef map<string, NUM_PROB>         PROB_MAP; /* 분모 분자 확률값 */

typedef map <string, int>             TAGSET; /* 태그 빈도 */

////////////////////////////////////////////////////////////////////////////////
extern int calc_MLE_probability(C_FREQ &unigram_freq, CC_FREQ &bigram_freq, PROB_MAP &prob);
extern int calc_MLE_probability_with_freq_threshold(C_FREQ &unigram_freq, CC_FREQ &bigram_freq, 
                                             PROB_MAP &prob, int threshold);

extern int calc_LINEAR_INTERPOL_probability5(PROB_MAP &prob_map,
                                             CCCCC_FREQ &_12345_freq, CCCC_FREQ &_1234_freq,
                                             CCCC_FREQ &_2345_freq, CCC_FREQ &_234_freq, 
                                             CCC_FREQ &_345_freq, CC_FREQ &_34_freq, 
                                             CC_FREQ &_45_freq, C_FREQ &_4_freq, 
                                             C_FREQ &_5_freq);

////////////////////////////////////////////////////////////////////////////////
extern double map_get_probability1(PROB_MAP &probs, string num);
extern double map_get_probability2(PROB_MAP &probs, string num, string denom);
extern double map_get_probability3(PROB_MAP &probs, string num, string t1, string t2);
extern double map_get_probability4(PROB_MAP &probs, string num, string t1, string t2, string t3);
extern double map_get_probability5(PROB_MAP &probs, string num, string t1, string t2, string t3, string t4);

extern int map_print_probability(char *filename, PROB_MAP &probs, char *mode);
extern int map_scan_probability(char *filename, PROB_MAP &probs, char *mode);

extern void get_tagset(PROB_MAP &probs, TAGSET &tagset);

////////////////////////////////////////////////////////////////////////////////

// fst_filename : 분모-분자에 대한 FST 파일
extern int fst_print_probability(char *fst_filename, char *hash_filename, char *prob_filename, PROB_MAP &probs);

// 확률을 파일에 출력
// P(분자|분모) = freq(분자"|분모) / freq(분모) = 확률
// 입력 : probs (확률 구조)
// numerator_filename : 분자에 대한 파일 (문자열)
// prob_filename : 확률 파일 (double)
extern int fst_print_probability2(char *denom_fst_filename, char *numerator_filename, char *prob_filename, PROB_MAP &probs);

// 분모-분자, 확률
extern int fst_probability_open(char *fst_filename, char *hash_filename, char *fst_PROB_filename, void **fst_fst, double **fst_prob);

// 분모, 분자, 확률
extern int fst_probability_open2(char *denom_fst_filename, char *numerator_filename, char *fst_PROB_filename, 
                                 void **denom_fst, vector<string> &numerator, double **fst_prob);

extern void fst_probability_close(void *fst_fst, double *fst_prob);

extern double fst_get_probability1(void *fst_fst, double *fst_prob, string num);
extern double fst_get_probability2(void *fst_fst, double *fst_prob, string num, string t4);
extern double fst_get_probability3(void *fst_fst, double *fst_prob, string num, string t3, string t4);
extern double fst_get_probability4(void *fst_fst, double *fst_prob, string num, string t2, string t3, string t4);
extern double fst_get_probability5(void *fst_fst, double *fst_prob, string num, string t1, string t2, string t3, string t4);

////////////////////////////////////////////////////////////////////////////////

#endif
