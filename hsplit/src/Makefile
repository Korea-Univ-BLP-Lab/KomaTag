CC = gcc
#For Optimization
CFLAGS = -O2
#For debugging
#CFLAGS = -g

RM= rm -f
CP= cp

LIB= ../lib/
BIN= ../bin/

.c.o: 
	$(CC) -c $(CFLAGS) $<

all:	libhsplit.a hsplit

libhsplit.a:    hsplit.o
	$(AR) rcv $(LIB)$@ hsplit.o
	ranlib $(LIB)$@

hsplit:	hsplit_test.o  $(LIB)libhsplit.a
	$(CC) -o $(BIN)$@ hsplit_test.o $(LIB)libhsplit.a

clean: 	rmobj rmlib rmbin
rmobj:
	${RM} *.o 
rmlib:
	${RM} $(LIB)libhsplit.a
rmbin:
	${RM} $(BIN)hsplit

