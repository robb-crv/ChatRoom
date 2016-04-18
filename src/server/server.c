
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

user_info_t* users[MAX_CLIENT]; 
unsigned int current_users;    
sem_t user_data_sem;


//thread che gestisce la connessione in entrata
void *chat_thread_function(void *arg) {

    //allocare struttura dati per i canali 
    
    int ret;
    thread_arg_t* args = (thread_arg_t*) arg;

    char msg[SIZE_MSG];
    char username[USERNAME_SIZE];
    char error_msg[SIZE_MSG];

    //ricezione username utente
    size_t user_join_msg_len= receive_message(args->sock, username, SIZE_MSG);
    
    if(user_join_msg_len < 0) close_socket(args);

    //username ottenuto, viene effettuata la registrazione utente nella struttura dati del server 
    ret=user_access(args->sock, username, args->addr);
    
    if(ret==ERR_MAX_USERS) {
        
        //invio messaggio chiusura all'utente e chiusura descrittori 
        sprintf(error_msg, "Raggiunto limite massimo di connessioni: (%d)\n", MAX_CLIENT);
        send_msg_SERVER(args->sock, error_msg);
        close_socket(args);
    }

    if(ret==ERR_USERNAME_NOT_AVAILABLE){
        
        //invio messaggio chiusura all'utente e chiusura descrittori  
        sprintf(error_msg, "Username non disponibile\n");
        send_msg_SERVER(args->sock, error_msg);
        close_socket(args);
    }

    //viene mostrato al client il messaggio di benvenuto e le possibili scelte NEW-JOIN-LEAVE-DELETE
    sprintf(error_msg, "Benvenuto nella chat (%s)!\n", username);
    send_msg_SERVER(args->sock, error_msg);

    //SCELTA OPERAZIONE DA EFFETTUARE NEW-JOIN-LEAVE-CLOSE   
    choise_message(args->sock);

    size_t len=receive_message(args->sock, msg, SIZE_MSG);
    if(len<0){
        close_socket(args);
    }

    int flag=0; 
    do {
        if(msg[0]==CMD_CHAR){
            if(strcmp(msg+1, CREATE_NEW_CMD)==0){
                /*
                * si apre nuovo canale con un thread e si aggiorna la struttura dati
                * aggiungengo il canale alla lista dei canali disponibile nella chatroom 
                * aggiungiamo il canale alla lista dei canali a cui il client partecipa 
                */ 
            }else if (strcmp(msg+1, JOIN_CMD)==0){
                //
            }else if(strcmp(msg+1, LEAVE_CMD)==0){
                // lasciamo il canale e riportiamo l'utente in chat_thread_function 
                //mostrandogli di nuovo i comandi in modo che può rimanere in chat e
                //e ricominciare a fare quello che vuole
            }else  if(strcmp(msg+1, CLOSE_CMD)==0){
                //chiudiamo la connessione dell'utente con il server
            }else {
                sprintf(error_msg, "Comando errato!");
                send_msg_SERVER(args->sock, error_msg);
                choise_message(args->sock);
            }
        }else {
            sprintf(error_msg, "Comando errato!");
            send_msg_SERVER(args->sock, error_msg);
            choise_message(args->sock);
        }

    }while(!flag);


    //return NULL;
    
    //DA ELIMINARE
    pthread_exit(NULL);
}




void* broadcast_function(void *args) {
    while (1) {
        msg_t *msg = estrai_da_coda();
        broadcast(msg);
        free(msg);
    }
}



//accetta connessioni in ingresso 
void accept_connection(unsigned short port_number) {

	int ds_server, ds_client, ret;

	struct sockaddr_in server_addr={0};
    //usiamo sockaddr_len come parametro per l'accept
    int sockaddr_len= sizeof(struct sockaddr_in);

	ds_server=socket(AF_INET, SOCK_STREAM,0); 
    GESTIONE_ERRORE(ds_server, "Errore creazione socket ds_server -- server\n");

	//inizializzazione struttura indirizzo
	server_addr.sin_family= AF_INET;
	server_addr.sin_port=port_number;
	server_addr.sin_addr.s_addr=INADDR_ANY;
    
	//binding dell'indirizzo
	ret=bind(ds_server, (struct sockaddr*) &server_addr, sockaddr_len);
    GESTIONE_ERRORE(ret, "Errore binding su ds_server --  server\n");
    
	ret=listen(ds_server, QUEUE_LEN);
    GESTIONE_ERRORE(ret, "Errore listen su ds_socket --  server\n");
    

    struct sockaddr_in* client_addr= calloc(1, sizeof(struct sockaddr_in));

	//ciclo accettazione connessioni in ingresso 
	while(1) {
        
        ds_client = accept(ds_server, (struct sockaddr*)client_addr, (socklen_t*) &sockaddr_len);
        if (ds_client == -1 && errno == EINTR) continue;
        GESTIONE_ERRORE(ds_client, "Errore accept su ds_socket\n");

        
        thread_arg_t * args =(thread_arg_t*) malloc(sizeof(thread_arg_t));
        args-> sock= ds_client;
        args-> addr= client_addr;
        
        pthread_t thread_i;
        ret=pthread_create(&thread_i, NULL, chat_thread_function, args);
        GESTIONE_ERRORE_THREAD(ret, "Errore creazione thread\n");

        ret = pthread_detach(thread_i);
        GESTIONE_ERRORE_THREAD(ret, "Errore detach thread\n");

        //riallocazione struttura per la prossima connessione in ingresso
        client_addr = calloc(1, sizeof(struct sockaddr_in));
	}
    
}






int main (int argc, char* argv[]) {

	if(argc==2) {

	long p_numb= strtol(argv[1], NULL, 0); //converte stringa argv[1] in long
	if (p_numb < 1024 || p_numb > 49151) {
        fprintf(stderr, "Errore: usare port_number tra 1024 e 49151.\n");
        exit(EXIT_FAILURE);
    }
    unsigned short port_number= htons((unsigned short) p_numb); //port number -> unsigned short
    

    accept_connection(port_number);

    exit(EXIT_SUCCESS);

    
    } else {
        printf("**Errore sintassi** ./server <numero di porta>\n");  
    }
     return 0;

}
