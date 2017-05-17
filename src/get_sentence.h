#ifndef __GET_SENTENCE_H__
#define __GET_SENTENCE_H__

extern int get_sentence_with_sbd(void *sb_fst, FILE *fp, vector<string> &word);

extern int get_sentence_from_row_format(FILE *fp, vector<string> &word);

extern int get_sentence_from_column_format(FILE *fp, vector<string> &word);

extern int get_sentence_from_morphological_analyzed_text(FILE *fp, vector<string> &word, 
                                                  vector<ANALYZED_RESULT> &morph_analyzed_result);

extern int get_sentence_from_morphological_analyzed_text_with_sbd(void *sb_fst, FILE *fp, vector<string> &word, 
                                                  vector<ANALYZED_RESULT> &morph_analyzed_result);

extern int sbd_open(void **sb_fst, char *filename);

extern void sbd_close(void *sb_fst);

#endif
