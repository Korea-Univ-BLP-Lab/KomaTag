#ifndef __ENV_H__
#define __ENV_H__

extern char rsc_file_with_full_path[40][100];

extern short komatag_CheckEnv (char *RSC_Path);

// env.cpp에 정의된 rsc_file_names와 연결됨
#define RMEJ_FST           0
#define RMEJ_HASH          1
#define RMEJ_INFO          2
#define RMEJ_FREQ          3
#define PHONETIC_PRB       4
#define PHONETIC_INFO      5
#define LEXICAL_PRB        6
#define TRANSITION_PRB     7
#define TAG_S_FST          8
#define TAG_S_HASH         9
#define TAG_S_PROB         10
#define SYLLABLE_S_FST     11
#define SYLLABLE_S_HASH    12
#define SYLLABLE_S_PROB    13
#define SYLLABLE_DIC       14
#define SB_FST             15
#define INTER_TRANS_PRB    16
#define INTRA_TRANS_PRB    17
#define FULLMORPHEME_DEF   18
#define HEAD               19
#define DUMMY_HEAD         20
#define TAIL               21
#define INTER_TRANS_EJ_PRB 22
#define S_TRANSITION_FST   23
#endif
