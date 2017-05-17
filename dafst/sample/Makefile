# For gcc
CC = g++

#For Optimization
CFLAGS = -O2
#For debugging
#CFLAGS = -g

INCLUDES = -I../include
LIBS = -L../lib

RM= /bin/rm -f

.cpp.o: 
	$(CC) -c $(CFLAGS) $(INCLUDES) $<

all:	build append remove hash2str traverse traverse2 str2tabular

build:	build.o 
	$(CC) -o ../bin/$@ $(LIBS) build.o -ldafst

append:	append.o
	$(CC) -o ../bin/$@ $(LIBS) append.o -ldafst

remove:	remove.o
	$(CC) -o ../bin/$@ $(LIBS) remove.o -ldafst

#str2hash:	str2hash.o
#	$(CC) -o ../bin/$@ $(LIBS) str2hash.o -ldafst

hash2str:	hash2str.o
	$(CC) -o ../bin/$@ $(LIBS) hash2str.o -ldafst

traverse:	traverse.o
	$(CC) -o ../bin/$@ $(LIBS) traverse.o -ldafst

traverse2:	traverse2.o
	$(CC) -o ../bin/$@ $(LIBS) traverse2.o -ldafst

str2tabular:       str2tabular.o
	$(CC) -o ../bin/$@ $(LIBS) str2tabular.o -ldafst

clean:
	rm -f *.o *.a
