#ifndef _PHONETIC_CHANGE_H_
#define _PHONETIC_CHANGE_H_

#include <vector>
#include "probability_tool.h"

/****************************************************************************/
/* 음운 현상 복원 */

typedef vector<string> STAGS; /* 음절 태그열, 예) 학 -> (태그1, 태그2) */
typedef vector<STAGS> SEQ_STAGS; /* 음절 태그열의 열, 예) 학교에 -> (태그1, 태그2), (태그3), (태그4, 태그5) */

typedef map<string, SEQ_STAGS> RESTORED_STAGS; /* 복원된 어절, 음절태그열, 예) 학교에, (태그1, 태그2), (태그3), (태그4, 태그5) */

typedef multimap<double, string, greater<double> > RESTORED_RESULT; /* 확률, 복원된 어절, 음절태그열 (내림차순 정렬) */

extern int prokoma_phonetic_open(char *PHONETIC_PRB_Path, char *PHONETIC_INFO_Path, char *SYLLABLE_DIC_Path,
                                 PROB_MAP &phonetic_prob, PROB_MAP &phonetic_info, PROB_MAP &syllable_dic);

extern int phonetic_recovery(PROB_MAP &phonetic_prob, PROB_MAP &phonetic_info, PROB_MAP &syllable_dic,
                             const char *input_ej, 
                             RESTORED_RESULT &restored_ej, RESTORED_STAGS &str_syl_tag_seq,
                             int &syllable_only);

extern int print_syl_tag_seq(SEQ_STAGS &syl_tag_seq);

#endif
