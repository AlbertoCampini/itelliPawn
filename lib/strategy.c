#include "strategy.h"
#include <stdio.h>

int pointer2LinearArray(const int x, const int y, const int base) {
    return base * (y - 1) + x;
}

int flagPositionStrategy(int strategy, int idSemMatrix, const int base, const int higth) {
    switch(strategy) {
        case POS_STRATEGY_RANDOM:
            return flagPositionStrategyRandom(idSemMatrix, base, higth);
            break;
        case POS_STRATEGY_ODD_OR_EVEN:
            return flagPositionStrategyOddOrEven(idSemMatrix, base, higth);
            break;
        default:
            return -1;
            break;
    }
}
int flagPositionStrategyRandom(int idSemMatrix, const int base, const int higth) {
    int pos = generateRandom(0, (base * higth));
    while(!waitSemWithoutWait(idSemMatrix, pos)) {
        pos = generateRandom(0, (base * higth));
    }
    return pos;
}
int flagPositionStrategyOddOrEven(int idSemMatrix, const int base, const int higth) {
    int pos = generateRandom(0, (base * higth));
    while((!waitSemWithoutWait(idSemMatrix, pos) || ((pos % 2) == 0)) || (waitSemWithoutWait(idSemMatrix, pos - 1) || waitSemWithoutWait(idSemMatrix, pos + 1))) {

        pos = generateRandom(0, (base * higth));
    }
    return pos;
}


int positionStrategy(int *matrix, int strategy, int gamerName, int idSemMatrix, const int base, const int higth) {
    switch(strategy) {
        case POS_STRATEGY_RANDOM:
            return positionStrategyRandom(idSemMatrix, base, higth);
            break;
        case POS_STRATEGY_ODD_OR_EVEN:
            return positionStrategyOddOrEven(idSemMatrix, base, higth);
            break;
        case POS_STRATEGY_ODD_OR_EVEN_ROW:
            return positionStrategyOddOrEvenRow(matrix, gamerName, idSemMatrix, base, higth);
            break;
        default:
            return -1;
            break;
    }
}
int positionStrategyRandom(int idSemMatrix, const int base, const int higth) {
    int pos = generateRandom(0, (base * higth));
    while(!waitSemWithoutWait(idSemMatrix, pos)) {
        pos = generateRandom(0, (base * higth));
    }
    /*Blocco il semaforo della cella in cui mi voglio posizionare e ritorno pos*/
    if(!modifySem(idSemMatrix, pos, 1)) {
        return -1;
    }
    return pos;
}
int positionStrategyOddOrEven(int idSemMatrix, const int base, const int higth) {
    int pos = generateRandom(0, (base * higth));
    while(!waitSemWithoutWait(idSemMatrix, pos) || ((pos % 2) == 1)) {
        pos = generateRandom(0, (base * higth));
    }
    /*Blocco il semaforo della cella in cui mi voglio posizionare e ritorno pos*/
    if(!modifySem(idSemMatrix, pos, 1)) {
        return -1;
    }
    return pos;
}
int positionStrategyOddOrEvenRow(int *matrix, int gamerName, int idSemMatrix, const int base, const int higth) {
    int i, row, pos, okPos = 1;

    do {
        pos = generateRandom(0, (base * higth));
        row = pos / base;
        while(!waitSemWithoutWait(idSemMatrix, pos) || ((row % 2) == gamerName)) {
            pos = generateRandom(0, (base * higth));
            row = pos / base;
        }

        /*TODO: si blocca qui!*/
        printf("%d\n", okPos);
        for(i = 0; i < base && okPos; i++) {
            printf("%d ", matrix[(row * base) + i]);
            if(matrix[(row * base) + i] == gamerName + 1) {
                okPos = 0;
            }
        }
    } while(!okPos);


    /*Blocco il semaforo della cella in cui mi voglio posizionare e ritorno pos*/
    if(!modifySem(idSemMatrix, pos, 1)) {
        return -1;
    }
    return pos;
}


int movesStrategy(int *matrix, int strategy, int idSemMatrix, int idSemFlags, const int actualPos, const int base, const int higth) {
    switch(strategy) {
        case MOVES_STRATEGY_RANDOM:
            return movesStrategyRandom(idSemMatrix, idSemFlags, actualPos, base, higth);
            break;
        case MOVES_STRATEGY_DX_OR_SX:
            return movesStrategyDxOrSx(matrix, idSemMatrix, idSemFlags, actualPos, base, higth);
            break;
        default:
            return -1;
            break;
    }
}
int movesStrategyRandom(int idSemMatrix, int idSemFlags, const int actualPos, const int base, const int higth) {
    int move = generateRandom(0, 3);
    int newPos, continua = 1;
    while(((newPos = findCorrectPos(move, actualPos, base, higth)) == -1 || !waitSemWithoutWait(idSemMatrix, newPos)) && continua) {
        if(waitSemWithoutWait(idSemFlags, 4)) {
            continua = 0;
        } else {
            move = generateRandom(0, 3);
        }

    }

    /*Se sono uscito dal while perchè tutte le Flags sono state prese non devo tornare una nuova posizione*/
    if(waitSemWithoutWait(idSemFlags, 4)) {
        return -1;
    }
    /*Blocco il semaforo della nuova posizione*/
    if(!modifySem(idSemMatrix, newPos, 1)) {
        return -1;
    }
    /*Sblocco il semaforo della vecchia posizione*/
    if(!modifySem(idSemMatrix, actualPos, -1)) {
        return -1;
    }
    return newPos;
}
int movesStrategyDxOrSx(int *matrix, int idSemMatrix, int idSemFlags, const int actualPos, const int base, const int higth) {
    /*Provo a DX e SX se c'è una bandierina (solo un movimento)*/
    int newPosDx = findCorrectPos(2, actualPos, base, higth);
    int newPosSx = findCorrectPos(3, actualPos, base, higth);

    if(newPosDx != -1 && waitSemWithoutWait(idSemMatrix, newPosDx) && *(matrix + newPosDx) < 0) {
        /*Blocco il semaforo della nuova posizione*/
        if(!modifySem(idSemMatrix, newPosDx, 1)) {
            return -1;
        }
        /*Ri-sblocco il semaforo della nuova posizione*/
        if(!modifySem(idSemMatrix, newPosDx, -1)) {
            return -1;
        }
        return newPosDx;
    }
    if(newPosSx != -1 && waitSemWithoutWait(idSemMatrix, newPosSx) && *(matrix + newPosSx) < 0) {
        /*Blocco il semaforo della nuova posizione*/
        if(!modifySem(idSemMatrix, newPosSx, 1)) {
            return -1;
        }
        /*Ri-sblocco il semaforo della nuova posizione*/
        if(!modifySem(idSemMatrix, newPosSx, -1)) {
            return -1;
        }
        return newPosSx;
    }

    return -1;
}
int findCorrectPos(int move, const int actualPos, const int base, const int higth) {
    switch(move) {
        case MOVE_UP:
            if((actualPos - base) > 0) {
                return (actualPos - base);
            }
            break;
        case MOVE_DW:
            if((actualPos + base) < (base * higth)) {
                return (actualPos + base);
            }
            break;
        case MOVE_DX:
            if((actualPos + 1) < (base * ((actualPos / base) + 1))) {
                return (actualPos + 1);
            }
            break;
        default:
            if((actualPos - 1) > (base * (actualPos / base))) {
                return (actualPos - 1);
            }
            break;
    }
    return -1;
}
