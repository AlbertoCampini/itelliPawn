#include "lib/core.h"
#include "lib/strategy.h"
#include <unistd.h>

#define CONF_FILE_PATH "./config"

typedef struct {
    int points;
    int nMovesLeft;
    int nMovesDo;
} DataGamer;

static int idMatrix, idSemMatrix, idSemSyncRound, idMsgGamer, idSemGamer;

void endHandle(int signal) {
    removeSem(idSemSyncRound);
    removeSem(idSemMatrix);
    removeSem(idSemGamer);
    removeQueue(idMsgGamer);
    removeSHM(idMatrix);
    kill(-getpid(), SIGINT);
    printf("\nAttuata procedura di terminazione debug\n");
    exit(0);
}

int main() {
    srand(time(NULL));

    pid_t pidChild;
    int i, numFlags, posFlag, statusFork, totalPoints, firstRound, SO_NUM_G, SO_NUM_P, SO_BASE, SO_ALTEZZA, SO_FLAG_MIN, SO_FLAG_MAX, SO_N_MOVES, SO_MAX_TIME;
    char *argsToGamer[ARGS_TO_PASS_OF_GAMER];
    char bufferIdMsg[MAX_BUFF_SIZE], bufferIdSemGamer[MAX_BUFF_SIZE], bufferIdSemMatrix[MAX_BUFF_SIZE], bufferIdMatrix[MAX_BUFF_SIZE], bufferIdSemSyncRound[MAX_BUFF_SIZE];
    int *matrix;
    double totalTime, currentTime;
    ResultRound *dataGamer; /*Array per ogni Gamer che tiene conto dei dati statistici*/
    SyncGamer syncGamer; /*Invio al Gamer*/
    ResultRound resultRound;
    struct timespec tim;

    /*Inizializzo variabili per gestione Timer*/
    tim.tv_sec = 0;
    tim.tv_nsec = 100000;
    totalTime = 0;

    sigset_t maskSignal;
    struct sigaction signalAct;

    /*signalAct.sa_handler = endHandle;
    signalAct.sa_flags = 0;
    signalAct.sa_mask = maskSignal;
    if(sigaction(SIGUSR1, &signalAct, 0) < 0) {
        ERROR;
        return 0;
    }*/

    /*Imposto i segnali: blocco nella maschera SIGUSR1 verranno ereditati da gamer e pawn*/
     sigemptyset(&maskSignal);
     sigaddset(&maskSignal, SIGUSR1);
     if(sigprocmask(SIG_BLOCK, &maskSignal, NULL) < 0) {
         ERROR;
         return 0;
     }
     /*memset(&signalAct, 0, sizeof(signalAct));
     signalAct.sa_handler = endHandle;
     signalAct.sa_flags = 0;
     sigaddset(&maskSignal, SIGINT);
     signalAct.sa_mask = maskSignal;
     if(sigaction(SIGINT, &signalAct, 0) < 0) { ERROR; return 0; }*/

    /*Leggo dal file di config*/
    SO_NUM_G = readConfig("SO_NUM_G", HARD_MODE, CONF_FILE_PATH);
    if(SO_NUM_G < 0){ ERROR; return 0; }
    SO_NUM_P = readConfig("SO_NUM_P", HARD_MODE, CONF_FILE_PATH);
    if(SO_NUM_P < 0){ ERROR; return 0; }
    SO_BASE = readConfig("SO_BASE", HARD_MODE, CONF_FILE_PATH);
    if(SO_BASE < 0){ ERROR; return 0; }
    SO_ALTEZZA = readConfig("SO_ALTEZZA", HARD_MODE, CONF_FILE_PATH);
    if(SO_ALTEZZA < 0){ ERROR; return 0; }
    SO_FLAG_MIN = readConfig("SO_FLAG_MIN", HARD_MODE, CONF_FILE_PATH);
    if(SO_FLAG_MIN < 0){ ERROR; return 0; }
    SO_FLAG_MAX = readConfig("SO_FLAG_MAX", HARD_MODE, CONF_FILE_PATH);
    if(SO_FLAG_MAX < 0){ ERROR; return 0; }
    SO_N_MOVES = readConfig("SO_N_MOVES", HARD_MODE, CONF_FILE_PATH);
    if(SO_N_MOVES < 0){ ERROR; return 0; }
    SO_MAX_TIME = readConfig("SO_MAX_TIME", HARD_MODE, CONF_FILE_PATH);
    if(SO_MAX_TIME < 0){ ERROR; return 0; }
    firstRound = 1;

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
    idSemSyncRound = createAndInitSems(IPC_PRIVATE, 5, SO_NUM_G);/*SEM1: Gamer posiziona i Pawns*/
    if(!idSemSyncRound) {
        printf("Errore creazione semafori giocatori: ");ERROR;
        return 0;
    }
    if(!modifySem(idSemSyncRound, 1, -(SO_NUM_G - 1))) { ERROR; return 0; }/*SEM2: Master finisce le bandierine (inizializzato ad 1)*/
    if(!modifySem(idSemSyncRound, 2, (SO_NUM_G * SO_NUM_P)-SO_NUM_G)) { ERROR; return 0; }/*SEM3: Gamer fornisce strategie ai Pawns*/
    if(!modifySem(idSemSyncRound, 3, -(SO_NUM_G - 1))) { ERROR; return 0; }/*SEM4: Avvio round (inizializzato ad 1)*/


    /*INVOCO I GIOCATORI*/
    dataGamer = (ResultRound *)malloc(sizeof(ResultRound) * SO_NUM_G);
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
            setpgid(statusFork, getpid());
            (dataGamer + i)->points = 0;
            (dataGamer + i)->nMovesDo = 0;
            (dataGamer + i)->order = i;
            (dataGamer + i)->nMovesLeft = SO_N_MOVES * SO_NUM_P;

            syncGamer.order = i;
            syncGamer.strategy = 0;
            syncGamer.name = i + 1;
            syncGamer.points = (dataGamer + i)->points;
            if(!sendMessageToGamer(idMsgGamer, statusFork, syncGamer)) {
                printf("Errore send messaggio: ");
                ERROR;
            }
        }
    }

    /*POSIZIONO LE BANDIERINE*/
    /*1) Attendo che tutti i giocatori posizionino le pedine*/
    if(!waitSem(idSemSyncRound, 0)) {ERROR; return 0;}

    do {
        /*2) Posiziono le bandierine*/
        numFlags = generateRandom(SO_FLAG_MIN, SO_FLAG_MAX);
        printf("Ho posizionato %d flags tra %d e %d (il mio PID e' %d)\n", numFlags, SO_FLAG_MIN, SO_FLAG_MAX, getpid());

        for(i = 0; i < numFlags; i++) {
            posFlag = flagPositionStrategy(POS_STRATEGY_RANDOM, idSemMatrix, SO_BASE, SO_ALTEZZA);
            while(*(matrix + posFlag) < 0) {
                posFlag = flagPositionStrategy(POS_STRATEGY_RANDOM, idSemMatrix, SO_BASE, SO_ALTEZZA);
            }
            *(matrix + posFlag) = ((i + 1) * -1);
        }
        /*SEM5: Semaforo per sapere quando sono state prese tutte le flags (è inizializzato al numero di bandierine)*/
        if(!modifySem(idSemSyncRound, 4, (-(SO_NUM_G * firstRound) + numFlags))) { ERROR; return 0; }

        /*Stampo la matrix e le metriche del punto 1.6*/
        for(i = 0; i < SO_NUM_G; i++) {
            printf("--Giocatore %d: punteggio %d, mosse residue %d\n", (dataGamer + i)->order + 1, (dataGamer + i)->points, (dataGamer + i)->nMovesLeft);
        }
        printMatrix(matrix, SO_BASE, SO_ALTEZZA);

        printf("Mi preparo ad iniziare il i round\n");
        currentTime = 0;
        /*3) Dichiaro che ho posizionato le bandierine ai Gamer --> SEM2*/
        if (!modifySem(idSemSyncRound, 1, -1)) {
            ERROR;
            return 0;
        }

        /*4) Attendo che i Gamer forniscano la strategia ai Pawns*/
        if (!waitSem(idSemSyncRound, 2)) {
            ERROR;
            return 0;
        }

        /*5) Avvio il round: avvio il Timer*/
        statusFork = fork();
        switch (statusFork) {
            case -1:
                printf("Impossibile avviare il Timer\n");
                break;
            case 0:
                /*Timer: aspetto SO_MAX_TIME e poi lancio il segale*/
                if (!waitSem(idSemSyncRound, 3)) {
                    ERROR;
                    return 0;
                }
                printf("Il Timer è partito...\n\n");
                /*Calcolo il tempo senza sleep() salvando quanto è passato da inizio round*/
                while (!waitSemWithoutWait(idSemSyncRound, 4) && (currentTime) < (SO_MAX_TIME)) {
                    currentTime += 0.0001;
                    nanosleep(&tim, NULL);
                }

                if (!waitSemWithoutWait(idSemSyncRound, 4)) {
                    printf(RED);
                    printf("--TIMEOUT--\n");
                    printf(RESET_COLOR);
                    if (kill(0, SIGUSR1) < 0) {
                        printf(" - Errore TIMER: ");
                        ERROR;
                    }
                } else {
                    printf("Round terminato in %lf secondi\n", currentTime);
                }
                exit(0);
                break;
            default:
                /*Master: avvio il round*/
                if (!modifySem(idSemSyncRound, 3, -1)) {
                    ERROR;
                    return 0;
                }

                /*Attendo di ricevere SO_NUM_G messaggi di resoconto del round*/
                printf("\n");
                for (i = 0; i < SO_NUM_G; i++) {
                    if (!receiveMessageResultRound(idMsgGamer, 1, &resultRound)) { printf("master\n"); }
                    else {
                        printf("--Giocatore %d [points: %d, nMovesLeft: %d, nMovesDo: %d]\n", resultRound.order + 1, resultRound.points, resultRound.nMovesLeft, resultRound.nMovesDo);
                        dataGamer[resultRound.order].nMovesLeft = resultRound.nMovesLeft;
                        dataGamer[resultRound.order].nMovesDo = resultRound.nMovesDo;
                        dataGamer[resultRound.order].points = resultRound.points;
                    }
                }

                /*Se scatta il timer e non ho preso tutte le bandierine stampo lo stato del semaforo 4 (quante bandierine mancano)*/
                if (!waitSemWithoutWait(idSemSyncRound, 4)) {
                    //printf("\nValore tornato dal timer: %d\n", status);
                    //totalTime += currentTime;
                    printf(GREEN);
                    printf("Gioco terminato:\n");
                    printf("\t\t%d flags mancanti\n", getValueOfSem(idSemSyncRound, 4));
                    printf("\t\t1 round giocati\n\n");
                    totalPoints = 0;
                    for(i = 0; i < SO_NUM_G; i++) {
                        printf("\t\t[Giocatore %d]:\n", i + 1);
                        printf("\t\t\t\tMosse fatte / mosse totali: %lf\n", ((double)dataGamer[i].nMovesDo / (double)(SO_NUM_P * SO_N_MOVES)));
                        printf("\t\t\t\tPunti ottenuti / mosse fatte: %lf\n", ((double)dataGamer[i].points / (double)dataGamer[i].nMovesDo));
                        totalPoints += dataGamer[i].points;
                    }
                    printf("\t\tPunti totali / tempo di gioco totale: %lf\n", (totalPoints / totalTime));
                    printf(RESET_COLOR);

                    /*Fine gioco*/
                    while ((pidChild = wait(NULL)) != -1) {
                    }

                    free(dataGamer);
                    removeSem(idSemSyncRound);
                    removeSem(idSemMatrix);
                    removeSem(idSemGamer);
                    removeQueue(idMsgGamer);
                    removeSHM(idMatrix);
                } else {
                    printf("\nTutte le %d bandierine sono state prese!\n", numFlags - getValueOfSem(idSemSyncRound, 4));

                    /*Reset di tutti i semafori e variabili*/
                    firstRound = 0;
                    if(!modifySem(idSemSyncRound, 1, 1)) { ERROR; return 0; }/*SEM2: Master finisce le bandierine (inizializzato ad 1)*/
                    if(!modifySem(idSemSyncRound, 2, (SO_NUM_G * SO_NUM_P))) { ERROR; return 0; }/*SEM3: Gamer fornisce strategie ai Pawns*/
                    if(!modifySem(idSemSyncRound, 3, 1)) { ERROR; return 0; }/*SEM4: Avvio round (inizializzato ad 1)*/
                    printf("Reset dei semafori: %d, %d, %d", getValueOfSem(idSemSyncRound, 1), getValueOfSem(idSemSyncRound, 2), getValueOfSem(idSemSyncRound, 3));
                }

                printMatrix(matrix, SO_BASE, SO_ALTEZZA);
                break;
        }
    } while(waitSemWithoutWait(idSemSyncRound, 4));

    return 0;
}
