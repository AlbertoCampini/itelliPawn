#include "lib/core.h"
#include "lib/strategy.h"
#include <unistd.h>

#define CONF_FILE_PATH "./config"

static int i, points, nMoves, gamerName, SO_BASE, SO_ALTEZZA, idMsgPawns, idSemMatrix, idSemSyncRound, posInMatrix, oldPosInMatrix, alreadySend = 0;
static int *matrix;
static SyncPawn syncGamer; /*Ricevo dal Gamer*/

static void timeoutHandle (int sig) {
    /*Se non ho ancora mandato il Msg al Gamer lo faccio da qui*/
    if(!alreadySend) {
        ResultRound resultRound;
        resultRound.points = points;
        resultRound.nMovesLeft = nMoves - i;
        resultRound.nMovesDo = i;
        resultRound.order = syncGamer.order;
        if(!sendMessageResultRound(idMsgPawns, 1, resultRound)) {
            ERROR;
        }
    }

    /*Serve per mantenere sicuramente la posizione sulla scacchiera quando scatta il Timer*/
    switch(syncGamer.strategy) {
        case MOVES_STRATEGY_DX_OR_SX:
            if(posInMatrix < 0) {
                *(matrix + oldPosInMatrix) = gamerName;
            }
            break;
        default:
            if(posInMatrix >= 0) {
                *(matrix + posInMatrix) = gamerName;
            } else {
                *(matrix + oldPosInMatrix) = gamerName;
            }
            break;
    }
    exit(0);
}

int main(int argc, char *argv[]) {
    int SO_MIN_HOLD_NSEC, idMatrix, nextFlag, menuChoise;
    int *flag;
    ResultRound resultRound;
    struct timespec tim;

    sigset_t maskSignal;
    struct sigaction signalAct;

    if(argc != (ARGS_TO_PASS_OF_PAWNS - 1)) {
        printf("Parametri passati insufficienti");
        return 0;
    }

    /*Abilito il segnale SIGUSR1*/
    sigaddset(&maskSignal, SIGUSR1);
    if(sigprocmask(SIG_UNBLOCK, &maskSignal, NULL) < 0) {
        ERROR;
        return 0;
    }
    /*imposto l'handler che verra eseguito quando chiamo SIGUSR1*/
    signalAct.sa_handler = timeoutHandle;
    signalAct.sa_flags = 0;
    signalAct.sa_mask = maskSignal;
    if(sigaction(SIGUSR1, &signalAct, 0) < 0) {
        ERROR;
        return 0;
    }

    /*Leggo dal file di config*/
    sscanf(argv[7], "%d", &menuChoise);
    SO_BASE = readConfig("SO_BASE", CONF_FILE_PATH, menuChoise);
    if(SO_BASE < 0){ ERROR; return 0; }
    SO_ALTEZZA = readConfig("SO_ALTEZZA", CONF_FILE_PATH, menuChoise);
    if(SO_ALTEZZA < 0){ ERROR; return 0; }
    SO_MIN_HOLD_NSEC = readConfig("SO_MIN_HOLD_NSEC", CONF_FILE_PATH, menuChoise);
    if(SO_MIN_HOLD_NSEC < 0){ ERROR; return 0; }

    tim.tv_sec = 0;
    tim.tv_nsec = SO_MIN_HOLD_NSEC;

    sscanf(argv[1], "%d", &idMatrix);
    sscanf(argv[2], "%d", &idSemMatrix);
    sscanf(argv[3], "%d", &posInMatrix);
    sscanf(argv[3], "%d", &oldPosInMatrix);
    sscanf(argv[4], "%d", &idMsgPawns);
    sscanf(argv[5], "%d", &idSemSyncRound);
    sscanf(argv[6], "%d", &gamerName);

    /*Attacco l'area del Matrix*/
    matrix = (int *)attachSHM(idMatrix);
    if(matrix == (void *)-1) {
        printf("Errore attach Matrix: ");ERROR;
        return 0;
    }

    do {
         /*Devo ricevere le configurazioni SyncPawn dal Gamer*/
        if(!receiveMessageToGamer(idMsgPawns, getpid(), &syncGamer)) { ERROR; return 0; }
        nMoves = syncGamer.nMoves;
        alreadySend = 0;

        /*Decremento il SEM3 di -1 per dichiarare al Master che ho ricevuto la strategia*/
        if(!modifySem(idSemSyncRound, 2, -1)) { ERROR; return 0; }

        /*Se la strategia è quella ON_LINE calcolo già le bandierine da seguire*/
        if(syncGamer.strategy == MOVES_STRATEGY_ON_LINE) {
            flag = (int *)malloc(sizeof(int) * 10);
            for (i = (oldPosInMatrix / SO_BASE) * SO_BASE, nextFlag = 0; i < (((oldPosInMatrix / SO_BASE) * SO_BASE) + SO_BASE - 1); i++) {
                if(matrix[i] < 0) {
                    flag[nextFlag] = i;
                    nextFlag++;
                }
            }
        }

        /*Attendo l'inizio round*/
        if(!waitSem(idSemSyncRound, 3)) {ERROR; return 0;}

        i = 0, points = 0, nextFlag = 0;
        while(i < nMoves && !waitSemWithoutWait(idSemSyncRound, 4)) {
            /*Pulisco la posizione precedente*/
            *(matrix + oldPosInMatrix) = 0;

            /*Trovo la nuova posizione*/
            if(syncGamer.strategy == MOVES_STRATEGY_ON_LINE) {
                if(flag != 0) {
                    posInMatrix = movesStrategy(matrix, syncGamer.strategy, idSemMatrix, flag[nextFlag], oldPosInMatrix, SO_BASE, SO_ALTEZZA);
                } else {
                    posInMatrix = -1;
                }
            } else {
                posInMatrix = movesStrategy(matrix, syncGamer.strategy, idSemMatrix, idSemSyncRound, oldPosInMatrix, SO_BASE, SO_ALTEZZA);
            }

            if(posInMatrix >= 0) {
                if(*(matrix + posInMatrix) < 0) {
                    /*Ho preso una Flags*/
                    points += (*(matrix + posInMatrix) * -1);
                    if(!modifySem(idSemSyncRound, 4, -1)) { ERROR; }
                    if(syncGamer.strategy == MOVES_STRATEGY_ON_LINE) { nextFlag++; }
                }

                switch(syncGamer.strategy) {
                    case MOVES_STRATEGY_DIAGONAL:
                    case MOVES_STRATEGY_DX_OR_SX:
                        /*Ritorno alla vecchia posizione (è come se facessi un passo in più)*/
                        if(*(matrix + posInMatrix) < 0) {
                            *(matrix + posInMatrix) = 0;
                        }
                        *(matrix + oldPosInMatrix) = gamerName;
                        i++;

                        /*Se ho fatto una diagonale devo scalare ancora una mossa: se la base e la colonna di posInMatrix sono entrabe diverse da quelle di oldPosInMatrix allora è una diagonale*/
                        if(((posInMatrix / SO_BASE) != (oldPosInMatrix / SO_BASE)) && ((posInMatrix - ((posInMatrix / SO_BASE) * SO_BASE) - 1) != (oldPosInMatrix - ((oldPosInMatrix / SO_BASE) * SO_BASE) - 1))) {
                            i += 2;
                        }
                        break;
                    default:
                        *(matrix + posInMatrix) = gamerName;
                        oldPosInMatrix = posInMatrix;
                        break;
                }

                i++;
                nanosleep(&tim, NULL);
            } else {
                /*La strategia non sa che mossa far fare alla pedina e allora si deve stagnare sul posto*/
                *(matrix + oldPosInMatrix) = gamerName;
            }
        }

        if(syncGamer.strategy == MOVES_STRATEGY_ON_LINE) {
            free(flag);
        }

        /*Invio il resoconto al mio Gamer*/
        resultRound.order = syncGamer.order;
        resultRound.points = points;
        resultRound.nMovesLeft = nMoves - i;
        resultRound.nMovesDo = i;
        if(!sendMessageResultRound(idMsgPawns, 1, resultRound)) {
            ERROR;
        }

        if(!waitSem(idSemSyncRound, 5)) {ERROR;}
    } while(1);

    return 0;
}
