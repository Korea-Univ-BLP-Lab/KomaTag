#ifndef __ME_VITERBI_H__
#define __ME_VITERBI_H__

#include "maxentmodel.hpp"
using namespace maxent;

extern void me_viterbi_search (MaxentModel &head_m, MaxentModel &tail_m, 
                               MaxentModel &dummy_head_m, 
                               WORD_FREQ &full_morpheme_map,
                               vector<ANALYZED_RESULT> &morph_analyzed_result, int total_time,
                               int *state_sequence, char delimiter);

#endif
