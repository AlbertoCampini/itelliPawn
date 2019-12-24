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

#define ARGS_TO_PASS_OF_GAMER 7
#define ARGS_TO_PASS_OF_PAWNS 8

#define ERROR printLastError()
#define EASY_MODE 0
#define HARD_MODE 1
#define MAX_LINE_CONF 100
#define MAX_BUFF_SIZE 20
#define NAME_GAMER_PROCESS "gamer"
#define NAME_PAWN_PROCESS "pawn"

#define BLU "\033[0;34m"
#define RED "\033[0;31m"
#define YELLOW "\033[0;33m"
#define GREEN "\033[0;32m"
#define MAGENTA "\033[0;35m"
#define CYAN "\033[0;36m"
#define RESET_COLOR "\033[0m"
#define PRINT_MATRIX printf("%d", matrix[i])

/*CORE FUNCTION*/
const int readConfig(char *config, const char *fPath);
void printLastError();
int generateRandom(int to, int from);
void printMatrix(int *matrix, const int base, const int higth);

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
