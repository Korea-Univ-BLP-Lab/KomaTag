#ifndef __definitions_H__
#define __definitions_H__

#pragma warning(disable: 4786)
#pragma warning(disable: 4503)

#include <map>
#include <stdio.h>
#include <string> 
#include <stdlib.h>

using namespace std;

//#define MODEL4
//#define MODEL5
//#define MODEL6
//#define MODEL7

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

// bigra1m, trigram 모델 선택 (토글해야 함)
#define TRIGRAM_TAGGING
//#define BIGRAM_TAGGING

/******************************************************************************/
/* 타입 선언 */

typedef map<string, int>             SURFACE_FREQ;         /* 표층문자열 빈도 */
typedef map<string, int>             LEXICAL_FREQ;         /* 어휘층문자열 빈도 */
typedef map<string, LEXICAL_FREQ>    SURFACE_LEXCIAL_FREQ; /* 표층문자열 어휘층문자열 빈도 */

typedef map <string, int>             TAGSET; /* 태그 빈도 */

typedef map<string, double>  BI_STATE_MAP; /* 태그, 확률 */
typedef multimap<double, string, greater<double> >  ANALYZED_RESULT_MAP; /* 확률, 분석 결과 (내림차순 정렬) */


typedef map<string, int>             WORD_FREQ; /* 단어 빈도 */
/******************************************************************************/
/* 상수 선언 */
#define DELIM "_|"       /* 분리자 */

#define BOW_TAG_2 "2|"   /* 어절시작 태그 -2 */
#define BOW_TAG_1 "1|"   /* 어절시작 태그 -1 */

#define BOW_SYL_2 "|2"  /* 어절시작 음절 -2 */
#define BOW_SYL_1 "|1"  /* 어절시작 음절 -1 */

#define EOW_TAG "$>"   /* 어절끝 태그 */
#define EOW_SYL "<$"  /* 어절끝 음절 */

#define BOSTAG_2 ">$-2"  /* 문장시작 태그 */
#define BOSTAG_1 ">$-1"  /* 문장시작 태그 */


#define MAX_LINE 1000 /* 문장의 최대 단어(토큰) 수 */
#define MAX_WORD_LEN 200 /* 단어의 최대 길이 (문자 수) */
#define MAX_TAG_LEN 50  // 태그의 최대 길이
#define MAX_RESULT_LEN 1000 // 형태소 분석 결과의 최대 길이

/******************************************************************************/
extern int verbosity;

#endif
