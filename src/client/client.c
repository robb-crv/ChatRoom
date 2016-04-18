
#include <stdio.h>
#include <stdlib.h> //per utilizzo strtol()
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../../utility_lib.h"
//#include "../utility.c"

/*

int shouldStop = 0;

void* receiveMessage(void* arg) {
    int socket_desc = (int)arg;

    char close_command[SIZE_MSG];
    sprintf(close_command,"%c%s", CMD_CHAR, QUIT_COMMAND);
    size_t close_command_len = strlen(close_command);

    struct timeval timeout;
    fd_set read_descriptors;
    int nfds = socket_desc + 1;

    char buf[MSG_SIZE];
    char nickname[NICKNAME_SIZE];
    char delimiter[2];
    sprintf(delimiter, "%c", MSG_DELIMITER_CHAR);

    while (!shouldStop) {
        int ret;

        // check every 1.5 seconds (why not longer?)
        timeout.tv_sec  = 1;
        timeout.tv_usec = 500000;

        FD_ZERO(&read_descriptors);
        FD_SET(socket_desc, &read_descriptors);

        // perform select() 
        ret = select(nfds, &read_descriptors, NULL, NULL, &timeout);

        if (ret == -1 && errno == EINTR) continue;
        ERROR_HELPER(ret, "Unable to select()");

        if (ret == 0) continue; // timeout expired

        // ret is 1: read available data!
        int read_completed = 0;
        int read_bytes = 0; // index for writing into the buffer
        int bytes_left = MSG_SIZE - 1; // number of bytes to (possibly) read
        while(!read_completed) {
            ret = recv(socket_desc, buf + read_bytes, 1, 0);
            if (ret == 0) break;
            if (ret == -1 && errno == EINTR) continue;
            ERROR_HELPER(ret, "Errore nella lettura da socket");
            bytes_left -= ret;
            read_bytes += ret;
            read_completed = bytes_left == 0 || buf[read_bytes - 1] == '\n';
        }

        if (ret == 0) {
            shouldStop = 1;
        } else {
            buf[read_bytes - 1] = '\0';
            int sender_length = strcspn(buf, delimiter);
            if (sender_length == strlen(buf)) {
                printf("[???] %s\n", buf);
            } else {
                snprintf(nickname, sender_length + 1, "%s", buf);
                printf("[%s] %s\n", nickname, buf + sender_length + 1);
            }
        }
    }

    pthread_exit(NULL);
}



void* sendMessage(void* arg) {
    int socket_desc = (int)arg;

    char close_command[MSG_SIZE];
    sprintf(close_command, "%c%s\n", COMMAND_CHAR, QUIT_COMMAND);
    size_t close_command_len = strlen(close_command);

    char buf[MSG_SIZE];

    while (!shouldStop) {
        int ret;

        // Read a line from stdin: fgets() reads up to sizeof(buf)-1
        // bytes and on success returns the first argument passed. 
        if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
            fprintf(stderr, "Error while reading from stdin, exiting...\n");
            exit(EXIT_FAILURE);
        }

        if (shouldStop) break; // the endpoint might have closed the connection

        size_t msg_len = strlen(buf);

        // send message
        while ( (ret = send(socket_desc, buf, msg_len, 0)) < 0) {
            if (errno == EINTR) continue;
            ERROR_HELPER(-1, "Cannot write to socket");
        }

        // After a BYE command we should update shouldStop
        if (msg_len == close_command_len && !memcmp(buf, close_command, close_command_len)) {
            shouldStop = 1;
        }
    }

    pthread_exit(NULL);
}


*/

void chat_session(int socket_desc) {
    int ret;

    pthread_t chat_threads[2];

    ret = pthread_create(&chat_threads[0], NULL, receiveMessage, (void*)socket_desc);
    GENERIC_ERROR_HELPER(ret, ret, "Cannot create thread for receiving messages");

    ret = pthread_create(&chat_threads[1], NULL, sendMessage, (void*)socket_desc);
    GENERIC_ERROR_HELPER(ret, ret, "Cannot create thread for sending messages");

    // wait for termination
    ret = pthread_join(chat_threads[0], NULL);
    GENERIC_ERROR_HELPER(ret, ret, "Cannot join on thread for receiving messages");

    ret = pthread_join(chat_threads[1], NULL);
    GENERIC_ERROR_HELPER(ret, ret, "Cannot join on thread for sending messages");

    // close socket
    ret = close(socket_desc);
    ERROR_HELPER(ret, "Cannot close socket");
}




int main(int argc, char* argv[]) {

    if (argc==4) {

        //<./client> <IP_address> <port_number> <username>
        int ret;
        int ds_socket;
        struct sockaddr_in server_addr = {0};
        
        //network byte order
        in_addr_t ip_addr;
        
        ip_addr = inet_addr(argv[1]); 
        
        unsigned short tmp = strtol(argv[2], NULL, 0); 
        unsigned short port_number = htons((unsigned short)tmp);
        
        char username[USERNAME_SIZE];
        sprintf(username, "%s", argv[3]);

        ds_socket = socket(AF_INET, SOCK_STREAM, 0);
        GESTIONE_ERRORE(ds_socket, "Errore apertura socket -- client\n");
        
        server_addr.sin_addr.s_addr = ip_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = port_number;

        ret = connect(ds_socket, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
        GESTIONE_ERRORE(ret, "Errore connect\n");

        //invio il nickname preso in input dall'utente
        //il server lo legge e avvia la procedura di gestione
        char msg[SIZE_MSG];
        sprintf(msg, "%s", argv[3]);
        int len = strlen(msg);
        while ( (ret = send(ds_socket, msg, len, 0)) < 0) {
            if (errno == EINTR) continue;
            GESTIONE_ERRORE(-1, "Errore send su ds_sock\n");
        }   

        
        chat_session(ds_socket);
        

    }else printf("**Errore sintassi** ./client -IP address-,  -port_number-\n");

    exit(EXIT_SUCCESS);
        
}
