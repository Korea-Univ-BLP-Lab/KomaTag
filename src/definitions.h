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
// ǰ�� ���� �� ����(����ؾ� ��)
#define HMM_TAGGING
//#define MAX_ENT_TAGGING
//#define HMM_TAGGING_EJ

// HMM_TAGGING�� ����ϴ� ��쿡�� �ش��
// bigram, trigram �� ���� (����ؾ� ��)
//#define BIGRAM_TAGGING
#define TRIGRAM_TAGGING

// �и� ��� ���� (��� ��쿡 �ش��)
#define USING_DENOMINATOR

// HMM_TAGGING_EJ�� ����ϴ� ��쿡�� �ش��
// �Ʒ��� ��۵Ǿ�� ��
//#define MODEL4
//#define MODEL5
//#define MODEL6
#define MODEL7

///////////////////////////////////////////////////////////////////////////////
#ifdef MODEL4
#define USING_ALL_TAG // ��� �±׸� ���
#endif

#ifdef MODEL5
#define USING_ALL_TAG // ��� �±׸� ���
#define USING_FIRST_TAG // ���� �м� ��� // ù �±׸� ���
#endif

#ifdef MODEL6
#define USING_HEAD_TAIL_TAG // �Ӹ��� ���� �±׸� ���
#endif

#ifdef MODEL7
#define USING_HEAD_TAIL_TAG // �Ӹ��� ���� �±׸� ���
#define USING_FIRST_TAG // ���� �м� ��� // ù �±׸� ���
#endif

///////////////////////////////////////////////////////////////////////////////

/* �м� ���� ������ ���� */
// E, M, S, EM, ES, MS, EMS (7���� ����)
#define EOJEOL_ANALYSIS    1
#define MORPHEME_ANALYSIS  2
#define SYLLABLE_ANALYSIS  4

#define ABSOLUTE_MORPH_CONSTRAINT 1
#define CONSISTENCY_CONSTRAINT    2
#define RELATIVE_MORPH_CONSTRAINT 4

/******************************************************************************/
/* Ÿ�� ���� */

typedef map<string, int>             WORD_FREQ; /* �ܾ� �� */

typedef map<string, double>  BI_STATE_MAP; /* �±�, Ȯ�� */

typedef map<int, BI_STATE_MAP> SUB_STRING_INFO; /* (�κй��ڿ�) ��ȣ, �±� (1,...,n), Ȯ�� (1,...,n) */

typedef multimap<double, string, greater<double> >  ANALYZED_RESULT_MAP; /* Ȯ��, �м� ��� (�������� ����) */

typedef pair<double, string>  ONE_RESULT; /* Ȯ��, �м� ��� (�������� ����) */
typedef vector<ONE_RESULT> ANALYZED_RESULT;

typedef vector<string> SUB_STRING;

typedef multimap<double, vector<string>, greater<double> > RESULT_S_MAP; /* bfs.cpp���� ��� */

/******************************************************************************/
/* ��� ���� */
#define DELIM "_|"              /* �и��� */

#define BOW_TAG_2 "2|"         /* �������� �±� -2 */
#define BOW_TAG_1 "1|"         /* �������� �±� -1 */

#define BOW_SYL_2 "|2"  /* �������� ���� -2 */
#define BOW_SYL_1 "|1"  /* �������� ���� -1 */

#define EOW_TAG "$>"   /* ������ �±� */
#define EOW_SYL "<$"  /* ������ ���� */

#define BOSTAG_1 ">$-1"  /* ������� �±� */
#define BOSTAG_2 ">$-2"

/* �м� ��� ���� */
#define EOJEOL 1
#define MORPHEME 2
#define SYLLABLE 4
#define NORESULT 0

//#define MAX_LINE 1000 /* ������ �ִ� �ܾ�(��ū) �� */
//#define MAX_WORD_LEN 200 /* �ܾ��� �ִ� ���� (���� ��) */
#define MAX_TAG_LEN 50

/******************************************************************************/

//extern int verbosity;

#endif
