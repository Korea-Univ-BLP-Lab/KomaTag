komatag

[사용법]
  komatag [OPTIONS] input-file(s)
  komatag [OPTIONS] [< stdin] [> stdout]

*입력파일명 끝에 ".out"이 붙는다.

[옵션]

  -h : display help

      도움말 출력
     
  -v verbosity[0-6] [default = 2]

      프로그램 실행중 메시지 출력의 정도 (숫자가 높을수록 자세한 메시지가 출력됨)
      
  -d path : resource directory [default = current directory]

      리소스가 있는 디렉토리(폴더) (기본값은 현재 디렉토리)

  -m : run morphological analysis only
       otherwise, run morphological analysis and POS tagging [default]

       형태소 분석만 수행하는 옵션
       이 옵션을 설정하지 않으면 기본값으로 태깅까지 수행한다.
       
  -i input_file_style [default = t]
       t : text without sentence breaking
       rs : row format text with sentence breaking (one sentence per line)
       cs : column format text with sentence breaking
            (blank line in each sentence boundary)
       m : morphological analyzed text without sentence breaking (KU style)
       ms : morphological analyzed text with sentence breaking (KU style)
            (blank line in each sentence boundary)
            
       입력 파일의 형식을 지정한다. (기본값은 t)
       t : 문장 분리가 되어있지 않은 텍스트
       rs : 한 줄에 한 문장이 들어있는 텍스트
       cs : 한 줄에 한 어절이 들어있는 텍스트 (문장 구분은 빈 라인으로 한다.)
       m : 문장 분리 되지 않은 형태소 분석된 텍스트 (단, KU 형식이어야 함)
       ms : 문장 분리된 형태소 분석된 텍스트 (단, KU 형식이어야 함)
       
  -s : print the results with sentence breaking (by blank line)
       otherwise, print the results without sentence breaking [default]

      문장 구분 결과를 함께 출력 (빈 라인으로 구분)
      이 옵션을 설정하지 않으면 출력시 문장 구분을 하지 않는다.
      
  -k : print the results with the KU style
         ex) "morph1/pos1+morph2/pos2+..."
       otherwise, print the results with the Sejong corpus style [default]
         ex) "morph1/pos1 + morph2/pos2 + ..."
         
       KU 형식으로 결과를 출력한다.
         예) 시작했다.	시작/NNG+하/XSV+았/EP+다/EF+./SF
         
       이 옵션이 지정되지 않으면 세종 말뭉치 형식으로 출력함 (기본값)
         예) 시작했다.	시작/NNG + 하/XSV + 았/EP + 다/EF + ./SF
       
  -l delimiter : set the delimiter between morphemes and tags [default = '/']

       형태소와 태그 사이의 구분자를 지정한다. (기본값은 '/')
       
  -u processing_unit_type [default = ems]
       e : Eojeol-unit analysis
       m : morpheme-unit analysis
       s : syllable-unit analysis
       em : Eojeol-unit + morpheme-unit analysis
       es : Eojeol-unit + syllable-unit analysis
       ms : morpheme-unit + syllable-unit analysis
       ems : Eojeol-unit + morpheme-unit + syllable-unit analysis

       분석 단위를 지정한다. (기본값은 ems)
       e : 어절 단위 분석
       m : 형태소 단위 분석
       s : 음절 단위 분석
       em : 어절 단위 + 형태소 단위 분석
       es : 어절 단위 + 음절 단위 분석
       ms : 형태소 단위 + 음절 단위 분석
       ems : 어절 단위 + 형태소 단위 + 음절 단위 분석

  -x [1-20]: cutoff threshold for morpheme-unit analysis [default = 10]

      형태소 단위 분석을 위한 임계값 (기본값은 10)
      
  -y [1-20]: cutoff threshold for syllable-unit analysis [default = 10]

      음절 단위 분석을 위한 임계값 (기본값은 10)
      
  -b [1-20] : beam size for syllable-unit analysis [default = 15]

      음절 단위 분석을 위한 빔 사이즈 (기본값은 15)

   * x, y, b 옵션에서 지정하는 값들은 클수록 더 많은 분석 결과를 내며, 분석 속도는 느려진다.

[사용예]
  test.txt라는 파일을 형태소 분석과 태깅을 하고 싶으면
  komatag test.txt
  
  형태소 분석만 하고 싶으면
  komatag -m test.txt
  
  만약 어절 단위 분석과 음절 단위 분석을 하고 싶으면
  komatag -u es test.txt
  
  과 같이 하면 된다.

===============================================================================
komatag-nbest

[사용법]
komatag-nbest [OPTIONS] input-file [output-file]
komatag-nbest [OPTIONS] [< stdin] [> stdout]

*출력파일명을 생략하면 입력파일명 끝에 ".out"이 붙는다.

  -h : display help
  
       도움말 출력
    
  -v verbosity[0-6] [default = 2]
  
     프로그램 실행중 메시지 출력의 정도 (숫자가 높을수록 자세한 메시지가 출력됨)
  
  -d path : resource directory [default = current directory]
     
     리소스가 있는 디렉토리(폴더) (기본값은 현재 디렉토리)
     
  -i input_file_style [default = m]
       m : morphological analyzed text without sentence breaking (KU style)
       ms : morphological analyzed text with sentence breaking (KU style)
            (blank line in each sentence boundary)
            
      입력 파일의 형식을 지정한다. (기본값은 m)
       m : 문장 분리 되지 않은 형태소 분석된 텍스트 (단, KU 형식이어야 함)
       ms : 문장 분리된 형태소 분석된 텍스트 (단, KU 형식이어야 함)
       
  -s : print the results with sentence breaking (by blank line)
       otherwise, print the results without sentence breaking [default]
       
       문장 구분 결과를 함께 출력 (빈 라인으로 구분)
       이 옵션을 설정하지 않으면 출력시 문장 구분을 하지 않는다.

  -k : print the results with the KU style
         ex) "morph1/pos1+morph2/pos2+..."
       otherwise, print the results with the Sejong corpus style [default]
         ex) "morph1/pos1 + morph2/pos2 + ..."

       KU 형식으로 결과를 출력한다.
         예) 시작했다.	시작/NNG+하/XSV+았/EP+다/EF+./SF
         
       이 옵션이 지정되지 않으면 세종 말뭉치 형식으로 출력함 (기본값)
         예) 시작했다.	시작/NNG + 하/XSV + 았/EP + 다/EF + ./SF

  -c constraint type for N-best tagging [default = c]
       c : consistency constraint between morphological analysis and tagging
       a : c + absolute threshold constraint
       r : c + relative threshold constraint
       ar : c + a + r
       
       n-best 태깅을 위한 제약의 유형 (하나의 결과를 출력하기 위한 조건) (기본값은 c)
       c : 형태소 분석 결과 중 1위와 품사 태깅 결과가 일치해야 함
       a : c와 절대 임계값 제약을 함께 사용함
       r : c와 상대 임계값 제약을 함께 사용함
       ar : c와 a와 r을 모두 사용함
       
  -r [1-20]: relative threshold value [default = 5]
       
       상대 임계값 설정 (기본값은 5)
       
  -a [1-1000]: absolute threshold value [default = 10]
  
       절대 임계값 설정 (기본값은 5)
