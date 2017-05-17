#include <stdio.h>
#include "probability_tool.h"

/******************************************************************************/
/* Ȯ���� ��� (text ���) */
// mode : "t" = text file, "b" = binary file
int map_print_probability(char *filename, PROB_MAP &probs, char *mode) {
  
  FILE *fp;

  // text ����̸�
  if (strcmp(mode, "t") == 0) {

    if ((fp = fopen(filename, "wt")) == NULL) {
      fprintf(stderr, "Error: cannot open file [%s]\n", filename);
      return 0;
    }
  
    for (PROB_MAP::iterator denom = probs.begin(); denom != probs.end(); ++denom)  {

      for (NUM_PROB::iterator num = probs[denom->first].begin(); 
           num != probs[denom->first].end(); ++num) {

        fprintf(fp, "%s\t%s\t%12.11e\n", denom->first.c_str(), /* �и� */
                num->first.c_str(), /* ����-�и� */
                probs[denom->first][num->first]); /* Ȯ�� */
      }
    }
  } // end of if
  
  // binary ����̸�
  else {

    double prob;
    int length;

    if ((fp = fopen(filename, "wb")) == NULL) {
      fprintf(stderr, "Error: cannot open file [%s]\n", filename);
      return 0;
    }

    for (PROB_MAP::iterator denom = probs.begin(); denom != probs.end(); ++denom)  {

      for (NUM_PROB::iterator num = probs[denom->first].begin(); 
        num != probs[denom->first].end(); ++num) {

        length = (int) strlen(denom->first.c_str()) + 1; 

        fwrite(denom->first.c_str(), length, 1, fp); /* ���ڿ� (�и�) */

        length = (int) strlen(num->first.c_str()) + 1; 

        fwrite(num->first.c_str(), length, 1, fp); /* ���ڿ� (���� - �и�) */

        prob = probs[denom->first][num->first]; 
        fwrite(&prob, sizeof(double), 1, fp); /* Ȯ�� */
      }
    }

  } // end of else

  fclose(fp);
  return 1;
}

/******************************************************************************/
/* Ȯ���� �Է� (text ���) */
/* ���ϰ� : ���� = 0, Ȯ���� �� */
// mode : "t" = text file, "b" = binary file
int map_scan_probability(char *filename, PROB_MAP &probs, char *mode) {

  FILE *fp;

  int count = 0;
  int line_num = 0;

  // text ����̸�
  if (strcmp(mode, "t") == 0) {
  
    char denom[1000];
    char num[1000];
    double prob;

    int num_item;

    if ((fp = fopen(filename, "rt")) == NULL) {
      fprintf(stderr, "Error: cannot open file [%s]\n", filename);
      return 0;
    }

    while ((num_item = fscanf(fp, "%s%s%lf", 
                  denom, /* �и� */
                  num,   /* ����-�и� */
                  &prob)) != EOF) /* log(Ȯ��) */  {

      line_num++;

      if (num_item != 3) {
        fprintf(stderr, "line number = %d\n", line_num);
        return 0;
      }
      probs[denom][num] = prob;
      count++;
    }
  } // end of if


  // binary ����̸�
  else {
    char *denom;
    char *num;

    double prob;
    char *file_contents = NULL;

    if ((fp = fopen(filename, "rb")) == NULL) {
      fprintf(stderr, "Error: cannot open file [%s]\n", filename);
      return 0;
    }

    /* ȭ���� ũ�⸦ �˾Ƴ� */
    fseek(fp, 0, SEEK_END); /* ȭ���� �� */
    long FileSize = ftell(fp);   /* ȭ���� ��ġ */
    fseek(fp, 0, SEEK_SET); /* ȭ���� ó�� */

    file_contents = (char *) malloc(FileSize); /* �޸� �Ҵ� */
    if (file_contents == NULL) {
      fprintf(stderr, "Not enough memory\n");
      return 0;
    }

    fread(file_contents, FileSize, 1, fp); /* ȭ�� ��ü�� �о���� */

    //file_contents[FileSize] = 0; /* NULL */
    
    char *ptr;
    ptr = file_contents;

    while (1) {

      if (ptr - file_contents >= FileSize) break; /* ȭ�� ������ �� �о����� ���� */

      denom = ptr; /* �и� */
      ptr += (strlen(denom) + 1);

      num = ptr; /* ���� */
      ptr += (strlen(num) + 1);

      memcpy(&prob, ptr, sizeof(double)); /* Ȯ�� */
      ptr += sizeof(double);
      
      probs[denom][num] = prob;

      count++;
    }
   
    free(file_contents); /* �޸� ���� */
    
  } // end of else

  fclose(fp);

  return count;
}

/******************************************************************************/
/* �±� ������ �˾Ƴ� */
void get_tagset(PROB_MAP &probs, TAGSET &tagset) {

  /* word */
  for (PROB_MAP::iterator denom = probs.begin(); denom != probs.end(); ++denom)  {
    /* tag */
    for (NUM_PROB::iterator num = probs[denom->first].begin(); 
      num != probs[denom->first].end(); ++num) {

        tagset[num->first]++;
    }
  }
}
