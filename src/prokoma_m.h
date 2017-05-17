#ifndef PROKOMA_M_H
#define PROKOMA_M_H

extern int prokoma_m_open(char *LEXICAL_PRB_Path, char *TRANSITION_PRB_Path, 
                          PROB_MAP &lexical_prob, PROB_MAP &transition_prob);

extern int prokoma_m(PROB_MAP &transition_prob, PROB_MAP &lexical_prob, 
                     RESTORED_RESULT &restored_ej,
                     RESTORED_STAGS &str_syl_tag_seq,
                     ANALYZED_RESULT &analyzed_result_m, double cutoff_threshold, char delimiter);

#endif
