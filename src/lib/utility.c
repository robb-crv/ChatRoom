
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utility_lib.h"
#include "prototype.h"

extern sem_t users_sem;
extern user_info_t* users[MAX_CLIENT];
extern int current_users_number;


void broadcast(msg_t* msg) {

    int ret;
    ret = sem_wait(&users_sem);
    GESTIONE_ERRORE(ret, "Errore sem_wait users_sem\n");

    int i;
    char msg_to_send[SIZE_MSG];
    sprintf(msg_to_send, "%s - %s", msg->username, msg->msg);

    for (i = 0; i < current_users_number; i++) {
        
        if (strcmp(msg->username, users[i]->username) != 0) {
            send_msg(users[i]->socket, msg_to_send);
            users[i]->rcv_msg++;
        } else {
            users[i]->snd_msg++;
        }
    }

    ret = sem_post(&users_sem);
    GESTIONE_ERRORE(ret, "Errore sem_post users_sem\n");
}


//la funzione user_access() gestisce il tentativo di accesso di un utente al server
//se il num di utenti non supera il limite max consentito, l'utente viene registrato 
//nelle strutture dati del server 

int user_access(int ds_socket, const char* username, struct sockaddr_in* addr) {

    int ret; 

    ret=sem_wait(&users_sem);
    GESTIONE_ERRORE(ret, "Errore sem_wait users_sem\n");

    //verifica sul numero massimo di connessioni
    if(current_users_number==MAX_CLIENT){

        ret=sem_post(&users_sem);
        GESTIONE_ERRORE(ret, "Errore sem_post users_sem\n");
        return ERR_MAX_USERS;
    }

    //verifica disponibilità username
    int i; 
    for(i=0; i<current_users_number; i++) {

        if(strcmp(username, users[i]->username) == 0){
            ret=sem_post(&users_sem);
            GESTIONE_ERRORE(ret, "Errore sem_post user_sem\n");
            return ERR_USERNAME_NOT_AVAILABLE;
        }
    }

    //registrazione utente nelle strutture dati del server
    user_info_t* new_user= (user_info_t*) malloc(sizeof(user_info_t));
    new_user->socket=ds_socket;
    inet_ntop(AF_INET, &(addr->sin_addr), new_user->address, INET_ADDRSTRLEN);
    new_user->port_numb= htons(addr->sin_port);
    sprintf(new_user->username, "%s", username);
    new_user-> snd_msg = 0;
    new_user-> rcv_msg = 0;
    new_user-> num_channels_joined = 0;

    users[current_users_number++]=new_user;

    return 0;

}


void close_socket(thread_arg_t* args){
    int ret;
    ret=close(args->sock);
    GESTIONE_ERRORE(ret, "Errore close socket\n");
    free(args->addr);
    free(args);
    pthread_exit(NULL);
}


void send_msg_SERVER(int ds_socket, char* msg ){
    char* tmp_buf[SIZE_MSG];
    sprintf(tmp_buf, "(%s) - %s", SERVER_NAME, msg);
    send_msg(ds_socket, tmp_buf);
}


void choise_message(int ds_socket) {

    char* buf[SIZE_MSG];
    
    sprintf(buf, "Lista dei comandi disponibili: \n");
    send_msg_SERVER(ds_socket, buf);

    sprintf(buf, "%s%s : crea un nuovo canale\n", CMD_CHAR, CREATE_NEW_CMD);
    send_msg_SERVER(ds_socket, buf);
    
    sprintf(buf, "%s%s : join ad un canale gia esistente\n", CMD_CHAR, JOIN_CMD);
    send_msg_SERVER(ds_socket, buf);
    
    sprintf(buf, "%s%s : elimina il canale (solo per amministratori)\n", CMD_CHAR, CLOSE_CMD);
    send_msg_SERVER(ds_socket, buf);

    sprintf(buf, "%s%s : lascia il canale \n", CMD_CHAR, LEAVE_CMD);
    send_msg_SERVER(ds_socket, buf);


}