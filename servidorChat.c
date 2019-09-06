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

pthread_mutex_t mutex_list = PTHREAD_MUTEX_INITIALIZER;

struct client_data{
    char nickname[32];
    int sk;
    struct sockaddr_in *client_addr;
    struct linkedlist_t *msg_list;
};

struct linkedlist_t *thread_list = NULL;

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

struct client_data * lookforNickname(char* nickname, struct linkedlist_t *l){
    struct linkedlist_node_t *n = l->first;

    while(n){
        if(!strcmp(nickname, ((struct client_data*)(n->elem))->nickname)){
            printf("%s %s\n", nickname, ((struct client_data*)(n->elem))->nickname);
            return n->elem;
        }
        n = n->next;
    }

    return NULL;
}

void finish_client(struct linkedlist_t *l, struct client_data *c){
        close(c->sk);
        linkedlist_destroy(&c->msg_list);
        pthread_mutex_lock(&mutex_list);
        linkedlist_remove(l, c);
        pthread_mutex_unlock(&mutex_list);
        free(c);
        printf("Client disconnected.\n");
        fflush(stdout);
}

void send_to_all(struct linkedlist_t *l, char *msg){
    struct linkedlist_node_t *n = l->first;
    while(n){
        char *m = malloc(strnlen(msg, 512));
        strncpy(m, msg, 512);
        linkedlist_insert_tail(((struct client_data *)(n->elem))->msg_list, m);
        n = n->next;
    }
}

void print_msg(char *msg){
    printf("%d %s\n", msg[0], &msg[1]);
}

int send_private(struct linkedlist_t *l, char *msg){
    struct client_data *d;
    char *nick = msg;
    int c = 0;
    char *m;

    while(*msg++ != '|' && c++ < 32);

    if(c > 32){
        return 1;
    }

    *msg++ = 0;

    d = lookforNickname(nick, l);

    if(d == NULL){
        return 1;
    }

    m = malloc(strnlen(msg, 512));
    strncpy(m, msg, 512);
    linkedlist_insert_tail(d->msg_list, m);

    return 0;
}

void get_list(struct linkedlist_t *l, char *msg){
    struct linkedlist_node_t *n = l->first;
    msg[0] = 0;

    while(n){
        strncat(msg, ((struct client_data *)(n->elem))->nickname, 32);
        n = n->next;
        if(n != NULL){
            strcat(msg, "|");
        }
    }
}

void * client_handle(void* cd){
    struct client_data *client = (struct client_data *)cd;
    char msg[512];
    int running = 1;

    client->msg_list = linkedlist_create();

    /* Imprime IP e porta do cliente. */
    printf("Received connection from %s:%d\n", inet_ntoa(client->client_addr->sin_addr), ntohs(client->client_addr->sin_port));
    fflush(stdout);

    /* Client must send message 200 */
    recvMSG(client->sk, msg);

    if(msg[0] != CODE_NICKNAME){
        sendMSG(client->sk, CODE_ERROR, "");
        finish_client(thread_list, client);      
        return NULL;
    }else{
        if (!lookforNickname(&msg[1], thread_list)){
            strncpy(client->nickname, &msg[1], NICKNAMESZ);
            sendMSG(client->sk, CODE_SUCESS, "");
        }else{ 
            /* Nickname already exist, disconnect client. */
            sendMSG(client->sk, CODE_ERROR, "");
            finish_client(thread_list, client);
            return NULL;
        }
    }

    /* Client accepted, start chat. */
    while(running){
        recvMSG(client->sk, msg);
        print_msg(msg);
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

        if(linkedlist_size(client->msg_list) > 0 && running){
            while (linkedlist_size(client->msg_list) > 0){
                char *m = linkedlist_remove_head(client->msg_list);
                sendMSG(client->sk, CODE_CLIENT_MESSAGE, m);
                free(m);
            }
        }
    }

    /* Fecha conexão com o cliente. */
    finish_client(thread_list, client);
    return NULL;
}


int main(int argc, char *argv[])
{
    int listenfd = 0;
    struct sockaddr_in serv_addr; 
    int addrlen;
    struct client_data *cd;
    pthread_t thr;

    if(argc < 2){
        printf("Usage: %s <port number>", argv[0]);
        exit(0);
    }

    thread_list = linkedlist_create();

    /* Cria o Socket: SOCK_STREAM = TCP */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));

	/* Configura servidor para receber conexoes de qualquer endereço:
	 * INADDR_ANY e ouvir na porta 5000 */ 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1])); 

	/* Associa o socket a estrutura sockaddr_in */
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

	/* Inicia a escuta na porta */
    listen(listenfd, 10); 

    while(1) {
        cd = (struct client_data *)malloc(sizeof(struct client_data));
        memset(cd, 0, sizeof(struct client_data));
        cd->client_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
        addrlen = sizeof(struct sockaddr_in);
        pthread_mutex_lock(&mutex_list);
        linkedlist_insert_tail(thread_list, cd);
        pthread_mutex_unlock(&mutex_list);

		/* Aguarda a conexão */	
        printf("Waiting for connection... ");
        cd->sk = accept(listenfd, (struct sockaddr*)cd->client_addr, (socklen_t*)&addrlen); 

        pthread_create(&thr, NULL, client_handle, (void *)cd);
        pthread_detach(thr);

     }
}




