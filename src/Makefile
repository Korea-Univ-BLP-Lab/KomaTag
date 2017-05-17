CC = g++
CFLAGS = -O2
#CFLAGS = -g
RM= rm -f
BIN= ../bin/
LIB= ../lib/

ENTRY2FST_LIB= ../dafst/lib/libentry2fst.a 
PROBABILITY_TOOL_LIB= ../probability_tools/lib/libprobability_tool.a
HSPLIT_LIB= ../hsplit/lib/libhsplit.a

CP= cp
.cpp.o: 
	$(CC) -c $(CFLAGS) $<

all: 	libprokoma.a libneohantag.a komatag komatag-nbest

libprokoma.a: prokoma.o prokoma_e.o \
              prokoma_m.o \
              phonetic_change.o triangular_matrix.o \
              prokoma_s.o \
              unit_conversion.o bfs.o 
	$(AR) rcv $(LIB)$@ prokoma.o prokoma_e.o \
              prokoma_m.o \
              phonetic_change.o triangular_matrix.o \
              prokoma_s.o \
              unit_conversion.o bfs.o 
	ranlib $(LIB)$@

libneohantag.a: get_morph_tag.o neohantag.o viterbi.o 
	$(AR) rcv $(LIB)$@ get_morph_tag.o neohantag.o viterbi.o
	ranlib $(LIB)$@

komatag: komatag.o \
	common.o env.o get_sentence.o global_option.o \
	report.o 
	$(CC) -o $(BIN)$@ komatag.o \
        common.o env.o get_sentence.o global_option.o \
        report.o \
        $(LIB)libprokoma.a $(LIB)libneohantag.a \
        $(ENTRY2FST_LIB) \
	      $(PROBABILITY_TOOL_LIB) \
	      $(HSPLIT_LIB) \
        -lm

komatag-nbest: komatag-nbest.o constraint-nbest.o \
	common.o get_sentence.o report.o env.o global_option-nbest.o
	$(CC) -o $(BIN)$@ komatag-nbest.o constraint-nbest.o \
	common.o get_sentence.o report.o env.o global_option-nbest.o \
	$(LIB)libprokoma.a $(LIB)libneohantag.a \
	$(ENTRY2FST_LIB) \
	$(PROBABILITY_TOOL_LIB) \
	$(HSPLIT_LIB) \
	-lm

clean: 	rmobj rmbin rmlib

rmobj:
	${RM} *.o 

rmbin:
	${RM} $(BIN)komatag
	${RM} $(BIN)komatag-nbest

rmlib:
	${RM} $(LIB)libprokoma.a
	${RM} $(LIB)libneohantag.a
