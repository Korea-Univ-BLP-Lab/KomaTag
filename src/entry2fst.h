#ifndef __entry2fst_H__
#define __entry2fst_H__


/******************************************************************************/
/* 타입 선언 */



extern int entry2fst(char *filename, char *list_filename, char *fst_filename, char *hash_filename,
                     char *info_filename, char *freq_filename, 
                     int cutoff_threshold, int cutoff_threshold2);

extern int fst_open(char *fst_Path, char *hash_Path, char *fst_INFO_Path, char *fst_FREQ_Path,
                   void **fst_fst, char ***fst_info, int **fst_freq);
                                      
extern void fst_close(void *fst_fst, char **fst_info, int *fst_freq);

#endif
