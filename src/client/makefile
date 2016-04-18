
CC= gcc -Wall -O0 -g
LDFLAGS= -lpthread 


all: client

client: client.o 
		$(CC) -c client.o $(LDFLAGS) 


client.o: client.c
		$(CC) -c client.c
 		
clean: 
		rm -rf *o client
		
