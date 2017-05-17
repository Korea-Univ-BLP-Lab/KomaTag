#ifndef _PROKOMA_E_H_
#define _PROKOMA_E_H_

extern int prokoma_e_open (char *RMEJ_FST_Path, char *RMEJ_hash_Path, char *RMEJ_FST_INFO_Path, char *RMEJ_FST_FREQ_Path, 
                           void **rmej_fst, char ***rmej_info, int **rmej_freq);

extern void prokoma_e_close(void *rmej_fst, char **rmej_info, int *rmej_freq);

extern int prokoma_e(void *fst, char **rmej_info, int *rmej_freq, 
                     const char *ej, ANALYZED_RESULT &result);

#endif
