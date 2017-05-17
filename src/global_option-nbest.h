#ifndef __GLOBAL_OPTION_H__
#define __GLOBAL_OPTION_H__

/******************************************************************************/
typedef struct globals_s {
  char *cmd;                       // command name
  char rsc_path[200];              // 리소스 경로 디렉토리
  int run_sentence_breaking;       // 문장 분리를 해야하는 경우 -i r
  int print_sentence_breaking;     // 문장 경계 출력
  int ku_style;                    // 결과 출력 형식 (고대 or 민연)
  char delimiter;                  // 분리자 (형태소와 태그 사이의)

  int constraint; // N-best 태깅을 위한 제약 조건
  int relative_threshold; // 상대 임계값
  int absolute_threshold; // 절대 임계값

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
