#ifndef __GLOBAL_OPTION_H__
#define __GLOBAL_OPTION_H__

/******************************************************************************/
typedef struct globals_s {
  char *cmd;                       // command name
  char rsc_path[200];              // ���ҽ� ��� ���丮
  int morphological_analysis_only; // ���¼� �м��� �ϴ� ���
  int tagging_only;                // �±븸 �ϴ� ��� (���¼� �м� ����� �Է����� ����)
  int run_sentence_breaking;       // ���� �и��� �ؾ��ϴ� ��� -i r
  int column_format;               // �Է� ������ column format�� ��� = 1
  int print_sentence_breaking;     // ���� ��� ���
  int ku_style;                    // ��� ��� ���� (��� or �ο�)
  char delimiter;                  // �и��� (���¼ҿ� �±� ������)
  int processing_unit;             // �м� ���� (����, ���¼�, ����)
  int cutoff_threshold_m;          // ���¼� ���� �м�
  int cutoff_threshold_s;          // ���� ���� �м�
  int beam_size;                   // ���� ���� �м�

} globals_t;

typedef globals_t *globals_pt;

extern char *Version;
extern char *Description;
extern char *option_description[];
extern globals_pt g;

extern void usage(void);
extern char *get_basename(char *name, char *s);
extern globals_pt init_globals(globals_pt old);
extern void print_globals(globals_pt g);
extern void free_globals(globals_pt old);
extern int get_options(globals_pt g, int argc, char **argv);

#endif
