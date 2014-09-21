CC=gcc
OBJ1=client.c
OBJ2=server.c

all: client.o server.o

client.o:$(OBJ1)
	$(CC) -o $@ -c $<
	
server.o:$(OBJ2)
	$(CC) -lpthread -o $@ -c $<
	
.PHONY:clean
clean:
	rm -f *.o