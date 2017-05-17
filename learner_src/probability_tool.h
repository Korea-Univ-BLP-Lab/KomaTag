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
#define ALMOST_ZERO 1.0e-100 /* ������ 0�� ���� �Ǹ� log(0) = inf�� �Ǿ ���� �߻� ���� */
#define LOG_ALMOST_ZERO -100

//#define DELIM "_|"
#define DELIM_PROB "\"|" // "�� ���� ���� : ���� �������� (��°����� ���ں��� �� ���� ���ڸ� �����)
#define NULLSTRING "***"

////////////////////////////////////////////////////////////////////////////////
typedef map<string, int>       C_FREQ;     /* ���� �� */
typedef map<string, C_FREQ>    CC_FREQ;    /* ���� ���� �� */
typedef map<string, CC_FREQ>   CCC_FREQ;   /* ���� ���� ���� �� */
typedef map<string, CCC_FREQ>  CCCC_FREQ;  /* ���� ���� ���� ���� �� */
typedef map<string, CCCC_FREQ> CCCCC_FREQ; /* ���� ���� ���� ���� ���� �� */


typedef map<string, double>           NUM_PROB; /* ���� Ȯ���� */
typedef map<string, NUM_PROB>         PROB_MAP; /* �и� ���� Ȯ���� */

typedef map <string, int>             TAGSET; /* �±� �� */

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

// fst_filename : �и�-���ڿ� ���� FST ����
extern int fst_print_probability(char *fst_filename, char *hash_filename, char *prob_filename, PROB_MAP &probs);

// Ȯ���� ���Ͽ� ���
// P(����|�и�) = freq(����"|�и�) / freq(�и�) = Ȯ��
// �Է� : probs (Ȯ�� ����)
// numerator_filename : ���ڿ� ���� ���� (���ڿ�)
// prob_filename : Ȯ�� ���� (double)
extern int fst_print_probability2(char *denom_fst_filename, char *numerator_filename, char *prob_filename, PROB_MAP &probs);

// �и�-����, Ȯ��
extern int fst_probability_open(char *fst_filename, char *hash_filename, char *fst_PROB_filename, void **fst_fst, double **fst_prob);

// �и�, ����, Ȯ��
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
