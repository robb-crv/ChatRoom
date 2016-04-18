#include <stdio.h>
#include <semaphore.h>

#include "utility_lib.h"
#include "prototype.h"

#define BUFFER_SIZE 256

 
msg_t* buffer_FIFO[BUFFER_SIZE]; // buffer circolare una coda FIFO
int read_index; //indice lettura buffer
int write_index; //indice scrittura buffer 

sem_t s_empty; //posizioni occupate nel buffer
sem_t s_fill; //posizioni libere nel buffer
sem_t s_write; //semaforo per scrittura


void inizializza_coda() {
    
    int ret;

    //s_empty=BUFFER_SIZE
    ret = sem_init(&s_empty, 0, BUFFER_SIZE);
    GESTIONE_ERRORE(ret, "Errore sem_init empty_sem\n");

    //s_fill=0
    ret = sem_init(&s_fill, 0, 0);
    GESTIONE_ERRORE(ret, "Errore sem_init fill_sem\n");
    //s_write=1 semaforo binario
    ret = sem_init(&s_write, 0, 1);
    GESTIONE_ERRORE(ret, "Errore sem_init write_sem\n");
}


void inserisci_in_coda(const char* username, const char *msg) {

    int ret;
    msg_t* msg_buf= (msg_t*) malloc(sizeof(msg_t));
    sprintf(msg_buf->username, "%s", username);
    sprintf(msg_buf-> msg, "%s", msg);
    
    ret = sem_wait(&s_empty);
    GESTIONE_ERRORE(ret, "Errore sem_wait s_empty\n");

    ret = sem_wait(&s_write);
    GESTIONE_ERRORE(ret, "Errore sem_wait s_write\n");

    buffer_FIFO[write_index] = msg_buf;
    write_index = (write_index + 1) % BUFFER_SIZE; //buffer circolare

    ret = sem_post(&s_write);
    GESTIONE_ERRORE(ret, "Errore sem_post S_write\n");

    ret = sem_post(&s_fill);
    GESTIONE_ERRORE(ret, "Errore sem_post s_fill\n");

}


msg_t* estrai_da_coda() {

    int ret;
    msg_t *msg = NULL;

    ret = sem_wait(&s_fill);
    GESTIONE_ERRORE(ret, "Errore sem_wait s_fill\n");

    msg = buffer_FIFO[read_index];
    read_index = (read_index + 1) % BUFFER_SIZE;

    ret = sem_post(&s_empty);
    GESTIONE_ERRORE(ret, "Errore sem_post S_empty\n");

    return msg;
}
