#ifndef _PROKOMA_H_
#define _PROKOMA_H_

#include "probability_tool.h"

extern int prokoma(const char *input_ej, 
            void *rmej_fst, int *rmej_freq, char **rmej_info, 
            PROB_MAP &phonetic_prob, PROB_MAP &phonetic_info, PROB_MAP &syllable_dic,
            PROB_MAP &transition_prob, PROB_MAP &lexical_prob, 
            void *tag_s_fst, double *tag_s_prob,
            void *syllable_s_fst, double *syllable_s_prob,
            void *s_transition_fst, 
            double cutoff_threshold_m, double cutoff_threshold_s, int beam_size,
            ANALYZED_RESULT &analyzed_result, char delimiter, int processing_unit);

extern int print_analyzed_result(FILE *fp, const char *input_ej, ANALYZED_RESULT &analyzed_result, 
                                 char delimiter, int output_style);

extern int prokoma_open(void **rmej_fst, char ***rmej_info, int **rmej_freq, 
                 PROB_MAP &phonetic_prob, PROB_MAP &phonetic_info, PROB_MAP &syllable_dic,
                 PROB_MAP &transition_prob, PROB_MAP &lexical_prob,
                 void **tag_s_fst, double **tag_s_prob,
                 void **syllable_s_fst, double **syllable_s_prob,
                 void **s_transition_fst,
                 int processing_unit);

extern int prokoma_close(void *rmej_fst, char **rmej_info, int *rmej_freq, 
                  void *tag_s_fst, double *tag_s_prob, void *syllable_s_fst, double *syllable_s_prob,
                  void *s_transition_fst,
                  int processing_unit);
#endif
