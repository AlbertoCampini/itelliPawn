#include "lib/core.h"
#include "lib/strategy.h"
#include <unistd.h>

#define CONF_FILE_PATH "./config"

int main() {
    srand(time(NULL));

    pid_t pidChild;
    int i, numFlags, idMatrix, idMsgGamer, idSemGamer, idSemMatrix, idSemSyncRound, statusFork, SO_NUM_G, SO_BASE, SO_ALTEZZA, SO_FLAG_MIN, SO_FLAG_MAX;
    char *argsToGamer[ARGS_TO_PASS_OF_GAMER];
    char bufferIdMsg[MAX_BUFF_SIZE], bufferIdSemGamer[MAX_BUFF_SIZE], bufferIdSemMatrix[MAX_BUFF_SIZE], bufferIdMatrix[MAX_BUFF_SIZE], bufferIdSemSyncRound[MAX_BUFF_SIZE];
    int *matrix;

    /*Leggo dal file di config*/
    SO_NUM_G = readConfig("SO_NUM_G", HARD_MODE, CONF_FILE_PATH);
    if(SO_NUM_G < 0){ ERROR; return 0; }
    SO_BASE = readConfig("SO_BASE", HARD_MODE, CONF_FILE_PATH);
    if(SO_BASE < 0){ ERROR; return 0; }
    SO_ALTEZZA = readConfig("SO_ALTEZZA", HARD_MODE, CONF_FILE_PATH);
    if(SO_ALTEZZA < 0){ ERROR; return 0; }
    SO_FLAG_MIN = readConfig("SO_FLAG_MIN", HARD_MODE, CONF_FILE_PATH);
    if(SO_FLAG_MIN < 0){ ERROR; return 0; }
    SO_FLAG_MAX = readConfig("SO_FLAG_MAX", HARD_MODE, CONF_FILE_PATH);
    if(SO_FLAG_MAX < 0){ ERROR; return 0; }


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

    /*Istanzio il set di semafori per aspettare che i Gamer finiscono la fase di posizionamento pedine*/
    idSemSyncRound = createAndInitSems(IPC_PRIVATE, 4, SO_NUM_G);/*SEM1: Gamer posiziona i Pawns*/
    if(!idSemSyncRound) {
        printf("Errore creazione semafori giocatori: ");ERROR;
        return 0;
    }
    if(!modifySem(idSemSyncRound, 1, -(SO_NUM_G - 1))) { ERROR; return 0; }/*SEM2: Master finisce le bandierine*/
    if(!modifySem(idSemSyncRound, 2, -SO_NUM_G)) { ERROR; return 0; }/*SEM3: Gamer fornisce strategie ai Pawns*/
    if(!modifySem(idSemSyncRound, 3, -SO_NUM_G)) { ERROR; return 0; }/*SEM4: Avvio round*/

    /*INVOCO I GIOCATORI*/
    for(i = 0; i < SO_NUM_G; i++) {
        statusFork = fork();
        if(statusFork == 0) {
            sprintf(bufferIdMsg, "%d", idMsgGamer);
            sprintf(bufferIdSemGamer, "%d", idSemGamer);
            sprintf(bufferIdSemMatrix, "%d", idSemMatrix);
            sprintf(bufferIdMatrix, "%d", idMatrix);
            sprintf(bufferIdSemSyncRound, "%d", idSemSyncRound);
            argsToGamer[0] = NAME_GAMER_PROCESS;
            argsToGamer[1] = bufferIdMsg;
            argsToGamer[2] = bufferIdSemGamer;
            argsToGamer[3] = bufferIdSemMatrix;
            argsToGamer[4] = bufferIdMatrix;
            argsToGamer[5] = bufferIdSemSyncRound;
            argsToGamer[6] = NULL;
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
            if(!sendMessageToGamer(idMsgGamer, statusFork, syncGamer)) {
                printf("Errore send messaggio: ");
                ERROR;
            }
        }
    }

    /*POSIZIONO LE BANDIERINE*/
    /*1) Attendo che tutti i giocatori posizionino le pedine*/
    if(!waitSem(idSemSyncRound, 0)) {ERROR; return 0;}

    /*2) Posiziono le bandierine*/
    numFlags = generateRandom(SO_FLAG_MIN, SO_FLAG_MAX);
    for(i = 0; i < numFlags; i++) {
        *(matrix + positionStrategy(POS_STRATEGY_RANDOM, idSemMatrix, SO_BASE, SO_ALTEZZA)) = -1;
    }

    /*3) Dichiaro che ho posizionato le bandierine ai Gamer --> SEM2*/
    if(!modifySem(idSemSyncRound, 1, -1)) { ERROR; return 0; }
    /*4) Attendo che i Gamer forniscano la strategia ai Pawns*/
    /*5) Avvio il round*/


    while((pidChild = wait(NULL)) != -1) {
    }

    printMatrix(matrix, SO_BASE, SO_ALTEZZA);

    removeSem(idSemSyncRound);
    removeSem(idSemMatrix);
    removeSem(idSemGamer);
    removeQueue(idMsgGamer);
    removeSHM(idMatrix);
    return 0;
}
