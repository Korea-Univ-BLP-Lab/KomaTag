CC = g++
CFLAGS = -O2
#CFLAGS = -g
RM= rm -f
CP= cp

BIN= ../bin/
LIB= ../lib/
MYLIB= /home/dglee/lib/
MYBIN=/home/dglee/bin/

.c.o: 
	$(CC) -c $(CFLAGS) $<

all:	fstm2005 cpall

fstm2005:	fstm.o env.o posdic.o cmn98.o $(LIB)libdafst.a 
	$(CC) -o $(BIN)$@ fstm.o env.o posdic.o cmn98.o $(LIB)libdafst.a 

clean: 	rmobj rmbin
rmobj:
	${RM} *.o 
rmbin:
	${RM} $(BIN)fstm2005

cpall: cpbin
cpbin:
	${CP} $(BIN)fstm2005 $(MYBIN)

