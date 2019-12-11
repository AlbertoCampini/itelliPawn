#include "core/core.h"
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define CONF_FILE_PATH "./config"

int main() {
    srand(time(NULL));

    pid_t pidChild;
    int i, shm_id, idMsgGamer, idSemGamer, statusFork, SO_NUM_G, SO_BASE, SO_ALTEZZA;
    char *args[4];
    char bufferIdMsg[MAX_BUFF_SIZE], bufferIdSem[MAX_BUFF_SIZE];

    /*Leggo dal file di config*/
    SO_NUM_G = readConfig("SO_NUM_G", HARD_MODE, CONF_FILE_PATH);
    if(SO_NUM_G < 0){ ERROR; return 0; }
    SO_BASE = readConfig("SO_BASE", EASY_MODE, CONF_FILE_PATH);
    if(SO_BASE < 0){ ERROR; return 0; }
    SO_ALTEZZA = readConfig("SO_ALTEZZA", EASY_MODE, CONF_FILE_PATH);
    if(SO_ALTEZZA < 0){ ERROR; return 0; }

    /*ottengo il puntatore alla mem condivisa di dimensione SO_ALTEZZA * SO_BASE*/
    /*shm_id = shmget(IPC_PRIVATE, SO_ALTEZZA*SO_BASE*sizeof(int), IPC_CREAT | 0666);*/

    /*Istanzio la coda di messaggi privata tra Master e Gamer*/
    idMsgGamer = createMsgQueue(IPC_PRIVATE);
    if(!idMsgGamer) {
        printf("Errore creazione coda: ");ERROR;
        return 0;
    }

    /*Istanzio il set di semafori per la fase di posizionamento pedine*/
    idSemGamer = createAndInitSems(IPC_PRIVATE, SO_NUM_G, 1);
    if(!idSemGamer) {
        printf("Errore creazione semafori: ");ERROR;
        return 0;
    }
    /*Prendo un semaforo random nel set e lo "rilascio" con 0*/
    if(!modifySem(idSemGamer, generateRandom(0, SO_NUM_G - 1), -1)) {
        printf("Errore set semaforo a 0: ");ERROR;
        return 0;
    }

    for(i = 0; i < SO_NUM_G; i++) {
        statusFork = fork();
        if(statusFork == 0) {
            sprintf(bufferIdMsg, "%d", idMsgGamer);
            sprintf(bufferIdSem, "%d", idSemGamer);
            args[0] = NAME_GAMER_PROCESS;
            args[1] = bufferIdMsg;
            args[2] = bufferIdSem;
            args[3] = NULL;
            execve(NAME_GAMER_PROCESS, args, NULL);
            break;
        } else if(statusFork == -1){
            printf("Errore durante la generazione del fork\n");
            printLastError();
            break;
        } else {
            SyncGamer syncGamer;
            syncGamer.order = i;
            syncGamer.strategy = "Strategia";
            if(!sendMessage(idMsgGamer, statusFork, syncGamer)) {
                printf("Errore send messaggio: ");
                printLastError();
            }
        }
    }

    while((pidChild = wait(NULL)) != -1) {
    }

    removeSem(idSemGamer);
    removeQueue(idMsgGamer);
    return 0;
}
