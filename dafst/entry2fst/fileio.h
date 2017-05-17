/* Filename     : fileio.h */
/* Description  : external header of fileio.c */
/* Programmer   : Do-Gil Lee (Dept. of Computer Science, Korea University) */
/* Date         : 12 Dec 2002 */
#ifndef __FILEIO_H__
#define __FILEIO_H__

/******************************************************************************/
/* 입력된 화일(file_name)을 라인별로 저장 */
/* 각 라인은 EntryTable로 접근할 수 있다. */
/* 함수 내부에서 메모리를 할당하므로, 더 이상 EntryTable을 사용하지 않게 되면 */
/* free(EntryTable)을 외부에서 해 줘야 한다. */
/* 리턴값 : 라인의 수 */
/* EntryTable : 입력 라인들의 포인터 */
/******************************************************************************/
extern int file2lines(char *file_name, char ***EntryTable, char **Entry);

#endif
