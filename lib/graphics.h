#define BLU "\033[0;34m"
#define RED "\033[0;31m"
#define YELLOW "\033[0;33m"
#define GREEN "\033[0;32m"
#define MAGENTA "\033[0;35m"
#define CYAN "\033[0;36m"
#define RESET_COLOR "\033[0m"
#define PRINT_MATRIX printf("%d", matrix[i])

void printTitle();
void printMatrix(int *matrix, const int base, const int higth);
