#define POS_STRATEGY_RANDOM 0
#define POS_STRATEGY_ODD_OR_EVEN 1
#define POS_STRATEGY_ODD_OR_EVEN_ROW 2
#define POS_STRATEGY_DIAGONAL 3

#define MOVES_STRATEGY_RANDOM 0
#define MOVES_STRATEGY_DX_OR_SX 1
#define MOVES_STRATEGY_ON_LINE 2
#define MOVES_STRATEGY_DIAGONAL 3

#define MOVE_UP 0
#define MOVE_DW 1
#define MOVE_DX 2
#define MOVE_SX 3

int pointer2LinearArray(const int x, const int y, const int base);
int positionStrategy(int *matrix, int strategy, int gamerName, int orderPawn, int idSemMatrix, const int base, const int higth);
int flagPositionStrategy(int strategy, int idSemMatrix, const int base, const int higth);
int movesStrategy(int *matrix, int strategy, int idSemMatrix, int idSemFlags, const int actualPos, const int base, const int higth);
