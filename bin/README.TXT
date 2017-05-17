#문장분리
sentence_break_tagged sj2003.cps sj2003.cps2

#학습
komatag_learn sj2003.cps2
./komatag_learn ~/experiment/eval_hantag/etri/0/trn.cps

#실행

#형태소분석
komatag -d /home/dglee/experiment/eval_hantag/sjall/0  -m -k -s test.txt test.mout

#형태소분석+태깅
komatag -d /home/dglee/experiment/eval_hantag/sjall/0  -k test.txt

#태깅
komatag -d /home/dglee/experiment/eval_hantag/sjall/0  -ims -k -s test.mout test.tout
