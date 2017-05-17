CC = gcc
CFLAGS = -O2
#CFLAGS = -g

.c.o: 
	$(CC) -c $(CFLAGS) $<

all: total
total:
	cd src; make; cd ..;
clean:
	cd src; make clean; cd ..;
