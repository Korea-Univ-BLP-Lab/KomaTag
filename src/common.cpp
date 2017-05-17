#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*****************************************************************************/
int print_progress (int percent, long filesize, FILE * fp) {
  long curpos;
  double newpercent;
  curpos = ftell (fp);
  newpercent = (double) curpos / filesize * 100;
  if ((int) newpercent > percent)
    fprintf (stderr, "\r%3d%% done..", (int) newpercent);
  return (int) newpercent;
}
