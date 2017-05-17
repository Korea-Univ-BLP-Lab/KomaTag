#ifndef __GLOBAL_OPTION_H__
#define __GLOBAL_OPTION_H__

/******************************************************************************/
typedef struct globals_s {
  char *cmd;                       // command name
  char rsc_path[200];              // 리소스 경로 디렉토리
  int morphological_analysis_only; // 형태소 분석만 하는 경우
  int tagging_only;                // 태깅만 하는 경우 (형태소 분석 결과를 입력으로 받음)
  int run_sentence_breaking;       // 문장 분리를 해야하는 경우 -i r
  int column_format;               // 입력 파일이 column format인 경우 = 1
  int print_sentence_breaking;     // 문장 경계 출력
  int ku_style;                    // 결과 출력 형식 (고대 or 민연)
  char delimiter;                  // 분리자 (형태소와 태그 사이의)
  int processing_unit;             // 분석 단위 (어절, 형태소, 음절)
  int cutoff_threshold_m;          // 형태소 단위 분석
  int cutoff_threshold_s;          // 음절 단위 분석
  int beam_size;                   // 음절 단위 분석

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
