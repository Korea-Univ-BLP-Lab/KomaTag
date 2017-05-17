#ifndef WIN32


#include <stdio.h>
#include "env.h"
#include "definitions.h"

//#include "maxentmodel.hpp"
#include <maxent/maxentmodel.hpp>

using namespace maxent;

int me_neohantag_open(MaxentModel &head_m, MaxentModel &tail_m, 
                      MaxentModel &dummy_head_m,
                      WORD_FREQ &full_morpheme_map) {

  FILE *fp;

  ///////////////////////////////////////////////////////////
  // 실질 형태소 정의 파일 읽기
  fprintf(stderr, "\tLoading fullmorpheme definition file [%s]..", rsc_file_with_full_path[FULLMORPHEME_DEF]);
  
  // 파일 열기
  if ((fp = fopen (rsc_file_with_full_path[FULLMORPHEME_DEF], "rt")) == NULL) {
    fprintf(stderr, "File open error : %s\n", rsc_file_with_full_path[FULLMORPHEME_DEF]);
    return 0;
  }

  char tagname[10];

  while (fscanf(fp, "%s", tagname) != EOF) {
    full_morpheme_map[tagname]++;
  }
  
  fclose(fp); // 파일 닫기

  fprintf(stderr, "\t[done]\n");

  ///////////////////////////////////////////////////////////
  // 최대 엔트로피 모델 로딩
  fprintf(stderr, "\tLoading maximum entropy model [%s]..", rsc_file_with_full_path[HEAD]);
  head_m.load (rsc_file_with_full_path[HEAD]);
  fprintf(stderr, "\t[done]\n");
  
  fprintf(stderr, "\tLoading maximum entropy model [%s]..", rsc_file_with_full_path[DUMMY_HEAD]);
  dummy_head_m.load (rsc_file_with_full_path[DUMMY_HEAD]);
  fprintf(stderr, "\t[done]\n");


  return 1;

}

#endif
