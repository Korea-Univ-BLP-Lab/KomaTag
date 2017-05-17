#ifndef _NEOHANTAG_H_
#define _NEOHANTAG_H_

extern int neohantag();

extern int neohantag_open(char *inter_transition_Path, char *intra_transition_Path,
                   PROB_MAP &inter_transition_prob, PROB_MAP &intra_transition_prob);

extern void print_tagging_result(FILE *fp, vector<string> ejs, vector<ANALYZED_RESULT> &morph_analyzed_result, 
                                 int num_word, int *result,
                                 char delimiter, int output_style);

extern void bigram_viterbi_search (PROB_MAP &intra_transition_prob, PROB_MAP &inter_transition_prob, 
                                   vector<ANALYZED_RESULT> &morph_analyzed_result, int total_time,
                                   int *state_sequence, char delimiter);

extern void bigram_viterbi_search_ej (PROB_MAP &inter_transition_prob, 
                                      vector<ANALYZED_RESULT> &morph_analyzed_result, int total_time,
                                      int *state_sequence, char delimiter);
#endif
