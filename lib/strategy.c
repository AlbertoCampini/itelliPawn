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


int positionStrategy(int *matrix, int strategy, int gamerName, int orderPawn, int idSemMatrix, const int base, const int higth) {
    switch(strategy) {
        case POS_STRATEGY_RANDOM:
            return positionStrategyRandom(idSemMatrix, base, higth);
            break;
        case POS_STRATEGY_ODD_OR_EVEN:
            return positionStrategyOddOrEven(idSemMatrix, base, higth);
            break;
        case POS_STRATEGY_ODD_OR_EVEN_ROW:
            return positionStrategyOddOrEvenRow(matrix, gamerName, orderPawn, idSemMatrix, base, higth);
            break;
        case POS_STRATEGY_DIAGONAL:
            return positionStrategyDiagonal(idSemMatrix, base, higth);
            break;
        default:
            return -1;
            break;
    }
}
/*Posizionamento random*/
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
/*Posizionamento in colonne dispari (per Hard Mode)*/
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
/*Posizionamento in righe: ogni squadra ha la sua riga (per Easy Mode)*/
int positionStrategyOddOrEvenRow(int *matrix, int gamerName, int orderPawn, int idSemMatrix, const int base, const int higth) {
    int i, pos;

    pos = (((orderPawn * 2) + gamerName) * base) + generateRandom(0, base);
    while(!waitSemWithoutWait(idSemMatrix, pos)) {
        pos = (((orderPawn * 2) + gamerName) * base) + generateRandom(0, base);
    }

    /*Blocco il semaforo della cella in cui mi voglio posizionare e ritorno pos*/
    if(!modifySem(idSemMatrix, pos, 1)) {
        return -1;
    }
    return pos;
}
/*Posizionamento in diagonale (per Hard Mode)*/
int positionStrategyDiagonal(int idSemMatrix, const int base, const int higth) {
    int pos = generateRandom(0, (base * higth));
    int row = pos / base;
    int col = pos - (row * base) - 1;
    while(!waitSemWithoutWait(idSemMatrix, pos) || ((row % 2) == 0 && (col % 2) == 1) || ((row % 2) == 1 && (col % 2) == 0)) {
        pos = generateRandom(0, (base * higth));
        row = pos / base;
        col = pos - (row * base) - 1;
    }
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
        case MOVES_STRATEGY_ON_LINE:
            return movesStrategyOnLine(matrix, idSemMatrix, idSemFlags, actualPos, base, higth);
            break;
        case MOVES_STRATEGY_DIAGONAL:
            return movesStrategyDiagonal(matrix, idSemMatrix, idSemFlags, actualPos, base, higth);
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
    /*Provo a DX, SX, UP e DW se c'è una bandierina (solo un movimento)*/
    int newPosDx = findCorrectPos(MOVE_DX, actualPos, base, higth);
    int newPosSx = findCorrectPos(MOVE_SX, actualPos, base, higth);
    int newPosUp = findCorrectPos(MOVE_UP, actualPos, base, higth);
    int newPosDw = findCorrectPos(MOVE_DW, actualPos, base, higth);

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
    if(newPosUp != -1 && waitSemWithoutWait(idSemMatrix, newPosUp) && *(matrix + newPosUp) < 0) {
        /*Blocco il semaforo della nuova posizione*/
        if(!modifySem(idSemMatrix, newPosUp, 1)) {
            return -1;
        }
        /*Ri-sblocco il semaforo della nuova posizione*/
        if(!modifySem(idSemMatrix, newPosUp, -1)) {
            return -1;
        }
        return newPosUp;
    }
    if(newPosDw != -1 && waitSemWithoutWait(idSemMatrix, newPosDw) && *(matrix + newPosDw) < 0) {
        /*Blocco il semaforo della nuova posizione*/
        if(!modifySem(idSemMatrix, newPosDw, 1)) {
            return -1;
        }
        /*Ri-sblocco il semaforo della nuova posizione*/
        if(!modifySem(idSemMatrix, newPosDw, -1)) {
            return -1;
        }
        return newPosDw;
    }

    return -1;
}
int movesStrategyOnLine(int *matrix, int idSemMatrix, int flag, const int actualPos, const int base, const int higth) {
    /*Controllo su tutta la linea se c'è una Flag: se si allora mi muovo altrimenti no*/
    int newPos;
    if(flag != 0) {
        /*So che c'è una flag sulla riga e mi muovo verso di essa*/
        if((actualPos - flag) < 0) {
            newPos = findCorrectPos(2, actualPos, base, higth);
        } else {
            newPos = findCorrectPos(3, actualPos, base, higth);
        }

        /*Blocco il semaforo della nuova posizione*/
        if(!modifySem(idSemMatrix, newPos, 1)) {
            return -1;
        }
        /*Sblocco il semaforo della vecchia posizione*/
        if(!modifySem(idSemMatrix, actualPos, -1)) {
            return -1;
        }
    } else {
        /*Ritorno -1 perchè non ha senso muovermi*/
        newPos = -1;
    }
    return newPos;
}
/*Provo a DX, SX, UP DW e le 4 diagonali possibili se c'è una bandierina (solo un movimento)*/
int movesStrategyDiagonal(int *matrix, int idSemMatrix, int idSemFlags, const int actualPos, const int base, const int higth) {
    int newPosDx = findCorrectPos(MOVE_DX, actualPos, base, higth);
    int newPosSx = findCorrectPos(MOVE_SX, actualPos, base, higth);
    int newPosUp = findCorrectPos(MOVE_UP, actualPos, base, higth);
    int newPosDw = findCorrectPos(MOVE_DW, actualPos, base, higth);

    int diagonal1 = findCorrectPos(MOVE_SX, actualPos, base, higth);
    if(diagonal1 != -1) { diagonal1 = findCorrectPos(MOVE_UP, diagonal1, base, higth); }

    int diagonal2 = findCorrectPos(MOVE_DX, actualPos, base, higth);
    if(diagonal2 != -1) { diagonal2 = findCorrectPos(MOVE_UP, diagonal2, base, higth); }

    int diagonal3 = findCorrectPos(MOVE_SX, actualPos, base, higth);
    if(diagonal3 != -1) { diagonal3 = findCorrectPos(MOVE_DW, diagonal3, base, higth); }

    int diagonal4 = findCorrectPos(MOVE_DX, actualPos, base, higth);
    if(diagonal4 != -1) { diagonal4 = findCorrectPos(MOVE_DW, diagonal4, base, higth); }

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
    if(newPosUp != -1 && waitSemWithoutWait(idSemMatrix, newPosUp) && *(matrix + newPosUp) < 0) {
        /*Blocco il semaforo della nuova posizione*/
        if(!modifySem(idSemMatrix, newPosUp, 1)) {
            return -1;
        }
        /*Ri-sblocco il semaforo della nuova posizione*/
        if(!modifySem(idSemMatrix, newPosUp, -1)) {
            return -1;
        }
        return newPosUp;
    }
    if(newPosDw != -1 && waitSemWithoutWait(idSemMatrix, newPosDw) && *(matrix + newPosDw) < 0) {
        /*Blocco il semaforo della nuova posizione*/
        if(!modifySem(idSemMatrix, newPosDw, 1)) {
            return -1;
        }
        /*Ri-sblocco il semaforo della nuova posizione*/
        if(!modifySem(idSemMatrix, newPosDw, -1)) {
            return -1;
        }
        return newPosDw;
    }
    if(diagonal1 != -1 && waitSemWithoutWait(idSemMatrix, diagonal1) && *(matrix + diagonal1) < 0) {
        /*Blocco il semaforo della nuova posizione*/
        if(!modifySem(idSemMatrix, diagonal1, 1)) {
            return -1;
        }
        /*Ri-sblocco il semaforo della nuova posizione*/
        if(!modifySem(idSemMatrix, diagonal1, -1)) {
            return -1;
        }
        return diagonal1;
    }
    if(diagonal2 != -1 && waitSemWithoutWait(idSemMatrix, diagonal2) && *(matrix + diagonal2) < 0) {
        /*Blocco il semaforo della nuova posizione*/
        if(!modifySem(idSemMatrix, diagonal2, 1)) {
            return -1;
        }
        /*Ri-sblocco il semaforo della nuova posizione*/
        if(!modifySem(idSemMatrix, diagonal2, -1)) {
            return -1;
        }
        return diagonal2;
    }
    if(diagonal3 != -1 && waitSemWithoutWait(idSemMatrix, diagonal3) && *(matrix + diagonal3) < 0) {
        /*Blocco il semaforo della nuova posizione*/
        if(!modifySem(idSemMatrix, diagonal3, 1)) {
            return -1;
        }
        /*Ri-sblocco il semaforo della nuova posizione*/
        if(!modifySem(idSemMatrix, diagonal3, -1)) {
            return -1;
        }
        return diagonal3;
    }
    if(diagonal4 != -1 && waitSemWithoutWait(idSemMatrix, diagonal4) && *(matrix + diagonal4) < 0) {
        /*Blocco il semaforo della nuova posizione*/
        if(!modifySem(idSemMatrix, diagonal4, 1)) {
            return -1;
        }
        /*Ri-sblocco il semaforo della nuova posizione*/
        if(!modifySem(idSemMatrix, diagonal4, -1)) {
            return -1;
        }
        return diagonal4;
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
