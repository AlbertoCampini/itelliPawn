#include "strategy.h"

int pointer2LinearArray(const int x, const int y, const int base) {
    return base * (y - 1) + x;
}

int flagPositionStrategy(int strategy, int idSemMatrix, const int base, const int higth) {
    switch(strategy) {
        case POS_STRATEGY_RANDOM:
            return flagPositionStrategyRandom(idSemMatrix, base, higth);
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


int positionStrategy(int strategy, int idSemMatrix, const int base, const int higth) {
    switch(strategy) {
        case POS_STRATEGY_RANDOM:
            return positionStrategyRandom(idSemMatrix, base, higth);
            break;
        case POS_STRATEGY_OPTIMIZE:
            return 2;
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


int movesStrategy(int strategy, int idSemMatrix, const int actualPos, const int base, const int higth) {
    switch(strategy) {
        case MOVES_STRATEGY_RANDOM:
            return movesStrategyRandom(idSemMatrix, actualPos, base, higth);
            break;
        default:
            return -1;
            break;
    }
}
int movesStrategyRandom(int idSemMatrix, const int actualPos, const int base, const int higth) {
    int move = generateRandom(0, 3);
    int newPos;
    while((newPos = findCorrectPos(move, actualPos, base, higth)) == -1 || !waitSemWithoutWait(idSemMatrix, newPos)) {
        move = generateRandom(0, 3);
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
