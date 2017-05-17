#include <stdio.h>
#include <vector>
#include <string>
#include "probability_tool.h"
//#include "FST.h"

using namespace std;
/******************************************************************************/
/* Ȯ���� ���Ͽ� ��� */
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


  // ����� ��� Ȯ�����鿡 ����
  
  // �и�
  for (PROB_MAP::iterator denom = probs.begin(); denom != probs.end(); ++denom)  {

    // ����
    for (NUM_PROB::iterator num = probs[denom->first].begin(); 
      num != probs[denom->first].end(); ++num) {

      prob = probs[denom->first][num->first]; // Ȯ����
      
      fprintf(list_fp, "%s%s%s\n", denom->first.c_str(), /* �и� */ 
                                   DELIM_PROB,
                                   num->first.c_str() /* ���� */);
      
      fwrite(&prob, sizeof(double), 1, prob_fp); // Ȯ����
      
    }
  }

  fclose(list_fp);
  fclose(prob_fp);
  
  // fst �����
  build_fst(TEMP_FILENAME, fst_filename, hash_filename);
  
  remove(TEMP_FILENAME); // ���� ����
  
  return 1;
}

/******************************************************************************/
// Ȯ���� ���Ͽ� ���
// P(����|�и�) = freq(����"|�и�) / freq(�и�) = Ȯ��

// �Է� : probs (Ȯ�� ����)

// denom_fst_filename : �и� ���� FST ���� (hash ������ ���� .hash�� �پ �ڵ� ������)
// fst_filename : �и�-���ڿ� ���� FST ����      (hash ������ ���� .hash�� �پ �ڵ� ������)
// numerator_filename : ���ڿ� ���� ���� (���ڿ�)
// prob_filename : Ȯ�� ���� (double)
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


  // ����� ��� Ȯ�����鿡 ����
  
  // �и�
  for (PROB_MAP::iterator denom = probs.begin(); denom != probs.end(); ++denom)  {

    // ����
    for (NUM_PROB::iterator num = probs[denom->first].begin(); 
      num != probs[denom->first].end(); ++num) {

      prob = probs[denom->first][num->first]; // Ȯ����
      
      // �и�
      fprintf(denom_fp, "%s\n", denom->first.c_str());

      // ����
      fprintf(num_fp, "%s\n", num->first.c_str());
      
      // Ȯ����
      fwrite(&prob, sizeof(double), 1, prob_fp); // Ȯ����
      
    }
  }

  fclose(denom_fp);
  fclose(num_fp);
  fclose(prob_fp);
  
  string hash_filename;

  hash_filename = denom_fst_filename;
  hash_filename += ".hash";

  // fst �����
  fprintf(stderr, "�и� ���� FST ������ ����� �ֽ��ϴ�.\n");
  build_fst(DENOM_FILENAME, denom_fst_filename, (char *) hash_filename.c_str());

  remove(DENOM_FILENAME); // ���� ����
  
  return 1;
}

/*****************************************************************************/
// ���Ϸκ��� double������ �о���δ�.
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
  
  fread (fst_info, sizeof (double), FileSize / sizeof (double), infofp); // �б�
  
  fclose (infofp);
  
  return fst_info;
}

/*****************************************************************************/
// ���Ϸκ��� ���ڿ��� �о string ���Ϳ� ����
static int Load_string(char *filename, vector<string> &list) {
  char temp_str[1024];
  FILE *fp;

  if ((fp = fopen (filename, "rt")) == NULL) {
    fprintf (stderr, "ERROR: Can't open file [%s]!\n", filename);
    return 0;
  }

  while (fgets(temp_str, 1024, fp) != NULL) {
    temp_str[strlen(temp_str)-1] = 0; // ���� ����

    list.push_back(temp_str);
  }
  return 1;
}

/*****************************************************************************/
// fst ���ϰ� Ȯ���� ������ �о���δ�.
int fst_probability_open(char *fst_filename, char *hash_filename, char *fst_PROB_filename, 
                   void **fst_fst, double **fst_prob) {

  // fst ����
  fprintf(stderr, "\tReading FST file.. [%s]", fst_filename);
  if (!(*fst_fst = LoadTransducer (fst_filename, hash_filename))) {
    fprintf (stderr, "[ERROR] Cannot open fst! [%s]\n", fst_filename);
    return 0;
  }
  fprintf(stderr, "\t[done]\n");
  
  // Ȯ�� ȭ�� ����
  fprintf(stderr, "\tReading probability file.. [%s]", fst_PROB_filename);
  if ((*fst_prob = Load_double (fst_PROB_filename)) == NULL) { // Ȯ�� �б�
    fprintf (stderr, "[ERROR] Cannot open FST! [%s]\n", fst_PROB_filename);
    return 0;
  }
  fprintf(stderr, "\t[done]\n");

  return 1;
}

/*****************************************************************************/
// fst ���ϰ� Ȯ���� ������ �о���δ�.
int fst_probability_open2(char *denom_fst_filename, char *numerator_filename, char *fst_PROB_filename, 
                         void **denom_fst, vector<string> &numerator, double **fst_prob) {

  // fst ����
  fprintf(stderr, "\tReading FST file.. [%s]", denom_fst_filename);
  if (!(*denom_fst = LoadTransducer (denom_fst_filename, NULL))) {
    fprintf (stderr, "[ERROR] Cannot open fst! [%s]\n", denom_fst_filename);
    return 0;
  }
  fprintf(stderr, "\t[done]\n");
  
  // ���� ȭ�� ����
  fprintf(stderr, "\tReading numerator file.. [%s]", numerator_filename);
  if (!Load_string (numerator_filename, numerator)) { // ���ڿ� �б�
    fprintf (stderr, "[ERROR] Cannot open numerator file! [%s]\n", numerator_filename);
    return 0;
  }
  fprintf(stderr, "\t[done]\n");

  // Ȯ�� ȭ�� ����
  fprintf(stderr, "\tReading probability file.. [%s]", fst_PROB_filename);
  if ((*fst_prob = Load_double (fst_PROB_filename)) == NULL) { // Ȯ�� �б�
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
// �޸� ����
/* �� �Լ��� ȣ���� : fst_close(fst_fst, fst_freq); �� ���� �Ѵ�. */
void fst_probability_close(void *fst_fst, double *fst_prob) {

  if (fst_fst) FreeTransducer(fst_fst);      /* FST */

  /* �޸� ���� */
  if (fst_prob) free (fst_prob);
  fst_prob = NULL;

}

