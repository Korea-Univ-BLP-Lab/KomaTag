#ifndef HSPLIT_H
#define HSPLIT_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MJ 1024
#define MAX_WORD 500 //1024 /* �и��� �ܾ��� ���� ���� */
#define MAX_SPLIT 100 /* �и��� �ܾ��� �ִ�� */

#define FIL (char) 127 /* 127:, 128:� : ä�� �ڵ� */

/* �ϼ��� */
#ifndef isHanja /* ���� */
#define isHanja(str0, str1) ( (str0 >= 0xCA) && (str0 <= 0xFD) && (str1 >= 0xA1) && (str1 <= 0xFE) )
#endif
 
#ifndef isHangul /* �ѱ� */
#define isHangul(str0, str1) ( (str0 >= 0xB0) && (str0 <= 0xC8) && (str1 >= 0xA1) && (str1 <= 0xFE) )
#endif
 
#ifndef is2Byte /* 2byte Code */
#define is2Byte(str0, str1) ( (str0 >= 0xA1) && (str0 <= 0xAC) && (str1 >= 0xA1) && (str1 <= 0xFE) )
#endif


#define T_ENG 0 /* english */
#define T_HAN 1 /* hangul */
#define T_HJ  2 /* hanja */
#define T_DIG 3 /* digit */
#define T_SYM 4 /* symbol */
#define T_2BSYM 5 /* 2byte symbol */

typedef struct Word_type {
  char word[MAX_WORD];
  int type;
} word_type;

/* hword�� �Է¹޾� word�� type�� ���� ���� �� sword�� ���� */
/* return value : �и��� word�� �� */
extern short split_by_word_type(word_type *sword, char *hword);

/* �־��� ���ڿ��� �� ���ں��� ������. */
/* hword = �Է� ���ڿ� */
/* splitchar = ��� ���� */
/* 1 byte ���ڴ� �տ� FIL�� �ٿ��� 2byte�� �����. */
/* return value : �и��� ������ �� */
extern int split_by_char (char *hword, char splitchar[][3]);

/* �־��� ���ڿ��� �� ���ں��� ������. */
/* hword = �Է� ���ڿ� */
/* splitchar = ��� ���� */
/* return value : ���� = 1 */
extern int split_by_char_array (char *hword, char *splitchar);

/* �־��� ���ڿ�(source_str)�� 1�� ���� �迭�� �� ���ڿ�(target_str)�� ��ȯ */
/* 1 byte ���ڴ� �տ� FIL�� ���� */
extern int convert_str(int num_char, char source_str[][3], char *target_str);

/* �־��� ���ڿ�(source_str)�� 1�� ���� �迭�� �� ���ڿ�(target_str)�� ��ȯ */
/* 1 byte ���ڴ� �տ� FIL�� ���� */
extern int convert_str_origin(int num_char, char source_str[][3], char *target_str);

/* �־��� ���ڿ�(source_str)�� ���ڿ�(target_str)�� ��ȯ */
/* 1 byte ���ڴ� �տ� FIL�� ���� */
extern int convert_str_origin_array(char *source_str, char *target_str);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
