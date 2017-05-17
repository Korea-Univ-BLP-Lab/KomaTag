#ifndef PROKOMA_S_H
#define PROKOMA_S_H

extern int prokoma_s_open(char *TAG_S_FST_Path, char *TAG_S_hash_Path, char *TAG_S_PROB_Path, 
                   char *SYLLABLE_S_FST_Path, char *SYLLABLE_S_hash_Path, char *SYLLABLE_S_PROB_Path,
                   char *S_TRANSITION_FST_Path,
                   void **tag_s_fst, double **tag_s_prob,
                   void **syllable_s_fst, double **syllable_s_prob,
                   void **s_transition_fst);

extern void prokoma_s_close(void *tag_s_fst, double *tag_s_prob, void *syllable_s_fst, double *syllable_s_prob);

extern int prokoma_s(void *tag_s_fst, double *tag_s_prob,
              void *syllable_s_fst, double *syllable_s_prob,
              void *s_transition_fst,
              RESTORED_RESULT &restored_ej, /* 음운 복원된 어절 */
              RESTORED_STAGS &str_syl_tag_seq,
              ANALYZED_RESULT &analyzed_result_s, 
              double cutoff_threshold, int beam_size,
              char delimiter);

#endif
