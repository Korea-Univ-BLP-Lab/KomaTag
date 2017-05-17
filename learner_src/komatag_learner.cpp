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

  g = init_globals(NULL);  // �޸� �Ҵ� �� �ʱ�ȭ
  g->cmd = strdup(get_basename(argv[0], NULL)); // �� ��ɾ� �̸��� ã�� ����
  
  // �ɼ� �˾Ƴ���
  int inputstart = get_options(g, argc, argv);

  report(1, "\n%s %s\n", Description, Version);
  report(6, "verbosity level = %d\n", verbosity);

  //////////////////////////////////////////////////////////////////////////
  // ��� �Է� ȭ�Ͽ� ����
  for (int i = inputstart; i < argc; i++) {
    
    fprintf(stderr, "Extracting Eojeol information from [%s]..\n", argv[i]);
    //////////////////////////////////////////////////////////////////////////
    // ���� ���� �н���
    prokoma_learn_e(5, argv[i]);
    //////////////////////////////////////////////////////////////////////////

    char morpheme_tagged_file[100];
    sprintf(morpheme_tagged_file, "%s.morph", argv[i]);
    fprintf(stderr, "Morpheme tagging from [%s] to [%s]\n", argv[i], morpheme_tagged_file);
    //////////////////////////////////////////////////////////////////////////
    // ǰ�� ���� ����ġ�κ��� ���¼ҿ� ǰ�� ����
    morpheme_tagging(argv[i], morpheme_tagged_file, g->delimiter);
    //////////////////////////////////////////////////////////////////////////
    
    char syllable_tagged_file[100];
    sprintf(syllable_tagged_file, "%s.syll", argv[i]);
    fprintf(stderr, "Syllable tagging from [%s] to [%s]\n", argv[i], syllable_tagged_file);
    //////////////////////////////////////////////////////////////////////////
    // ǰ�� ���� ����ġ�κ��� ���� ���� ǰ�� �±�
    syllable_tagging(argv[i], syllable_tagged_file, g->delimiter);
    //////////////////////////////////////////////////////////////////////////

    fprintf(stderr, "Extracting phonetic information from [%s]..\n", argv[i]);
    //////////////////////////////////////////////////////////////////////////
    // ǰ�� ���� ����ġ�κ��� ���� ���� ����
    phonetic_info(argv[i], PHONETIC_PRB, PHONETIC_INFO, g->delimiter);
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // ���¼� ���� �н���
    // �Է� : ���¼� �±�� ����ġ
    prokoma_learn_m(morpheme_tagged_file);
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // ���� ���� �н���
    // �Է� : ���� ���� ǰ�� �±�� ����ġ
    prokoma_learn_s(syllable_tagged_file);
    //////////////////////////////////////////////////////////////////////////

    // ���� ����
    //morpheme_tagged_file
    //syllable_tagged_file

    
    //////////////////////////////////////////////////////////////////////////
    // ǰ�� �°� �н���
    // HMM ��
    neohantag_learn(argv[i], g->delimiter);
    
    // �ִ� ��Ʈ���� ��
    //me_neohantag_learn(argv[i], g->delimiter);

    // ���� ���� HMM ��
//    neohantag_ej_learn(argv[i], g->delimiter);

  } // end of for


  free_globals(g); // ���� ������ ���� �޸� ���� �� ȭ�� �ݱ�

  return 1;
}
