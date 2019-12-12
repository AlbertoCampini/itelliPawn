#define POS_STRATEGY_RANDOM 0
#define POS_STRATEGY_OPTIMIZE 1

extern int pointer2LinearArray(const int x, const int y, const int base);
extern int positionStrategy(int strategy, int idSemMatrix, const int base, const int higth);

int pointer2LinearArray(const int x, const int y, const int base) {
    return base * (y - 1) + x;
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
