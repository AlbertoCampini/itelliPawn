#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/signal.h>

#define ARGS_TO_PASS_OF_GAMER 8
#define ARGS_TO_PASS_OF_PAWNS 9

#define ERROR printLastError()
#define MAX_LINE_CONF 100
#define MAX_BUFF_SIZE 20
#define NAME_GAMER_PROCESS "gamer"
#define NAME_PAWN_PROCESS "pawn"

/*CORE FUNCTION*/
int readConfig(char *config, const char *fPath, int preferedMode);
void printLastError();
int generateRandom(int to, int from);

/*CORE STRUCT*/
typedef struct {
    long mtype;
    int order; /*Serve per dare un ordine di posizionamento dei Gamer*/
    int name;
    int strategy;
    int points;
} SyncGamer;

typedef struct {
    long mtype;
    int order; /*Serve per dare un ordine di indicizzazione*/
    int strategy;
    int nMoves;
} SyncPawn;

typedef struct {
    long mtype;
    int order; /*Serve per dare un ordine di indicizzazione per sapere a quale Pawn fare riferimento*/
    int points;
    int nMovesLeft;
    int nMovesDo;
} ResultRound;

#if defined(__linux__)
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *_buf;
};
#endif

/*IPC QUEUE*/
int createMsgQueue(key_t key);
int removeQueue(int id);
int sendMessageToGamer(int idMsg, long msgType, SyncGamer syncGamer);
int receiveMessageToMaster(int idMsg, long msgType, void *msg);
int sendMessageToPawns(int idMsg, long msgType, SyncPawn syncPawn);
int receiveMessageToGamer(int idMsg, long msgType, void *msg);
int sendMessageResultRound(int idMsg, long msgType, ResultRound resultRound);
int receiveMessageResultRound(int idMsg, long msgType, void *msg);

/*IPC SEMAPHORE*/
int createAndInitSems(key_t semKey, const int nSems, unsigned short valInit);
int removeSem(int semId);
int modifySem(int semId, int semNum, const int num);
int getValueOfSem(int semId, int semNum);
int waitSem(int semId, int semNum);
int waitSemWithoutWait(int semId, int semNum);

/*IPC SHARED MEMORY*/
int createSHM(key_t key, size_t dim);
int removeSHM(int idSHM);
void *attachSHM(int idSHM);
void initSHM(int *matrix, const int base, const int higth, const int valInit);
