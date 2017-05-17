#ifndef UNIT_CONVERSION_H
#define UNIT_CONVERSION_H

extern int syllable2morpheme(char *result, int num_syl, char ej[][3], 
                             char tag_sequence[][MAX_TAG_LEN], char delimiter);
extern int check_tag_sequence(char tag_sequence[][MAX_TAG_LEN], SEQ_STAGS &syl_tag_seq);
extern int check_morpheme_result(char *str, RESTORED_STAGS &str_syl_tag_seq, char delimiter);
#endif
