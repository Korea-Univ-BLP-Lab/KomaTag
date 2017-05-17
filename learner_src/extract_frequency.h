#ifndef __EXTRACT_FREQUENCY_H__
#define __EXTRACT_FREQUENCY_H__

#include "probability_tool.h"

extern int extract_frequency_bigram_ej_by_ej(FILE *fp, 
                      C_FREQ &inter_tag_unigram_freq, 
                      CC_FREQ &inter_tag_bigram_freq,
                      char delimiter);

extern int extract_frequency_bigram_ej(FILE *fp, 
                      C_FREQ &inter_tag_unigram_freq, 
                      CC_FREQ &inter_tag_bigram_freq,
                      char delimiter);

extern int extract_frequency_bigram(FILE *fp, 
                                    C_FREQ &tag_unigram_freq, 
                                    CC_FREQ &tag_bigram_freq,
                                    C_FREQ &inter_tag_unigram_freq, 
                                    CC_FREQ &inter_tag_bigram_freq,
                                    char delimiter);

extern  int extract_frequency_trigram(FILE *fp,
                              C_FREQ &tag_bigram_freq,
                              CC_FREQ &tag_trigram_freq,
                              C_FREQ &inter_tag_bigram_freq,
                              CC_FREQ &inter_tag_trigram_freq,
                              char delimiter);
                                                                                                              

extern int extract_frequency_s(FILE *fp, C_FREQ &u_freq, C_FREQ &c_freq, 
                      CC_FREQ &uc_freq, CC_FREQ &cu_freq, 
                      CCC_FREQ &ucu_freq, CCC_FREQ &cuc_freq,
                      CCCC_FREQ &ucuc_freq, CCCC_FREQ &cucu_freq, 
                      CCCCC_FREQ &ucucu_freq, CCCCC_FREQ &cucuc_freq,
                      C_FREQ &c_type_freq, CC_FREQ &c_type_u_freq);
#endif
