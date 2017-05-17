#ifndef UTIL_H
#define UTIL_H


/* ------------------------------------------------------------ */
/* 0 errors only, 1 normal, ..., 6 really chatty */
extern int verbosity;

/* ------------------------------------------------------------ */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   returns number of CPU milliseconds used
*/
//extern unsigned long used_ms(void);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   print information to STDERR and exit
*/
extern void error(char *format, ...);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   selectively print information to STDERR
   - mode: print only if abs(mode)<=verbosity
   - format: format string
   - ...: arguments
*/
extern void report(int mode, char *format, ...);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   print small spinning wheel
   - mode: print only if abs(mode)<=verbosity
*/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   compares to ints, for qsort()
*/
extern int intcompare(const void *ip, const void *jp);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   tries to open file, exists on error
   - name: filename
   - mode: open mode
*/

#endif
