// DATE : 2004. 11. 6

#ifndef HSPLIT_H
#define HSPLIT_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MJ 1024
#define MAX_WORD 1024 /* ºÐ¸®ÇÒ ´Ü¾îÀÇ ÃÖÀå ±æÀÌ */
#define MAX_SPLIT 100 /* ºÐ¸®µÈ ´Ü¾îÀÇ ÃÖ´ë¼ö */

#define FIL (char) 127 /* 127:, 128:€ : Ã¤¿ò ÄÚµå */
#define FIL2 (char) 128 /* 127:, 128:€ : Ã¤¿ò ÄÚµå */

/* ¿Ï¼ºÇü */
#ifndef isHanja /* ÇÑÀÚ */
#define isHanja(str0, str1) ( (str0 >= 0xCA) && (str0 <= 0xFD) && (str1 >= 0xA1) && (str1 <= 0xFE) )
#endif
 
#ifndef isHangul /* ÇÑ±Û */
#define isHangul(str0, str1) ( (str0 >= 0xB0) && (str0 <= 0xC8) && (str1 >= 0xA1) && (str1 <= 0xFE) )
#endif
 
#ifndef is2Byte /* 2byte Code */
#define is2Byte(str0, str1) ( (str0 >= 0xA1) && (str0 <= 0xAC) && (str1 >= 0xA1) && (str1 <= 0xFE) )
#endif


#define T_ENG_UPPER 0 /* english upper case */
#define T_ENG_LOWER 1 /* english lower case */
#define T_HAN       2 /* hangul */
#define T_HJ        3 /* hanja */
#define T_DIG       4 /* digit */
#define T_SYM       5 /* symbol */
#define T_2BSYM     6 /* 2byte symbol */
#define T_FIL      7 /* FIL */

typedef struct Word_type {
  char word[MAX_WORD];
  int type;
} word_type;

/* hword¸¦ ÀÔ·Â¹Þ¾Æ wordÀÇ type¿¡ µû¶ó ³ª´« ÈÄ sword¿¡ ÀúÀå */
/* return value : ºÐ¸®µÈ wordÀÇ ¼ö */
extern short split_by_word_type(word_type *sword, char *hword);

/* ÁÖ¾îÁø ¹®ÀÚ¿­À» °¢ ¹®ÀÚº°·Î ³ª´«´Ù. */
/* hword = ÀÔ·Â ¹®ÀÚ¿­ */
/* splitchar = °á°ú ÀúÀå */
/* 1 byte ¹®ÀÚ´Â ¾Õ¿¡ FILÀ» ºÙ¿©¼­ 2byte·Î ¸¸µç´Ù. */
/* return value : ºÐ¸®µÈ ¹®ÀÚÀÇ ¼ö */
extern int split_by_char (char *hword, char splitchar[][3]);

/* ÁÖ¾îÁø ¹®ÀÚ¿­À» °¢ ¹®ÀÚº°·Î ³ª´«´Ù. */
/* hword = ÀÔ·Â ¹®ÀÚ¿­ */
/* splitchar = °á°ú ÀúÀå */
/* 1 byte ¹®ÀÚ´Â ¾Õ¿¡ FILÀ» Á¦°Å */
/* return value : ºÐ¸®µÈ ¹®ÀÚÀÇ ¼ö */
extern int split_by_char_origin (char *hword, char splitchar[][3]);

/* ÁÖ¾îÁø ¹®ÀÚ¿­À» °¢ ¹®ÀÚº°·Î ³ª´«´Ù. */
/* hword = ÀÔ·Â ¹®ÀÚ¿­ */
/* splitchar = °á°ú ÀúÀå */
/* return value : ¼º°ø = 1 */
extern int split_by_char_array (char *hword, char *splitchar);

/* ÁÖ¾îÁø ¹®ÀÚ¿­(source_str)À» 1Â÷ ¹®ÀÚ ¹è¿­·Î µÈ ¹®ÀÚ¿­(target_str)·Î º¯È¯ */
/* 1 byte ¹®ÀÚ´Â ¾Õ¿¡ FILÀ» À¯Áö */
extern int convert_str(int num_char, char source_str[][3], char *target_str);

/* ÁÖ¾îÁø ¹®ÀÚ¿­(source_str)À» 1Â÷ ¹®ÀÚ ¹è¿­·Î µÈ ¹®ÀÚ¿­(target_str)·Î º¯È¯ */
/* 1 byte ¹®ÀÚ´Â ¾Õ¿¡ FILÀ» Á¦°Å */
extern int convert_str_origin(int num_char, char source_str[][3], char *target_str);

/* ÁÖ¾îÁø ¹®ÀÚ¿­(source_str)À» ¹®ÀÚ¿­(target_str)·Î º¯È¯ */
/* 1 byte ¹®ÀÚ´Â ¾Õ¿¡ FILÀ» Á¦°Å */
extern int convert_str_origin_array(char *source_str, char *target_str);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
