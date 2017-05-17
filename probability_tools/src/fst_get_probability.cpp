#include "probability_tool.h"

//using namespace std;

/*****************************************************************************/
/* num : ���� */
// P(num)
double fst_get_probability1(void *fst_fst, double *fst_prob, 
                            string num) {

  //int i;
  int n;
  int num_index;
  string str_to_find = NULLSTRING DELIM_PROB+num; // P(����|***)
  
  //**/fprintf(stderr, "%s\n", str_to_find.c_str());

  // ���ڿ� -> �ε���(�ؽ�)��
  if ((n = String2Hash(fst_fst, (char *)str_to_find.c_str(), &num_index))== (-1)) { // ������
    return ALMOST_ZERO; /* ���� 0 */
  }
  else {

    return (fst_prob[n]);
    //for (i = 0; i < num_index; i++) { // �������� �м� ����� ���� ���
    //  // ��� : �ε�����ȣ, ���ڿ�, ī�װ�, ��
    //  return (fst_prob[n]);
    //  n++; // ����
    //}
  }
}


/*****************************************************************************/
/* num : ���� */
/* t4 : �и� */
double fst_get_probability2(void *fst_fst, double *fst_prob, 
                            string num, string t4) {

  //int i;
  int n;
  int num_index;
  string str_to_find = t4+DELIM_PROB+num; // P(����|�и�)

  // ���ڿ� -> �ε���(�ؽ�)��
  if ((n = String2Hash(fst_fst, (char *)str_to_find.c_str(), &num_index))== (-1)) { // ������
    return fst_get_probability1(fst_fst, fst_prob, num);
  }
  else {
    return (fst_prob[n]);
  }
}

/*****************************************************************************/
/* num : ���� */
/* t3+t4 : �и� */
double fst_get_probability3(void *fst_fst, double *fst_prob, 
                            string num, string t3, string t4) {

  //int i;
  int n;
  int num_index;
  string str_to_find = t3+t4+DELIM_PROB+num; // P(����|�и�)

  // ���ڿ� -> �ε���(�ؽ�)��
  if ((n = String2Hash(fst_fst, (char *)str_to_find.c_str(), &num_index))== (-1)) { // ������
    return fst_get_probability2(fst_fst, fst_prob, num, t4);
  }
  else {
    return (fst_prob[n]);
  }
}

/*****************************************************************************/
/* num : ���� */
/* t2+t3+t4 : �и� */
double fst_get_probability4(void *fst_fst, double *fst_prob, 
                            string num, string t2, string t3, string t4) {

  //int i;
  int n;
  int num_index;
  string str_to_find = t2+t3+t4+DELIM_PROB+num; // P(����|�и�)

  // ���ڿ� -> �ε���(�ؽ�)��
  if ((n = String2Hash(fst_fst, (char *)str_to_find.c_str(), &num_index))== (-1)) { // ������
    return fst_get_probability3(fst_fst, fst_prob, num, t3, t4);
  }
  else {
    return (fst_prob[n]);
  }
}

/*****************************************************************************/
/* num : ���� */
/* t1+t2+t3+t4 : �и� */
double fst_get_probability5(void *fst_fst, double *fst_prob, 
                            string num, string t1, string t2, string t3, string t4) {

  //int i;
  int n;
  int num_index;
  string str_to_find = t1+t2+t3+t4+DELIM_PROB+num; // P(����|�и�)

  // ���ڿ� -> �ε���(�ؽ�)��
  if ((n = String2Hash(fst_fst, (char *)str_to_find.c_str(), &num_index))== (-1)) { // ������
    return fst_get_probability4(fst_fst, fst_prob, num, t2, t3, t4);
  }
  else {
    return (fst_prob[n]);
  }
}
