#ifndef __definitions_H__
#define __definitions_H__

#pragma warning(disable: 4786)
#pragma warning(disable: 4503)

#include <map>
#include <vector>
#include <string> 
#include <stdio.h>
#include <stdlib.h>

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// 품사 부착 모델 종류(토글해야 함)
#define HMM_TAGGING
//#define MAX_ENT_TAGGING
//#define HMM_TAGGING_EJ

// HMM_TAGGING을 사용하는 경우에만 해당됨
// bigram, trigram 모델 선택 (토글해야 함)
//#define BIGRAM_TAGGING
#define TRIGRAM_TAGGING

// 분모 사용 유무 (모든 경우에 해당됨)
#define USING_DENOMINATOR

// HMM_TAGGING_EJ을 사용하는 경우에만 해당됨
// 아래는 토글되어야 함
//#define MODEL4
//#define MODEL5
//#define MODEL6
#define MODEL7

///////////////////////////////////////////////////////////////////////////////
#ifdef MODEL4
#define USING_ALL_TAG // 모든 태그를 사용
#endif

#ifdef MODEL5
#define USING_ALL_TAG // 모든 태그를 사용
#define USING_FIRST_TAG // 현재 분석 결과 // 첫 태그를 사용
#endif

#ifdef MODEL6
#define USING_HEAD_TAIL_TAG // 머리와 꼬리 태그를 사용
#endif

#ifdef MODEL7
#define USING_HEAD_TAIL_TAG // 머리와 꼬리 태그를 사용
#define USING_FIRST_TAG // 현재 분석 결과 // 첫 태그를 사용
#endif

///////////////////////////////////////////////////////////////////////////////

/* 분석 단위 결정을 위한 */
// E, M, S, EM, ES, MS, EMS (7가지 조합)
#define EOJEOL_ANALYSIS    1
#define MORPHEME_ANALYSIS  2
#define SYLLABLE_ANALYSIS  4

#define ABSOLUTE_MORPH_CONSTRAINT 1
#define CONSISTENCY_CONSTRAINT    2
#define RELATIVE_MORPH_CONSTRAINT 4

/******************************************************************************/
/* 타입 선언 */

typedef map<string, int>             WORD_FREQ; /* 단어 빈도 */

typedef map<string, double>  BI_STATE_MAP; /* 태그, 확률 */

typedef map<int, BI_STATE_MAP> SUB_STRING_INFO; /* (부분문자열) 번호, 태그 (1,...,n), 확률 (1,...,n) */

typedef multimap<double, string, greater<double> >  ANALYZED_RESULT_MAP; /* 확률, 분석 결과 (내림차순 정렬) */

typedef pair<double, string>  ONE_RESULT; /* 확률, 분석 결과 (내림차순 정렬) */
typedef vector<ONE_RESULT> ANALYZED_RESULT;

typedef vector<string> SUB_STRING;

typedef multimap<double, vector<string>, greater<double> > RESULT_S_MAP; /* bfs.cpp에서 사용 */

/******************************************************************************/
/* 상수 선언 */
#define DELIM "_|"              /* 분리자 */

#define BOW_TAG_2 "2|"         /* 어절시작 태그 -2 */
#define BOW_TAG_1 "1|"         /* 어절시작 태그 -1 */

#define BOW_SYL_2 "|2"  /* 어절시작 음절 -2 */
#define BOW_SYL_1 "|1"  /* 어절시작 음절 -1 */

#define EOW_TAG "$>"   /* 어절끝 태그 */
#define EOW_SYL "<$"  /* 어절끝 음절 */

#define BOSTAG_1 ">$-1"  /* 문장시작 태그 */
#define BOSTAG_2 ">$-2"

/* 분석 결과 단위 */
#define EOJEOL 1
#define MORPHEME 2
#define SYLLABLE 4
#define NORESULT 0

//#define MAX_LINE 1000 /* 문장의 최대 단어(토큰) 수 */
//#define MAX_WORD_LEN 200 /* 단어의 최대 길이 (문자 수) */
#define MAX_TAG_LEN 50

/******************************************************************************/

//extern int verbosity;

#endif
