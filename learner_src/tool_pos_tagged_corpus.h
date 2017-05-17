#ifndef __tool_pos_tagged_corpus_H__
#define __tool_pos_tagged_corpus_H__

/* ��� ��� */
#define BI    1 /* ����, �߰� */
#define BIS   2 /* ����, �߰�, �ܵ� */
#define IE    3 /* ����, �� */    
#define IES   4 /* ����, ��, �ܵ� */

extern int get_morphs_tags(char *str, char *raw_ej, int *morph_num, 
                           char morphs[][MAX_WORD], char tags[][MAX_WORD], 
                           int spacing_tags[MAX_WORD], char delimiter);

extern int get_morphs_tags(char *str, int *morph_num, 
                    char morphs[][MAX_WORD], char tags[][MAX_WORD], int spacing_tags[MAX_WORD],
                    char delimiter);

extern void get_syllable_tagging_result(int print_mode, char lexical_ej[][3], int lexical_ej_len, 
                                        char *tags, char syllable_tag[][30]);

#endif
