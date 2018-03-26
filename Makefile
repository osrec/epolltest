CC=gcc -pg
CFLAGS=-I. `pkg-config --cflags glib-2.0`
DEPS =	fserve.h utils.h worker.h nodeProtocol.h reactor.h
OBJ =	fserve.o utils.o worker.o nodeProtocol.o reactor.o
LIBS = -lpthread -lm `pkg-config --libs glib-2.0`

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

fserve: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	
.PHONY: clean

clean:
	rm -f *.o *~