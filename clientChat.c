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

struct client_data{
    char nickname[32];
    int sk;
    struct sockaddr_in *client_addr;
};

char * recvMSG(int s, char *msg){
    int pos = 0;
    msg[0] = 1;
    while (msg[pos]){
        recv(s, &msg[pos], 1, 0);
        pos++;
    }
    return msg;
}

void sendMSG(int s, char code, char *msg){
    send(s, &code, 1, 0);
    send(s, msg, strlen(msg)+1, 0);
}

void finish_client(struct linkedlist_t *l, struct client_data *c){
        close(c->sk);
        printf("Client disconnected.\n");
        fflush(stdout);
}

void * client_handle(void* cd){
    struct client_data *client = (struct client_data *)cd;
    char msg[512];
    int running = 1;

    /* Client accepted, start chat. */
    while(running){
        recvMSG(client->sk, msg);
        switch (msg[0]){
            case CODE_MESSAGE_PUBLIC:
                send_to_all(thread_list, &msg[1]);
                break;
            case CODE_MESSAGE_PRIVATE:
                if(send_private(thread_list, &msg[1])){
                    sendMSG(client->sk, CODE_SUCESS, "");
                }else{
                    sendMSG(client->sk, CODE_NICKNAME_NOT_FOUND, "");
                }
                break;
            case CODE_DISCONNECT:
                sendMSG(client->sk, CODE_SUCESS, "");
                running = 0;
                break;
            case CODE_LIST_ALL:
                get_list(thread_list, msg);
                sendMSG(client->sk, CODE_NAME_LIST, msg);
                break;
        }

    }

    /* Fecha conexão com o cliente. */
    finish_client(thread_list, client);
    return NULL;
}


int main(int argc, char *argv[])
{
    int listenfd = 0, sockfd;
    struct sockaddr_in serv_addr; 
    int addrlen;
    struct client_data *cd;
    pthread_t thr;
    char nickname[32];

    printf("Enter Server IP : ");
    fgets(ip_dest, sizeof(ip_dest), stdin);

    printf("Enter Nickname : ");
    fgets(nickname, sizeof(nickname), stdin);


    /*Cria o Socket */
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    /* Configura o IP de destino e porta na estrutura sockaddr_in */
    memset(&serv_addr, 0, sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000); 

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0){
        printf("\n inet_pton error occured\n");
        return 1;
    } 

    /* Conecta ao servidor. */
    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
       printf("\n Error : Connect Failed \n");
       return 1;
    } 


    sendMSG(sockfd, CODE_NICKNAME, nickname);
    recvMSG(sockfd, msg);
    if(msg[0] != CODE_SUCESS){
        printf("Finish by error code &d\n", CODE_ERROR);
        exit(0);
    }
    
    cd = (struct client_data *)malloc(sizeof(struct client_data));
    memset(cd, 0, sizeof(struct client_data));
    cd->client_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    addrlen = sizeof(struct sockaddr_in);
    pthread_create(&thr, NULL, client_handle, (void *)cd);
    pthread_detach(thr);

    

    return 0;

}





