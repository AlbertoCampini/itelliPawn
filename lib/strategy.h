#define POS_STRATEGY_RANDOM 0
#define POS_STRATEGY_OPTIMIZE 1

#define MOVES_STRATEGY_RANDOM 0
#define MOVE_UP 0
#define MOVE_DW 1
#define MOVE_DX 2
#define MOVE_SX 3

int pointer2LinearArray(const int x, const int y, const int base);
int positionStrategy(int strategy, int idSemMatrix, const int base, const int higth);
int flagPositionStrategy(int strategy, int idSemMatrix, const int base, const int higth);
int movesStrategy(int strategy, int idSemMatrix, const int actualPos, const int base, const int higth);
