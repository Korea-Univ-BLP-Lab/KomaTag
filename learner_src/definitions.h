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

// bigra1m, trigram �� ���� (����ؾ� ��)
#define TRIGRAM_TAGGING
//#define BIGRAM_TAGGING

/******************************************************************************/
/* Ÿ�� ���� */

typedef map<string, int>             SURFACE_FREQ;         /* ǥ�����ڿ� �� */
typedef map<string, int>             LEXICAL_FREQ;         /* ���������ڿ� �� */
typedef map<string, LEXICAL_FREQ>    SURFACE_LEXCIAL_FREQ; /* ǥ�����ڿ� ���������ڿ� �� */

typedef map <string, int>             TAGSET; /* �±� �� */

typedef map<string, double>  BI_STATE_MAP; /* �±�, Ȯ�� */
typedef multimap<double, string, greater<double> >  ANALYZED_RESULT_MAP; /* Ȯ��, �м� ��� (�������� ����) */


typedef map<string, int>             WORD_FREQ; /* �ܾ� �� */
/******************************************************************************/
/* ��� ���� */
#define DELIM "_|"       /* �и��� */

#define BOW_TAG_2 "2|"   /* �������� �±� -2 */
#define BOW_TAG_1 "1|"   /* �������� �±� -1 */

#define BOW_SYL_2 "|2"  /* �������� ���� -2 */
#define BOW_SYL_1 "|1"  /* �������� ���� -1 */

#define EOW_TAG "$>"   /* ������ �±� */
#define EOW_SYL "<$"  /* ������ ���� */

#define BOSTAG_2 ">$-2"  /* ������� �±� */
#define BOSTAG_1 ">$-1"  /* ������� �±� */


#define MAX_LINE 1000 /* ������ �ִ� �ܾ�(��ū) �� */
#define MAX_WORD_LEN 200 /* �ܾ��� �ִ� ���� (���� ��) */
#define MAX_TAG_LEN 50  // �±��� �ִ� ����
#define MAX_RESULT_LEN 1000 // ���¼� �м� ����� �ִ� ����

/******************************************************************************/
extern int verbosity;

#endif
