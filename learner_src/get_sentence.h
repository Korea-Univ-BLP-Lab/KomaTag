#ifndef __GET_SENTENCE_H__
#define __GET_SENTENCE_H__

extern int get_sentence(FILE *fp, char words[][MAX_WORD_LEN], char results[][MAX_RESULT_LEN]);
extern int read_sentence (FILE *fp, char sentence[][MAX_WORD_LEN]);
extern int get_sentence_by_syllable(FILE *fp, char syllables[][MAX_WORD_LEN], char tags[][MAX_TAG_LEN]);
#endif
