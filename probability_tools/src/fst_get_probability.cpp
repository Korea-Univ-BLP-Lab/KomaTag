#include "probability_tool.h"

//using namespace std;

/*****************************************************************************/
/* num : 분자 */
// P(num)
double fst_get_probability1(void *fst_fst, double *fst_prob, 
                            string num) {

  //int i;
  int n;
  int num_index;
  string str_to_find = NULLSTRING DELIM_PROB+num; // P(분자|***)
  
  //**/fprintf(stderr, "%s\n", str_to_find.c_str());

  // 문자열 -> 인덱스(해쉬)값
  if ((n = String2Hash(fst_fst, (char *)str_to_find.c_str(), &num_index))== (-1)) { // 없으면
    return ALMOST_ZERO; /* 거의 0 */
  }
  else {

    return (fst_prob[n]);
    //for (i = 0; i < num_index; i++) { // 복수개의 분석 결과가 있을 경우
    //  // 출력 : 인덱스번호, 문자열, 카테고리, 빈도
    //  return (fst_prob[n]);
    //  n++; // 증가
    //}
  }
}


/*****************************************************************************/
/* num : 분자 */
/* t4 : 분모 */
double fst_get_probability2(void *fst_fst, double *fst_prob, 
                            string num, string t4) {

  //int i;
  int n;
  int num_index;
  string str_to_find = t4+DELIM_PROB+num; // P(분자|분모)

  // 문자열 -> 인덱스(해쉬)값
  if ((n = String2Hash(fst_fst, (char *)str_to_find.c_str(), &num_index))== (-1)) { // 없으면
    return fst_get_probability1(fst_fst, fst_prob, num);
  }
  else {
    return (fst_prob[n]);
  }
}

/*****************************************************************************/
/* num : 분자 */
/* t3+t4 : 분모 */
double fst_get_probability3(void *fst_fst, double *fst_prob, 
                            string num, string t3, string t4) {

  //int i;
  int n;
  int num_index;
  string str_to_find = t3+t4+DELIM_PROB+num; // P(분자|분모)

  // 문자열 -> 인덱스(해쉬)값
  if ((n = String2Hash(fst_fst, (char *)str_to_find.c_str(), &num_index))== (-1)) { // 없으면
    return fst_get_probability2(fst_fst, fst_prob, num, t4);
  }
  else {
    return (fst_prob[n]);
  }
}

/*****************************************************************************/
/* num : 분자 */
/* t2+t3+t4 : 분모 */
double fst_get_probability4(void *fst_fst, double *fst_prob, 
                            string num, string t2, string t3, string t4) {

  //int i;
  int n;
  int num_index;
  string str_to_find = t2+t3+t4+DELIM_PROB+num; // P(분자|분모)

  // 문자열 -> 인덱스(해쉬)값
  if ((n = String2Hash(fst_fst, (char *)str_to_find.c_str(), &num_index))== (-1)) { // 없으면
    return fst_get_probability3(fst_fst, fst_prob, num, t3, t4);
  }
  else {
    return (fst_prob[n]);
  }
}

/*****************************************************************************/
/* num : 분자 */
/* t1+t2+t3+t4 : 분모 */
double fst_get_probability5(void *fst_fst, double *fst_prob, 
                            string num, string t1, string t2, string t3, string t4) {

  //int i;
  int n;
  int num_index;
  string str_to_find = t1+t2+t3+t4+DELIM_PROB+num; // P(분자|분모)

  // 문자열 -> 인덱스(해쉬)값
  if ((n = String2Hash(fst_fst, (char *)str_to_find.c_str(), &num_index))== (-1)) { // 없으면
    return fst_get_probability4(fst_fst, fst_prob, num, t2, t3, t4);
  }
  else {
    return (fst_prob[n]);
  }
}
