CC = g++
CFLAGS = -O2
#CFLAGS = -g
RM= rm -f
CP= cp

BIN= ../bin/
LIB=../lib/

.cpp.o: 
	$(CC) -c $(CFLAGS) $<

all:	libentry2fst.a entry2fst

libentry2fst.a: fileio.o get_fst.o ../src/FST.o ../src/ANCommon.o entry2fst.o
	$(AR) rcv $(LIB)$@ fileio.o get_fst.o ../src/FST.o ../src/ANCommon.o entry2fst.o
	ranlib $(LIB)$@

entry2fst: entry2fst_test.o 
	$(CC) -o $(BIN)$@ entry2fst_test.o $(LIB)libentry2fst.a

clean: 	rmobj rmbin
rmobj:
	${RM} *.o 
rmbin:
	${RM} $(BIN)entry2fst

