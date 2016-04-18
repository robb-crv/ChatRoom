
#ifndef __METHODS_H__ // evitare inclusioni multiple di un header
#define __METHODS_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>

//limite connessioni
#define MAX_CLIENT          20
#define MAX_USERS_CHAN      10
#define MAX_CHANNEL_JOINED  5
#define QUEUE_LEN           5

//size
#define SIZE_MSG            1024
#define USERNAME_SIZE       128

//comandi 
#define CREATE_NEW_CMD  "new"
#define JOIN_CMD        "join"
#define LEAVE_CMD       "leave"
#define CLOSE_CMD       "close"
#define CMD_CHAR        "#"

#define SERVER_NAME     "Chat"

//codici errore
#define ERR_MAX_USERS              -11
#define ERR_USERNAME_NOT_AVAILABLE -12



#define GESTIONE_ERRORE_GENERICO(condizione, codice_errore, messaggio_errore) do {     \
        if (condizione) {                                                              \
            fprintf(stderr, "%s: %s\n", messaggio_errore, strerror(codice_errore));    \
            exit(EXIT_FAILURE);                                                        \
        }                                                                              \
    } while(0)


#define GESTIONE_ERRORE(ret, msg)          GESTIONE_ERRORE_GENERICO((ret < 0), errno, msg)
#define GESTIONE_ERRORE_THREAD(ret, msg)   GESTIONE_ERRORE_GENERICO((ret != 0), ret, msg)

//struttura dati messaggi-username
typedef struct msg {

    char username[USERNAME_SIZE];
    char msg[SIZE_MSG];

} msg_t;

//struttura dati per il thread-i che gestisce la connessione
typedef struct thread_arg {

    int sock;
    struct sockaddr_in* addr;

}thread_arg_t;


//struttura dati per info sui client
typedef struct user_info {

    int socket;
    unsigned short port_numb;
    char address[INET_ADDRSTRLEN];
    char username[USERNAME_SIZE];
    unsigned int snd_msg;
    unsigned int rcv_msg;
    unsigned int num_channels_joined;
    //channel_t *channels_joined[MAX_CHANNEL_JOINED];

} user_info_t;

/*
//struttura dati per info sui canali
typedef struct channel {

    int ds_channel;
    user_info_t users[MAX_USERS_CHAN];

} channel_t;
*/

#endif
