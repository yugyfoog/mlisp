CFLAGS = -Wall -g

mlisp: mlisp.o
	cc -g -o mlisp mlisp.o


clean:
	rm -f *.o *~ *.stackdump


