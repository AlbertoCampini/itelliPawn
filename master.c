#include "lib/core.h"
#include <unistd.h>

#define CONF_FILE_PATH "./config"
#define ARGS_TO_PASS_OF_GAMER 6

int main() {
    srand(time(NULL));

    pid_t pidChild;
    int i, idMatrix, idMsgGamer, idSemGamer, idSemMatrix, statusFork, SO_NUM_G, SO_BASE, SO_ALTEZZA;
    char *argsToGamer[ARGS_TO_PASS_OF_GAMER];
    char bufferIdMsg[MAX_BUFF_SIZE], bufferIdSemGamer[MAX_BUFF_SIZE], bufferIdSemMatrix[MAX_BUFF_SIZE], bufferIdMatrix[MAX_BUFF_SIZE];
    int *matrix;

    /*Leggo dal file di config*/
    SO_NUM_G = readConfig("SO_NUM_G", HARD_MODE, CONF_FILE_PATH);
    if(SO_NUM_G < 0){ ERROR; return 0; }
    SO_BASE = readConfig("SO_BASE", HARD_MODE, CONF_FILE_PATH);
    if(SO_BASE < 0){ ERROR; return 0; }
    SO_ALTEZZA = readConfig("SO_ALTEZZA", HARD_MODE, CONF_FILE_PATH);
    if(SO_ALTEZZA < 0){ ERROR; return 0; }

    /*Ottengo il puntatore alla mem condivisa di dimensione SO_ALTEZZA * SO_BASE*/
    idMatrix = createSHM(IPC_PRIVATE, (SO_ALTEZZA * SO_BASE) * sizeof(int));
    if(!idMatrix) {
        printf("Errore creazione Matrix: ");ERROR;
        return 0;
    }
    matrix = (int *)attachSHM(idMatrix);
    if(matrix == (void *)-1) {
        printf("Errore attach Matrix: ");ERROR;
        return 0;
    }
    /*Inizializzo la Matrix tutta a 0*/
    initSHM(matrix, SO_BASE, SO_ALTEZZA, 0);

    /*Istanzio il set di semafori per il Matrix*/
    idSemMatrix = createAndInitSems(IPC_PRIVATE, (SO_BASE * SO_ALTEZZA), 0);
    if(!idSemMatrix) {
        printf("Errore creazione semafori matrix: ");ERROR;
        return 0;
    }

    /*Istanzio la coda di messaggi privata tra Master e Gamer*/
    idMsgGamer = createMsgQueue(IPC_PRIVATE);
    if(!idMsgGamer) {
        printf("Errore creazione coda: ");ERROR;
        return 0;
    }

    /*Istanzio il set di semafori per la fase di posizionamento pedine*/
    idSemGamer = createAndInitSems(IPC_PRIVATE, SO_NUM_G, 1);
    if(!idSemGamer) {
        printf("Errore creazione semafori giocatori: ");ERROR;
        return 0;
    }
    /*Prendo un semaforo random nel set e lo "rilascio" con 0*/
    if(!modifySem(idSemGamer, generateRandom(0, SO_NUM_G - 1), -1)) {
        printf("Errore set semaforo a 0: ");ERROR;
        return 0;
    }

    /*INVOCO I GIOCATORI*/
    for(i = 0; i < SO_NUM_G; i++) {
        statusFork = fork();
        if(statusFork == 0) {
            sprintf(bufferIdMsg, "%d", idMsgGamer);
            sprintf(bufferIdSemGamer, "%d", idSemGamer);
            sprintf(bufferIdSemMatrix, "%d", idSemMatrix);
            sprintf(bufferIdMatrix, "%d", idMatrix);
            argsToGamer[0] = NAME_GAMER_PROCESS;
            argsToGamer[1] = bufferIdMsg;
            argsToGamer[2] = bufferIdSemGamer;
            argsToGamer[3] = bufferIdSemMatrix;
            argsToGamer[4] = bufferIdMatrix;
            argsToGamer[5] = NULL;
            execve(NAME_GAMER_PROCESS, argsToGamer, NULL);
            break;
        } else if(statusFork == -1){
            printf("Errore durante la generazione del fork\n");
            printLastError();
            break;
        } else {
            SyncGamer syncGamer;
            syncGamer.order = i;
            syncGamer.strategy = 0;
            syncGamer.name = i + 1;
            if(!sendMessage(idMsgGamer, statusFork, syncGamer)) {
                printf("Errore send messaggio: ");
                printLastError();
            }
        }
    }

    while((pidChild = wait(NULL)) != -1) {
    }

    printMatrix(matrix, SO_BASE, SO_ALTEZZA);

    removeSem(idSemMatrix);
    removeSem(idSemGamer);
    removeQueue(idMsgGamer);
    removeSHM(idMatrix);
    return 0;
}
