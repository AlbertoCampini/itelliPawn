#include "lib/core.h"
#include "lib/strategy.h"
#include "lib/graphics.h"
#include <unistd.h>

#define CONF_FILE_PATH "./config"

static int idMatrix, idSemMatrix, idSemSyncRound, idMsgGamer, idSemGamer;

void endHandle(int signal) {
    removeSem(idSemSyncRound);
    removeSem(idSemMatrix);
    removeSem(idSemGamer);
    removeQueue(idMsgGamer);
    removeSHM(idMatrix);
    kill(0, SIGUSR1);
    printf("\nAttuata procedura di terminazione debug\n");
    exit(0);
}

int main() {
    pid_t pidChild;
    int i, numFlags, numRound, posFlag, statusFork, totalPoints, firstRound, idTotalTime, actualFlagsPoint, gamerWin, menuChoise, SO_NUM_G, SO_NUM_P, SO_BASE, SO_ALTEZZA, SO_FLAG_MIN, SO_FLAG_MAX, SO_N_MOVES, SO_MAX_TIME, SO_ROUND_SCORE, FLAG_STRATEGY;
    char *argsToGamer[ARGS_TO_PASS_OF_GAMER];
    char bufferIdMsg[MAX_BUFF_SIZE], bufferIdSemGamer[MAX_BUFF_SIZE], bufferIdSemMatrix[MAX_BUFF_SIZE], bufferIdMatrix[MAX_BUFF_SIZE], bufferIdSemSyncRound[MAX_BUFF_SIZE], bufferMenuChoise[MAX_BUFF_SIZE];
    int *matrix;
    time_t startTime, endTime;
    float *totalTime;
    float timeLeft;

    ResultRound *dataGamer; /*Array per ogni Gamer che tiene conto dei dati statistici*/
    SyncGamer syncGamer; /*Invio al Gamer*/
    ResultRound resultRound;
    struct timespec tim;

    sigset_t maskSignal;
    struct sigaction signalAct;

    /*Inizializzo variabili per gestione Timer*/
    tim.tv_sec = 0;
    tim.tv_nsec = 10000000;
    totalTime = 0;

    /*Inizializzo la radice randomica*/
    srand(time(NULL));

    /*Menu*/
    /*Menu*/
    do {
        system("clear");
        printTitle();
        printf("\t1) Easy mode\n");
        printf("\t2) Hard mode\n");
        printf("\t> ");
        scanf("%d", &menuChoise);
    } while(menuChoise != 1 && menuChoise != 2);
    system("clear");

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
     if(sigprocmask(SIG_BLOCK, &maskSignal, NULL) < 0) { ERROR; return 0; }
     /*Setto il segnale per la procedura di terminazione (CRTL-C)*/
     memset(&signalAct, 0, sizeof(signalAct));
     signalAct.sa_handler = endHandle;
     signalAct.sa_flags = 0;
     sigaddset(&maskSignal, SIGINT);
     signalAct.sa_mask = maskSignal;
     if(sigaction(SIGINT, &signalAct, 0) < 0) { ERROR; return 0; }

    /*Leggo dal file di config*/
    SO_NUM_G = readConfig("SO_NUM_G", CONF_FILE_PATH, menuChoise);
    if(SO_NUM_G <= 0){ ERROR; return 0; }
    SO_NUM_P = readConfig("SO_NUM_P", CONF_FILE_PATH, menuChoise);
    if(SO_NUM_P <= 0){ ERROR; return 0; }
    SO_BASE = readConfig("SO_BASE", CONF_FILE_PATH, menuChoise);
    if(SO_BASE <= 0){ ERROR; return 0; }
    SO_ALTEZZA = readConfig("SO_ALTEZZA", CONF_FILE_PATH, menuChoise);
    if(SO_ALTEZZA <= 0){ ERROR; return 0; }
    SO_FLAG_MIN = readConfig("SO_FLAG_MIN", CONF_FILE_PATH, menuChoise);
    if(SO_FLAG_MIN < 0){ ERROR; return 0; }
    SO_FLAG_MAX = readConfig("SO_FLAG_MAX", CONF_FILE_PATH, menuChoise);
    if(SO_FLAG_MAX < 0){ ERROR; return 0; }
    SO_N_MOVES = readConfig("SO_N_MOVES", CONF_FILE_PATH, menuChoise);
    if(SO_N_MOVES < 0){ ERROR; return 0; }
    SO_MAX_TIME = readConfig("SO_MAX_TIME", CONF_FILE_PATH, menuChoise);
    if(SO_MAX_TIME < 0){ ERROR; return 0; }
    SO_ROUND_SCORE = readConfig("SO_ROUND_SCORE", CONF_FILE_PATH, menuChoise);
    if(SO_ROUND_SCORE <= 0){ ERROR; return 0; }
    FLAG_STRATEGY = readConfig("FLAG_STRATEGY", CONF_FILE_PATH, menuChoise);
    if(FLAG_STRATEGY < 0){ FLAG_STRATEGY = 0; }

    firstRound = numRound = 1;

    /*Ottengo il puntatore alla memoria condivisa di dimensione SO_ALTEZZA * SO_BASE*/
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

    /*Istanzio il TotalTime condiviso con il Timer*/
    idTotalTime = createSHM(IPC_PRIVATE, sizeof(float));
    if(!idTotalTime) {
        printf("Errore creazione TotalTime: ");ERROR;
        return 0;
    }
    totalTime = (float *)attachSHM(idTotalTime);
    if(totalTime == (void *)-1) {
        printf("Errore attach TotalTime: ");ERROR;
        return 0;
    }
    *totalTime = 0;

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
    idSemSyncRound = createAndInitSems(IPC_PRIVATE, 6, SO_NUM_G);/*SEM1: Gamer posiziona i Pawns*/
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
            sprintf(bufferMenuChoise, "%d", menuChoise);
            argsToGamer[0] = NAME_GAMER_PROCESS;
            argsToGamer[1] = bufferIdMsg;
            argsToGamer[2] = bufferIdSemGamer;
            argsToGamer[3] = bufferIdSemMatrix;
            argsToGamer[4] = bufferIdMatrix;
            argsToGamer[5] = bufferIdSemSyncRound;
            argsToGamer[6] = bufferMenuChoise;
            argsToGamer[7] = NULL;
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
        printf("---------------------------INIZIO ROUND %d-------------------------------\n", numRound);
        /*Imposto SEM6: Fine round (inizializzato ad SO_NUM_G * 2 xkè viene decrementato anche al primo giro dal Gamer)*/
        if(!modifySem(idSemSyncRound, 5, SO_NUM_G)) { ERROR; return 0; }

        /*2) Posiziono le bandierine*/
        actualFlagsPoint = 0;
        numFlags = generateRandom(SO_FLAG_MIN, SO_FLAG_MAX);
        printf(MAGENTA);
        printf("Il Master ha posizionato %d flags\n", numFlags);
        printf(RESET_COLOR);

        for(i = 0; i < numFlags; i++) {
            posFlag = flagPositionStrategy(FLAG_STRATEGY, idSemMatrix, SO_BASE, SO_ALTEZZA);
            /*Serve per controllare se non esiste un altra Flag (valore negativo)*/
            while(*(matrix + posFlag) < 0) {
                posFlag = flagPositionStrategy(FLAG_STRATEGY, idSemMatrix, SO_BASE, SO_ALTEZZA);
            }

            /*Assegno un valore alla bandierina*/
            if(i == numFlags - 1) {
                *(matrix + posFlag) = (SO_ROUND_SCORE + actualFlagsPoint) * -1;
            } else {
                *(matrix + posFlag) = generateRandom(1, SO_ROUND_SCORE / numFlags) * -1;
                actualFlagsPoint += *(matrix + posFlag);
            }
        }

        /*SEM5: Semaforo per sapere quando sono state prese tutte le flags (è inizializzato al numero di bandierine)*/
        if(!modifySem(idSemSyncRound, 4, (-(SO_NUM_G * firstRound) + numFlags))) { ERROR; return 0; }

        /*Stampo la matrix e le metriche del punto 1.6*/
        for(i = 0; i < SO_NUM_G; i++) {
            printf(GREEN);
            printf("--Giocatore %d: ", (dataGamer + i)->order + 1);
            printf(RESET_COLOR);
            printf("punteggio %d, mosse fatte %d, mosse residue %d\n", (dataGamer + i)->points, (dataGamer + i)->nMovesDo, (dataGamer + i)->nMovesLeft);
        }
        printf(CYAN);
        printf("\n--Situazione INIZIALE della scacchiera--");
        printf(RESET_COLOR);
        printMatrix(matrix, SO_BASE, SO_ALTEZZA);

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

        /*Reset SEM2: Master finisce le bandierine (inizializzato ad 1)*/
        if(!modifySem(idSemSyncRound, 1, 1)) { ERROR; return 0; }

        /*5) Avvio il round: avvio il Timer*/
        statusFork = fork();
        switch (statusFork) {
            case -1:
                printf("Impossibile avviare il Timer\n");
                break;
            case 0:
                timeLeft = 0.0;
                /*Timer: aspetto SO_MAX_TIME e poi lancio il segale*/
                if (!waitSem(idSemSyncRound, 3)) {
                    ERROR;
                    return 0;
                }
                /*Calcolo il tempo senza sleep() salvando quanto è passato da inizio round*/
                time(&startTime);
                time(&endTime);
                do {
                    nanosleep(&tim, NULL);
                    timeLeft += 0.01;
                    time(&endTime);
                } while(!waitSemWithoutWait(idSemSyncRound, 4) && (difftime(endTime, startTime) < SO_MAX_TIME));

                if (!waitSemWithoutWait(idSemSyncRound, 4)) {
                    timeLeft = SO_MAX_TIME;
                    if (kill(0, SIGUSR1) < 0) {
                        printf(" - Errore TIMER: ");
                        ERROR;
                    }
                } else {
                    printf("Round terminato in %f secondi\n", /*difftime(endTime, startTime)*/timeLeft);
                }

                *totalTime += timeLeft;/*difftime(endTime, startTime);*/
                exit(0);
                break;
            default:
                /*Master: avvio il round*/
                if (!modifySem(idSemSyncRound, 3, -1)) {
                    ERROR;
                    return 0;
                }

                /*Attendo di ricevere SO_NUM_G messaggi di resoconto del round*/
                printf(CYAN);
                printf("\nESITO ROUND %d:\n", numRound);
                printf(RESET_COLOR);
                for (i = 0; i < SO_NUM_G; i++) {
                    if (!receiveMessageResultRound(idMsgGamer, 1, &resultRound)) { printf("master\n"); }
                    else {
                        printf(GREEN);
                        printf("--Giocatore %d: ", resultRound.order + 1);
                        printf(RESET_COLOR);
                        printf("punteggio %d, mosse fatte %d, mosse residue %d\n", resultRound.points, resultRound.nMovesDo, resultRound.nMovesLeft);
                        dataGamer[resultRound.order].nMovesLeft = resultRound.nMovesLeft;
                        dataGamer[resultRound.order].nMovesDo = resultRound.nMovesDo;
                        dataGamer[resultRound.order].points = resultRound.points;
                    }
                }

                /*Attendo che muoia il Timer prima di verificare se ricominciare un nuovo round o no*/
                waitpid(statusFork, NULL, 0);

                /*Se scatta il timer e non ho preso tutte le bandierine stampo lo stato del semaforo 4 (quante bandierine mancano)*/
                if (!waitSemWithoutWait(idSemSyncRound, 4)) {
                    printf(GREEN);
                    printf("\nGioco terminato in ");printf(RESET_COLOR);printf("%2f ", *(totalTime));printf(GREEN);printf("secondi:\n");
                    printf(RESET_COLOR);printf("\t%d ", numFlags - getValueOfSem(idSemSyncRound, 4));printf(GREEN);
                    printf("bandierine su ");printf(RESET_COLOR);printf("%d ", numFlags);printf(GREEN);printf("sono state prese (ne mancano ");
                    printf(RESET_COLOR);printf("%d", getValueOfSem(idSemSyncRound, 4));printf(GREEN);printf(")\n");
                    printf(RESET_COLOR);printf("\t%d ", numRound);printf(GREEN);printf("round giocati\n\n");
                    totalPoints = 0; gamerWin = 1;
                    for(i = 0; i < SO_NUM_G; i++) {
                        printf(GREEN);
                        printf("\t[Giocatore %d]:\n", i + 1);
                        printf(RESET_COLOR);
                        printf("\t\tMosse fatte / mosse totali: ");
                        printf(GREEN);
                        printf("%f\n", ((float)dataGamer[i].nMovesDo / (float)(SO_NUM_P * SO_N_MOVES)));
                        printf(RESET_COLOR);
                        if(dataGamer[i].nMovesDo != 0) {
                            printf("\t\tPunti ottenuti / mosse fatte: ");
                            printf(GREEN);
                            printf("%f\n", ((float)dataGamer[i].points / (float)dataGamer[i].nMovesDo));
                            printf(RESET_COLOR);
                        } else {
                            printf("\t\tPunti ottenuti / mosse fatte: ");
                            printf(GREEN);
                            printf("non calcolabile\n");
                            printf(RESET_COLOR);
                        }
                        totalPoints += dataGamer[i].points;
                        /*Calcolo del giocatore vincitore: uso gamerWin = 0 per stabilire la partità in parità*/
                        if(i > 0 && dataGamer[i].points >= dataGamer[i - 1].points) {
                            if(dataGamer[i].points > dataGamer[i - 1].points) {
                                gamerWin = i + 1;
                            } else {
                                gamerWin = 0;
                            }
                        }

                    }
                    printf(RESET_COLOR);
                    printf("\n\tPunti totali / tempo di gioco totale: ");
                    printf(GREEN);
                    printf("%f\n\n", (totalPoints / *(totalTime)));
                    if(gamerWin == 0) {
                        printf("Il gioco finisce in ");
                        printf(RESET_COLOR);
                        printf("pareggio!\n");
                    } else {
                        printf("Vince il ");
                        printf(RESET_COLOR);
                        printf("Giocatore %d!\n", gamerWin);
                    }


                    /*Fine gioco: attendo la morte dei Gamer*/
                    while ((pidChild = wait(NULL)) != -1) {}

                    free(dataGamer);
                    removeSem(idSemSyncRound);
                    removeSem(idSemMatrix);
                    removeSem(idSemGamer);
                    removeQueue(idMsgGamer);
                    removeSHM(idTotalTime);
                    removeSHM(idMatrix);
                } else {
                    printf(CYAN);
                    printf("\nTutte le %d bandierine sono state prese!", numFlags - getValueOfSem(idSemSyncRound, 4));
                    printf(RESET_COLOR);

                    /*Reset di tutti i semafori e variabili*/
                    firstRound = 0;
                    if(!modifySem(idSemSyncRound, 2, (SO_NUM_G * SO_NUM_P))) { ERROR; return 0; }/*SEM3: Gamer fornisce strategie ai Pawns*/
                    if(!modifySem(idSemSyncRound, 3, 1)) { ERROR; return 0; }/*SEM4: Avvio round (inizializzato ad 1)*/
                    numRound++;

                    /*Aspetto la fine del round*/
                    if(!waitSem(idSemSyncRound, 5)) {ERROR;}
                }

                /*Output della scacchiera*/
                printf(CYAN);
                printf("\n--Situazione FINALE della scacchiera--");
                printf(RESET_COLOR);
                printMatrix(matrix, SO_BASE, SO_ALTEZZA);
                printf("-----------------------------FINE ROUND---------------------------------\n\n");
                break;
        }
    } while(waitSemWithoutWait(idSemSyncRound, 4));

    return 0;
}
