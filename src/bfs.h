#ifndef _BFS_H_
#define _BFS_H_

#include "FST.h"

extern void trigram_breath_first_search(void *tag_s_fst, double *tag_s_prob,
                            void *syllable_s_fst, double *syllable_s_prob,
                            void *s_transition_fst,
                            SEQ_STAGS &syl_tag_seq,
                            char splitchar[][3], int total_time, 
                            double restored_prob, 
                            double cutoff_threshold, int beam_size,
                            RESULT_S_MAP results[]);
#endif
