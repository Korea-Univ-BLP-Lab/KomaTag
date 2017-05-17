#include <stdio.h>
#include "definitions.h"
#include "prokoma_learn.h"
#include "neohantag_learn.h"
#include "me_neohantag_learn.h"
#include "global_option.h"
#include "report.h"

#define PHONETIC_PRB "PHONETIC.prb"
#define PHONETIC_INFO "PHONETIC.info"

//////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) { 

  g = init_globals(NULL);  // 메모리 할당 및 초기화
  g->cmd = strdup(get_basename(argv[0], NULL)); // 순 명령어 이름만 찾기 위해
  
  // 옵션 알아내기
  int inputstart = get_options(g, argc, argv);

  report(1, "\n%s %s\n", Description, Version);
  report(6, "verbosity level = %d\n", verbosity);

  //////////////////////////////////////////////////////////////////////////
  // 모든 입력 화일에 대해
  for (int i = inputstart; i < argc; i++) {
    
    fprintf(stderr, "Extracting Eojeol information from [%s]..\n", argv[i]);
    //////////////////////////////////////////////////////////////////////////
    // 어절 단위 학습기
    prokoma_learn_e(5, argv[i]);
    //////////////////////////////////////////////////////////////////////////

    char morpheme_tagged_file[100];
    sprintf(morpheme_tagged_file, "%s.morph", argv[i]);
    fprintf(stderr, "Morpheme tagging from [%s] to [%s]\n", argv[i], morpheme_tagged_file);
    //////////////////////////////////////////////////////////////////////////
    // 품사 부착 말뭉치로부터 형태소와 품사 추출
    morpheme_tagging(argv[i], morpheme_tagged_file, g->delimiter);
    //////////////////////////////////////////////////////////////////////////
    
    char syllable_tagged_file[100];
    sprintf(syllable_tagged_file, "%s.syll", argv[i]);
    fprintf(stderr, "Syllable tagging from [%s] to [%s]\n", argv[i], syllable_tagged_file);
    //////////////////////////////////////////////////////////////////////////
    // 품사 부착 말뭉치로부터 음절 단위 품사 태깅
    syllable_tagging(argv[i], syllable_tagged_file, g->delimiter);
    //////////////////////////////////////////////////////////////////////////

    fprintf(stderr, "Extracting phonetic information from [%s]..\n", argv[i]);
    //////////////////////////////////////////////////////////////////////////
    // 품사 부착 말뭉치로부터 음운 복원 정보
    phonetic_info(argv[i], PHONETIC_PRB, PHONETIC_INFO, g->delimiter);
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // 형태소 단위 학습기
    // 입력 : 형태소 태깅된 말뭉치
    prokoma_learn_m(morpheme_tagged_file);
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // 음절 단위 학습기
    // 입력 : 음절 단위 품사 태깅된 말뭉치
    prokoma_learn_s(syllable_tagged_file);
    //////////////////////////////////////////////////////////////////////////

    // 파일 삭제
    //morpheme_tagged_file
    //syllable_tagged_file

    
    //////////////////////////////////////////////////////////////////////////
    // 품사 태거 학습기
    // HMM 모델
    neohantag_learn(argv[i], g->delimiter);
    
    // 최대 엔트로피 모델
    //me_neohantag_learn(argv[i], g->delimiter);

    // 어절 단위 HMM 모델
//    neohantag_ej_learn(argv[i], g->delimiter);

  } // end of for


  free_globals(g); // 전역 변수에 대한 메모리 해제 및 화일 닫기

  return 1;
}
