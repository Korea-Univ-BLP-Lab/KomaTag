#ifndef __get_morphs_tag_H__
#define __get_morphs_tag_H__

extern int get_morphs_tags(char *str, int *morph_num, 
                           char morphs[][MAX_WORD], char tags[][MAX_WORD], 
                           int spacing_tags[MAX_WORD],
                           char delimiter);

#endif
