#ifndef __GLOBAL_OPTION_H__
#define __GLOBAL_OPTION_H__

/******************************************************************************/
typedef struct globals_s {
  char *cmd;    /* command name */   
  char delimiter;
} globals_t;

typedef globals_t *globals_pt;

extern char *Version;
extern char *Description;
extern char *option_description[];
extern globals_pt g;

extern void usage(void);
extern char *get_basename(char *name, char *s);
extern globals_pt init_globals(globals_pt old);
extern void free_globals(globals_pt old);
extern int get_options(globals_pt g, int argc, char **argv);
extern int print_progress (int percent, long filesize, FILE *fp);

#endif
