
all:
	gcc -o servidorChat linkedlist.c servidorChat.c -Wall -lpthread
	gcc -o clientChat clientChat.c -Wall -lpthread

clean:
	rm -f servidorChat


