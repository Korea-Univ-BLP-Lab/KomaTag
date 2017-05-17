#ifndef _PHONETIC_CHANGE_H_
#define _PHONETIC_CHANGE_H_

#include <vector>
#include "probability_tool.h"

/****************************************************************************/
/* ���� ���� ���� */

typedef vector<string> STAGS; /* ���� �±׿�, ��) �� -> (�±�1, �±�2) */
typedef vector<STAGS> SEQ_STAGS; /* ���� �±׿��� ��, ��) �б��� -> (�±�1, �±�2), (�±�3), (�±�4, �±�5) */

typedef map<string, SEQ_STAGS> RESTORED_STAGS; /* ������ ����, �����±׿�, ��) �б���, (�±�1, �±�2), (�±�3), (�±�4, �±�5) */

typedef multimap<double, string, greater<double> > RESTORED_RESULT; /* Ȯ��, ������ ����, �����±׿� (�������� ����) */

extern int prokoma_phonetic_open(char *PHONETIC_PRB_Path, char *PHONETIC_INFO_Path, char *SYLLABLE_DIC_Path,
                                 PROB_MAP &phonetic_prob, PROB_MAP &phonetic_info, PROB_MAP &syllable_dic);

extern int phonetic_recovery(PROB_MAP &phonetic_prob, PROB_MAP &phonetic_info, PROB_MAP &syllable_dic,
                             const char *input_ej, 
                             RESTORED_RESULT &restored_ej, RESTORED_STAGS &str_syl_tag_seq,
                             int &syllable_only);

extern int print_syl_tag_seq(SEQ_STAGS &syl_tag_seq);

#endif
