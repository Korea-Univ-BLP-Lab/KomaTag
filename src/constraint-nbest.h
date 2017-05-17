#ifndef _CONSTRAINT_NBEST_H_
#define _CONSTRAINT_NBEST_H_

extern void print_nbest_tagging_result(FILE *fp, vector<string> ejs, 
                          vector<ANALYZED_RESULT> &morph_analyzed_result, 
                          int num_word, int *tagging_result,
                          char delimiter, int output_style, int constraint,
                          int relative_threshold, int absolute_threshold);

extern int print_nbest_tagging_result2(FILE *fp, vector<string> ejs, 
                          vector<ANALYZED_RESULT> &morph_analyzed_result, 
                          int num_word, int *tagging_result,
                          char delimiter, int output_style, int constraint,
                          int relative_threshold, int absolute_threshold);

#endif
