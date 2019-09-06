
all:
	gcc -o servidorChat linkedlist.c servidorChat.c -Wall -lpthread

clean:
	rm -f servidorChat


