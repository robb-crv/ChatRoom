CC= gcc -Wall -O0 -g
LDFLAGS= -lpthread 

all: server 

server: server.o
		$(CC) -c server.o $(LDFLAGS) 
	
server.o: server.c
		$(CC) -c server.c

clean: 
	rm -rf *o server
