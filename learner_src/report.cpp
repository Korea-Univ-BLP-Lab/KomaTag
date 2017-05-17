#include <stdlib.h>
#include <stdio.h>
#include <string.h> /* Linux: strpbrk */
#include <stdarg.h>
#include <errno.h>

int verbosity = 2; /* default */ /* 0 errors only, 1 normal, ..., 6 really chatty */
/******************************************************************************/
/*unsigned long used_ms() {
  struct rusage ru;

  if (getrusage(RUSAGE_SELF, &ru)) { report(0, "Can't get rusage: %s\n", strerror(errno)); }

  return 
    (ru.ru_utime.tv_sec+ru.ru_stime.tv_sec)*1000+
    (ru.ru_utime.tv_usec+ru.ru_stime.tv_usec)/1000;
}*/

/******************************************************************************/
void error(char *format, ...) {
  va_list ap;

//  fprintf(stderr, "[%9ld ms::%d] ", used_ms(), verbosity); 
  fprintf(stderr, "ERROR: ");
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
  //exit(0);
}

/******************************************************************************/
/* verbosity가 주어진 mode보다 크거나 같을 때만 출력 */
void report(int mode, char *format, ...) {
  va_list ap;

  if (mode > verbosity) { return; }

//  if (mode>=0) { fprintf(stderr, "[%9ld ms::%d] ", used_ms(), verbosity); } 
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
}

/******************************************************************************/
int intcompare(const void *ip, const void *jp) {
  int i = *((int *)ip);
  int j = *((int *)jp);

  if (i > j) { return 1; }
  if (i < j) { return -1; }
  return 0;
}


/******************************************************************************/
