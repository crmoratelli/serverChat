#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <pthread.h>
#include "linkedlist.h"

#define NICKNAMESZ 32
#define CODE_NICKNAME           100
#define CODE_MESSAGE_PUBLIC     101  
#define CODE_MESSAGE_PRIVATE    102
#define CODE_DISCONNECT         103
#define CODE_LIST_ALL           104
#define CODE_SUCESS             105
#define CODE_NICKNAME_NOT_FOUND 106
#define CODE_NAME_LIST          107
#define CODE_ERROR              108
#define CODE_CLIENT_MESSAGE     109
#define CODE_KEEPALIVE          110

#define OPTION_LIST_ALL			1
#define OPTION_SEND_ALL			2
#define OPTION_SEND_PRIVATE		3
#define OPTION_DISCONNECT		4

struct client_data{
    char nickname[32];
    int sk;
};

char * recvMSG(int s, char *msg){
    int pos = 0;
    while (1){
        recv(s, &msg[pos], 1, 0);
        if(!msg[pos]){
            break;
        }
        pos++;
    }
    return msg;
}

void sendMSG(int s, char code, char *msg){
    send(s, &code, 1, 0);
    send(s, msg, strlen(msg)+1, 0);
}

void * client_handle(void* cd){
    struct client_data *client = (struct client_data *)cd;
    char msg[512];
    int running = 1;

    /* Client accepted, start chat. */
    while(running){
        recvMSG(client->sk, msg);
        switch(msg[0]){
            case CODE_CLIENT_MESSAGE:
                printf("%s\n", &msg[1]);
                sendMSG(client->sk, CODE_SUCESS, "");
                break;
            case CODE_KEEPALIVE:
                sendMSG(client->sk, CODE_SUCESS, "");
                break;
            case CODE_ERROR:
                exit(0);
            default:
                printf("Message not recognized %d\n", msg[0]);
                exit(0);
                break;
        }
    }

    return NULL;
}

void print_options(){
	printf("Option: "\n);
	printf("\t1 - List all\n");
	printf("\t2 - Send to all\n");
	printf("\t3 - Send private\n");
	printf("\t4 - Disconnect\n");
	printf("Press option: ");
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in serv_addr; 
    struct client_data *cd;
    pthread_t thr;
    char msg[512];
    int op;
    

    if (argv < 3){
    	printf("Usage: %s <ip> <port> <nickname>\n");
    }

    /*Cria o Socket */
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    /* Configura o IP de destino e porta na estrutura sockaddr_in */
    memset(&serv_addr, 0, sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2])); 

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0){
        printf("\n inet_pton error occured\n");
        return 1;
    } 

    /* Conecta ao servidor. */
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
       printf("\n Error : Connect Failed \n");
       return 1;
    } 

    printf("Sending nickname... \n");
    sendMSG(sockfd, CODE_NICKNAME, argv[3]);
    printf("Waiting response... \n");
    recvMSG(sockfd, msg);
    if(msg[0] != CODE_SUCESS){
        printf("Finished by error code %d\n", CODE_ERROR);
        exit(0);
    }
    
    cd = (struct client_data *)malloc(sizeof(struct client_data));
    memset(cd, 0, sizeof(struct client_data));
    strcpy(cd->nickname, argv[3]);
    cd->sk = sockfd;
    pthread_create(&thr, NULL, client_handle, (void *)cd);
    pthread_detach(thr);

    while(strcmp(msg, "quit")){
    	print_options();
    	scanf("%d", &op);

    	switch(op){

    	}

        printf("Send a message: ");
        fgets(msg, sizeof(msg), stdin);

        sendMSG(sockfd, CODE_MESSAGE_PUBLIC, msg);
    }

    
    return 0;

}





