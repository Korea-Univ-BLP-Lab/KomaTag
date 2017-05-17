#include "probability_tool.h"

//using namespace std;


/******************************************************************************/
int calc_MLE_probability_with_freq_threshold(C_FREQ &unigram_freq, CC_FREQ &bigram_freq, 
                                             PROB_MAP &prob, int threshold) {
  
  /* 전이 확률 */
  /* 태그에 대한 loop */
  for (CC_FREQ::iterator itr2 = bigram_freq.begin(); itr2 != bigram_freq.end(); ++itr2) {

    for (C_FREQ::iterator itr3 = bigram_freq[itr2->first].begin(); 
      itr3 != bigram_freq[itr2->first].end(); ++itr3) {

      /* t1t2 t3 */
      ///**/fprintf(stdout, "%s %s -> %e\n", itr2->first.c_str(), itr3->first.c_str(), (double)itr3->second/unigram_freq[itr2->first]);
      if (itr3->second >= threshold) { /* 빈도가 threshold 이상인 경우에만 저장 */
        prob[itr2->first][itr3->first] = (double)itr3->second/unigram_freq[itr2->first];
      }
    }
  }
  return 1;
}

/******************************************************************************/
int calc_MLE_probability(C_FREQ &unigram_freq, CC_FREQ &bigram_freq, PROB_MAP &prob) {

  /* 전이 확률 */
  /* 태그에 대한 loop */
  for (CC_FREQ::iterator itr2 = bigram_freq.begin(); itr2 != bigram_freq.end(); ++itr2) {

    for (C_FREQ::iterator itr3 = bigram_freq[itr2->first].begin(); 
      itr3 != bigram_freq[itr2->first].end(); ++itr3) {

      /* t1t2 t3 */
      ///**/fprintf(stdout, "%s %s -> %e\n", itr2->first.c_str(), itr3->first.c_str(), (double)itr3->second/unigram_freq[itr2->first]);
      if (itr3->second > 1) { /* 빈도 2이상인 경우에만 저장 */
        prob[itr2->first][itr3->first] = (double)itr3->second/unigram_freq[itr2->first];
      }
    }
  }
  return 1;
}

/******************************************************************************/
/* 최대값의 번호(index)를 리턴 */
static int max_int(double *numbers, int num) {
  double cur_max = numbers[0];
  int cur_max_index = 0;

  for (int i = 1; i < num; i++) {
    if (cur_max < numbers[i]) {
      cur_max_index = i;
      cur_max = numbers[i];
    }
  }
  return cur_max_index;
}

/******************************************************************************/
static int calc_4_coefficient(double *coefficient,
               CCCC_FREQ &_1234_freq, CCC_FREQ &_123_freq,
               CCC_FREQ &_234_freq, CC_FREQ &_23_freq, 
               CC_FREQ &_34_freq, C_FREQ &_3_freq, 
               C_FREQ &_4_freq,
               C_FREQ &c_freq) {

  double ratio[4];
  int raw_coefficient[4] = {0,};
  int N = 0;

  CCCC_FREQ::iterator itr;
  CCC_FREQ::iterator itr2;
  CC_FREQ::iterator itr3;
  C_FREQ::iterator itr4;

  // 총 token의 수(N)를 계산
  for (itr4 = c_freq.begin(); itr4 != c_freq.end(); ++itr4) {
    N += itr4->second;
  }

  for (itr = _1234_freq.begin(); itr != _1234_freq.end(); ++itr)  {
    for (itr2 = _1234_freq[itr->first].begin(); itr2 != _1234_freq[itr->first].end(); ++itr2) {
      for (itr3 = _1234_freq[itr->first][itr2->first].begin(); itr3 != _1234_freq[itr->first][itr2->first].end(); ++itr3) {
        for (itr4 = _1234_freq[itr->first][itr2->first][itr3->first].begin(); itr4 != _1234_freq[itr->first][itr2->first][itr3->first].end(); ++itr4) {

          ratio[3] = (_123_freq[itr->first][itr2->first][itr3->first]-1 == 0)
            ? 0.0 : (double)(itr4->second-1) / (_123_freq[itr->first][itr2->first][itr3->first]-1); 

          ratio[2] = (_23_freq[itr2->first][itr3->first]-1 == 0) 
            ? 0.0 : (double)(_234_freq[itr2->first][itr3->first][itr4->first]-1) / (_23_freq[itr2->first][itr3->first]-1);

          ratio[1] = (_3_freq[itr3->first]-1 == 0) 
            ? 0.0 : (double)(_34_freq[itr3->first][itr4->first]-1) / (_3_freq[itr3->first]-1);

          ratio[0] = (double) (_4_freq[itr4->first]-1) / N;

          raw_coefficient[max_int(ratio, 4)] += itr4->second;
        }
      }
    }
  }

  // normalize coefficients
  int sum = raw_coefficient[0] + raw_coefficient[1] + raw_coefficient[2] + raw_coefficient[3];

  for (int i = 0; i < 4; i++) {
    coefficient[i] = (double) raw_coefficient[i] / sum;
    fprintf(stderr, "coefficient[%d] = %lf(%d/%d)\n", i, coefficient[i], raw_coefficient[i], sum);
  }

  return N; // token의 수
}

/******************************************************************************/
static int calc_linear_interpol_4_probs(PROB_MAP &prob, double *coefficient, int N,
                               CCCC_FREQ &_1234_freq, CCC_FREQ &_123_freq,
                               CCC_FREQ &_234_freq, CC_FREQ &_23_freq, 
                               CC_FREQ &_34_freq, C_FREQ &_3_freq,
                               C_FREQ &_4_freq) {

  double ratio[4];

  // 4개 짜리 확률
  {
    CCCC_FREQ::iterator itr;
    CCC_FREQ::iterator itr2;
    CC_FREQ::iterator itr3;
    C_FREQ::iterator itr4;

    for (itr = _1234_freq.begin(); itr != _1234_freq.end(); ++itr)  {
      for (itr2 = _1234_freq[itr->first].begin(); itr2 != _1234_freq[itr->first].end(); ++itr2) {
        for (itr3 = _1234_freq[itr->first][itr2->first].begin(); itr3 != _1234_freq[itr->first][itr2->first].end(); ++itr3) {
          for (itr4 = _1234_freq[itr->first][itr2->first][itr3->first].begin(); itr4 != _1234_freq[itr->first][itr2->first][itr3->first].end(); ++itr4) {

            ratio[3] = (_123_freq[itr->first][itr2->first][itr3->first] == 0) 
              ? ALMOST_ZERO : (double)itr4->second / _123_freq[itr->first][itr2->first][itr3->first];

            ratio[2] = (_23_freq[itr2->first][itr3->first] == 0) 
              ? ALMOST_ZERO : (double)_234_freq[itr2->first][itr3->first][itr4->first] / _23_freq[itr2->first][itr3->first];

            ratio[1] = (_3_freq[itr3->first] == 0)
              ? ALMOST_ZERO : (double)_34_freq[itr3->first][itr4->first] / _3_freq[itr3->first];

            ratio[0] = (double)_4_freq[itr4->first] / N;

            prob[itr->first+itr2->first+itr3->first][itr4->first] 
              = coefficient[3] * ratio[3] + coefficient[2] * ratio[2] + coefficient[1] * ratio[1] + coefficient[0] * ratio[0];

          } // end of for1
        } // end of for2
      } // end of for3
    } // end of for4
  }

  // 3개 짜리 확률
  {
    CCC_FREQ::iterator itr;
    CC_FREQ::iterator itr2;
    C_FREQ::iterator itr3;

    for (itr = _234_freq.begin(); itr != _234_freq.end(); ++itr)  {
      for (itr2 = _234_freq[itr->first].begin(); itr2 != _234_freq[itr->first].end(); ++itr2) {
        for (itr3 = _234_freq[itr->first][itr2->first].begin(); itr3 != _234_freq[itr->first][itr2->first].end(); ++itr3) {

          ratio[2] = (_23_freq[itr->first][itr2->first] == 0) 
            ? ALMOST_ZERO : (double)itr3->second / _23_freq[itr->first][itr2->first];

          ratio[1] = (_3_freq[itr2->first] == 0)
            ? ALMOST_ZERO : (double)_34_freq[itr2->first][itr3->first] / _3_freq[itr2->first];

          ratio[0] = (double)_4_freq[itr3->first] / N;

          prob[itr->first+itr2->first][itr3->first] 
            = coefficient[2] * ratio[2] + coefficient[1] * ratio[1] + coefficient[0] * ratio[0];

        } // end of for1
      } // end of for2
    } // end of for3
  }

  // 2개 짜리 확률
  {
    CC_FREQ::iterator itr;
    C_FREQ::iterator itr2;

    for (itr = _34_freq.begin(); itr != _34_freq.end(); ++itr)  {
      for (itr2 = _34_freq[itr->first].begin(); itr2 != _34_freq[itr->first].end(); ++itr2) {
 
        ratio[1] = (_3_freq[itr->first] == 0)
          ? ALMOST_ZERO : (double)itr2->second / _3_freq[itr->first];

        ratio[0] = (double)_4_freq[itr2->first] / N;

        prob[itr->first][itr2->first] 
          = coefficient[1] * ratio[1] + coefficient[0] * ratio[0];

      } // end of for1
    } // end of for2
  }
  return 1;
}

/******************************************************************************/
static int calc_5_coefficient(double *coefficient,
                               CCCCC_FREQ &_12345_freq, CCCC_FREQ &_1234_freq,
                               CCCC_FREQ &_2345_freq, CCC_FREQ &_234_freq, 
                               CCC_FREQ &_345_freq, CC_FREQ &_34_freq,
                               CC_FREQ &_45_freq, C_FREQ &_4_freq, 
                               C_FREQ &_5_freq,
                               C_FREQ &c_freq) {


  double ratio[5];
  int raw_coefficient[5] = {0,};
  int N = 0;

  CCCCC_FREQ::iterator itr;
  CCCC_FREQ::iterator itr2;
  CCC_FREQ::iterator itr3;
  CC_FREQ::iterator itr4;
  C_FREQ::iterator itr5;

  // 총 token의 수(N)를 계산
  for (itr5 = c_freq.begin(); itr5 != c_freq.end(); ++itr5) {
    N += itr5->second;
  }

  for (itr = _12345_freq.begin(); itr != _12345_freq.end(); ++itr)  {
    for (itr2 = _12345_freq[itr->first].begin(); itr2 != _12345_freq[itr->first].end(); ++itr2) {
      for (itr3 = _12345_freq[itr->first][itr2->first].begin(); itr3 != _12345_freq[itr->first][itr2->first].end(); ++itr3) {
        for (itr4 = _12345_freq[itr->first][itr2->first][itr3->first].begin(); itr4 != _12345_freq[itr->first][itr2->first][itr3->first].end(); ++itr4) {
          for (itr5 = _12345_freq[itr->first][itr2->first][itr3->first][itr4->first].begin(); itr5 != _12345_freq[itr->first][itr2->first][itr3->first][itr4->first].end(); ++itr5) {

            ratio[4] = (_1234_freq[itr->first][itr2->first][itr3->first][itr4->first]-1 == 0)
              ? 0.0 : (double)(itr5->second-1) / (_1234_freq[itr->first][itr2->first][itr3->first][itr4->first]-1); 

            ratio[3] = (_234_freq[itr2->first][itr3->first][itr4->first]-1 == 0) 
              ? 0.0 : (double)(_2345_freq[itr2->first][itr3->first][itr4->first][itr5->first]-1) / (_234_freq[itr2->first][itr3->first][itr4->first]-1);

            ratio[2] = (_34_freq[itr3->first][itr4->first]-1 == 0) 
              ? 0.0 : (double)(_345_freq[itr3->first][itr4->first][itr5->first]-1) / (_34_freq[itr3->first][itr4->first]-1);

            ratio[1] = (_4_freq[itr4->first]-1 == 0)
              ? 0.0 : (double)(_45_freq[itr4->first][itr5->first]-1) / (_4_freq[itr4->first]-1);

            ratio[0] = (double)_5_freq[itr5->first] / N;

            raw_coefficient[max_int(ratio, 5)] += itr5->second;
          }
        }
      }
    }
  }

  // normalize coefficients
  int sum = raw_coefficient[0] + raw_coefficient[1] + raw_coefficient[2] + raw_coefficient[3] + raw_coefficient[4];

  for (int i = 0; i < 5; i++) {
    coefficient[i] = (double) raw_coefficient[i] / sum;
    fprintf(stderr, "coefficient[%d] = %lf(%d/%d)\n", i, coefficient[i], raw_coefficient[i], sum);
  }

  return N; // token의 수
}

/******************************************************************************/
static int calc_linear_interpol_5_probs(PROB_MAP &prob, double *coefficient, int N,
                               CCCCC_FREQ &_12345_freq, CCCC_FREQ &_1234_freq,
                               CCCC_FREQ &_2345_freq, CCC_FREQ &_234_freq, 
                               CCC_FREQ &_345_freq, CC_FREQ &_34_freq,
                               CC_FREQ &_45_freq, C_FREQ &_4_freq, 
                               C_FREQ &_5_freq) {

  double ratio[5];

  // 5개 짜리 확률
  {
    CCCCC_FREQ::iterator itr;
    CCCC_FREQ::iterator itr2;
    CCC_FREQ::iterator itr3;
    CC_FREQ::iterator itr4;
    C_FREQ::iterator itr5;

    for (itr = _12345_freq.begin(); itr != _12345_freq.end(); ++itr)  {
      for (itr2 = _12345_freq[itr->first].begin(); itr2 != _12345_freq[itr->first].end(); ++itr2) {
        for (itr3 = _12345_freq[itr->first][itr2->first].begin(); itr3 != _12345_freq[itr->first][itr2->first].end(); ++itr3) {
          for (itr4 = _12345_freq[itr->first][itr2->first][itr3->first].begin(); itr4 != _12345_freq[itr->first][itr2->first][itr3->first].end(); ++itr4) {
            for (itr5 = _12345_freq[itr->first][itr2->first][itr3->first][itr4->first].begin(); itr5 != _12345_freq[itr->first][itr2->first][itr3->first][itr4->first].end(); ++itr5) {

              ratio[4] = (_1234_freq[itr->first][itr2->first][itr3->first][itr4->first] == 0)
                ? ALMOST_ZERO : (double)itr5->second / _1234_freq[itr->first][itr2->first][itr3->first][itr4->first]; 

              ratio[3] = (_234_freq[itr2->first][itr3->first][itr4->first] == 0) 
                ? ALMOST_ZERO : (double)_2345_freq[itr2->first][itr3->first][itr4->first][itr5->first] / _234_freq[itr2->first][itr3->first][itr4->first];

              ratio[2] = (_34_freq[itr3->first][itr4->first] == 0) 
                ? ALMOST_ZERO : (double)_345_freq[itr3->first][itr4->first][itr5->first] / _34_freq[itr3->first][itr4->first];

              ratio[1] = (_4_freq[itr4->first] == 0)
                ? ALMOST_ZERO : (double)_45_freq[itr4->first][itr5->first] / _4_freq[itr4->first];

              ratio[0] = (double)_5_freq[itr5->first] / N;

              prob[itr->first+itr2->first+itr3->first+itr4->first][itr5->first] 
                = coefficient[4] * ratio[4] + coefficient[3] * ratio[3] + coefficient[2] * ratio[2] + coefficient[1] * ratio[1] + coefficient[0] * ratio[0];
            } // end of for1
          } // end of for2
        } // end of for3
      } // end of for4
    } // end of for5
  }

  // 4개 짜리 확률
  {
    CCCC_FREQ::iterator itr;
    CCC_FREQ::iterator itr2;
    CC_FREQ::iterator itr3;
    C_FREQ::iterator itr4;

    for (itr = _2345_freq.begin(); itr != _2345_freq.end(); ++itr)  {
      for (itr2 = _2345_freq[itr->first].begin(); itr2 != _2345_freq[itr->first].end(); ++itr2) {
        for (itr3 = _2345_freq[itr->first][itr2->first].begin(); itr3 != _2345_freq[itr->first][itr2->first].end(); ++itr3) {
          for (itr4 = _2345_freq[itr->first][itr2->first][itr3->first].begin(); itr4 != _2345_freq[itr->first][itr2->first][itr3->first].end(); ++itr4) {

            ratio[3] = (_234_freq[itr->first][itr2->first][itr3->first] == 0) 
              ? ALMOST_ZERO : (double)itr4->second / _234_freq[itr->first][itr2->first][itr3->first];

            ratio[2] = (_34_freq[itr2->first][itr3->first] == 0) 
              ? ALMOST_ZERO : (double)_345_freq[itr2->first][itr3->first][itr4->first] / _34_freq[itr2->first][itr3->first];

            ratio[1] = (_4_freq[itr3->first] == 0)
              ? ALMOST_ZERO : (double)_45_freq[itr3->first][itr4->first] / _4_freq[itr3->first];

            ratio[0] = (double)_5_freq[itr4->first] / N;

            prob[itr->first+itr2->first+itr3->first][itr4->first] 
              = coefficient[3] * ratio[3] + coefficient[2] * ratio[2] + coefficient[1] * ratio[1] + coefficient[0] * ratio[0];

          } // end of for1
        } // end of for2
      } // end of for3
    } // end of for4
  }

  // 3개 짜리 확률
  {
    CCC_FREQ::iterator itr;
    CC_FREQ::iterator itr2;
    C_FREQ::iterator itr3;

    for (itr = _345_freq.begin(); itr != _345_freq.end(); ++itr)  {
      for (itr2 = _345_freq[itr->first].begin(); itr2 != _345_freq[itr->first].end(); ++itr2) {
        for (itr3 = _345_freq[itr->first][itr2->first].begin(); itr3 != _345_freq[itr->first][itr2->first].end(); ++itr3) {

          ratio[2] = (_34_freq[itr->first][itr2->first] == 0) 
            ? ALMOST_ZERO : (double)itr3->second / _34_freq[itr->first][itr2->first];

          ratio[1] = (_4_freq[itr2->first] == 0)
            ? ALMOST_ZERO : (double)_45_freq[itr2->first][itr3->first] / _4_freq[itr2->first];

          ratio[0] = (double)_5_freq[itr3->first] / N;

          prob[itr->first+itr2->first][itr3->first] 
            = coefficient[2] * ratio[2] + coefficient[1] * ratio[1] + coefficient[0] * ratio[0];

        } // end of for1
      } // end of for2
    } // end of for3
  }

  // 2개 짜리 확률
  {
    CC_FREQ::iterator itr;
    C_FREQ::iterator itr2;

    for (itr = _45_freq.begin(); itr != _45_freq.end(); ++itr)  {
      for (itr2 = _45_freq[itr->first].begin(); itr2 != _45_freq[itr->first].end(); ++itr2) {
 
        ratio[1] = (_4_freq[itr->first] == 0)
          ? ALMOST_ZERO : (double)itr2->second / _4_freq[itr->first];

        ratio[0] = (double)_5_freq[itr2->first] / N;

        prob[itr->first][itr2->first] 
          = coefficient[1] * ratio[1] + coefficient[0] * ratio[0];

      } // end of for1
    } // end of for2
  }

  // 1개 짜리 확률
  {
    C_FREQ::iterator itr;

    for (itr = _5_freq.begin(); itr != _5_freq.end(); ++itr) {
      ratio[0] = (double)itr->second / N;

      prob[NULLSTRING][itr->first] = coefficient[0] * ratio[0];
      ///**/fprintf(stderr, "prob(%lf) * coeff(%lf) = %lf\n", coefficient[0], ratio[0], coefficient[0] * ratio[0]);
    }
  }

  return 1;
}

/******************************************************************************/
/* Brants (2000)의 방법 */
int calc_LINEAR_INTERPOL_probability5(PROB_MAP &prob_map,
                                     CCCCC_FREQ &_12345_freq, CCCC_FREQ &_1234_freq,
                                     CCCC_FREQ &_2345_freq, CCC_FREQ &_234_freq, 
                                     CCC_FREQ &_345_freq, CC_FREQ &_34_freq, 
                                     CC_FREQ &_45_freq, C_FREQ &_4_freq, 
                                     C_FREQ &_5_freq) {

  double lambda[5];
  fprintf(stderr, "\ncalculating coefficients for linear interpolation.\n");
  
  // lambda를 계산한다.
  int N = calc_5_coefficient(lambda, _12345_freq, _1234_freq, _2345_freq, 
      _234_freq, _345_freq, _34_freq, _45_freq, _4_freq, _5_freq, _4_freq);

  // 전이 확률
  fprintf(stderr, "computing smoothed probabilities.\n");
  calc_linear_interpol_5_probs(prob_map, lambda, N, 
                               _12345_freq, _1234_freq, _2345_freq, 
                               _234_freq, _345_freq, _34_freq, _45_freq, _4_freq, _5_freq);

  return 1;
}
