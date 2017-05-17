#ifndef __prokoma_learn_H__
#define __prokoma_learn_H__

extern int prokoma_learn_e (int freq_boundary, char *input_filename);

extern int morpheme_tagging(char *infile, char *outfile, char delimiter);

extern int prokoma_learn_m (char *filename);

extern int syllable_tagging(char *infile, char *outfile, char delimiter);

extern int phonetic_info(char *infile, char *outfile1, char *outfile2, char delimiter);

extern int prokoma_learn_s (char *filename);

#endif
