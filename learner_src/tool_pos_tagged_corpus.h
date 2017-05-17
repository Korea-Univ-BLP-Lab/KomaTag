#ifndef __tool_pos_tagged_corpus_H__
#define __tool_pos_tagged_corpus_H__

/* 출력 모드 */
#define BI    1 /* 시작, 중간 */
#define BIS   2 /* 시작, 중간, 단독 */
#define IE    3 /* 시작, 끝 */    
#define IES   4 /* 시작, 끝, 단독 */

extern int get_morphs_tags(char *str, char *raw_ej, int *morph_num, 
                           char morphs[][MAX_WORD], char tags[][MAX_WORD], 
                           int spacing_tags[MAX_WORD], char delimiter);

extern int get_morphs_tags(char *str, int *morph_num, 
                    char morphs[][MAX_WORD], char tags[][MAX_WORD], int spacing_tags[MAX_WORD],
                    char delimiter);

extern void get_syllable_tagging_result(int print_mode, char lexical_ej[][3], int lexical_ej_len, 
                                        char *tags, char syllable_tag[][30]);

#endif
