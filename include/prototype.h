
#ifndef __METHODS_H__ // evitare inclusioni multiple di un header
#define __METHODS_H__

#include <sys/socket.h>
#include "utility_lib.h"

//server.c
void *chat_thread_function(void *arg);
void* broadcast_function(void *args);
void accept_connection(unsigned short port_number);

//client.c
void* receiveMessage(void* arg);
void* sendMessage(void* arg);
void chat_session(int socket_desc);

//coda_messaggi.c
void inizializza_coda();
void inserisci_in_coda(const char *username, const char *msg);
char* estrai_da_coda();

//send_receive.c
void send_message(int ds_socket, char* msg);
size_t receive_message(int ds_socket, char* msg, size_t msg_len);

//utility.c
void broadcast(char* msg);
int user_access(int ds_socket, const char* username, struct sockaddr_in* addr);
void close_socket(thread_arg_t* args);
void send_msg_SERVER(int ds_socket, char* msg);
void choise_message(int ds_socket);

//channel_routine.c
int new_channel_routine();
int join_channel_routine();
void leave_channel_routine();
void close_channel_routine();
void choise_rotuine(char* routine);


#endif
