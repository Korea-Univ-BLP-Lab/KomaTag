# For gcc
CC = g++
#CC = gcc

#For Optimization
CFLAGS = -O2
#For debugging
#CFLAGS = -g
INCLUDE = -I../include

RM= /bin/rm -f

.cpp.o: 
	$(CC) -c $(INCLUDE) $(CFLAGS) $<

all:	libdafst.a

libdafst.a:	ANCommon.o FST.o
	ar -r ../lib/$@ ANCommon.o FST.o

clean:
	rm -f *.o ../lib/*.a

