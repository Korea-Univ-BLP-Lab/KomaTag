CC = g++
CFLAGS = -O2
#CFLAGS = -g
RM= rm -f
CP= cp -r

BIN= ../bin/
LIB=../lib/

.cpp.o: 
	$(CC) -c $(CFLAGS) $<

all:	libprobability_tool.a

libprobability_tool.a: fst_probability_io.o \
                      probability_estimation.o \
                      fst_get_probability.o \
                      map_get_probability.o \
                      map_probability_io.o 
	$(AR) rcv $(LIB)$@ fst_probability_io.o \
                      probability_estimation.o \
                      fst_get_probability.o \
                      map_get_probability.o \
                      map_probability_io.o
	ranlib $(LIB)$@

clean: 	rmobj rmbin rmlib
rmobj:
	${RM} *.o 
rmbin:
	${RM} $(BIN)*

rmlib:
	${RM} $(LIB)*

