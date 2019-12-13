#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define ERROR printLastError()
#define EASY_MODE 0
#define HARD_MODE 1
#define MAX_LINE_CONF 100
#define MAX_BUFF_SIZE 20
#define NAME_GAMER_PROCESS "gamer"
#define NAME_PAWN_PROCESS "pawn"

/*CORE FUNCTION*/
extern const int readConfig(char *config, int mode, const char *fPath);
extern void printLastError();
extern int generateRandom(int to, int from);
extern void printMatrix(int *matrix, const int base, const int higth);

/*CORE STRUCT*/
typedef struct {
    long mtype;
    int order;
    int name;
    int strategy;
} SyncGamer;

#if defined(__linux__)
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *_buf;
};
#endif

/*IPC QUEUE*/
extern int createMsgQueue(key_t key);
extern int removeQueue(int id);
extern int sendMessage(int idMsg, long msgType, SyncGamer syncGamer);
extern int receiveMessage(int idMsg, long msgType, void *msg);

/*IPC SEMAPHORE*/
extern int createAndInitSems(key_t semKey, const int nSems, unsigned short valInit);
extern int removeSem(int semId);
extern int modifySem(int semId, int semNum, const int num);
extern int waitSem(int semId, int semNum);
extern int waitSemWithoutWait(int semId, int semNum);

/*IPC SHARED MEMORY*/
extern int createSHM(key_t key, size_t dim);
extern int removeSHM(int idSHM);
extern void *attachSHM(int idSHM);
extern void initSHM(int *matrix, const int base, const int higth, const int valInit);
