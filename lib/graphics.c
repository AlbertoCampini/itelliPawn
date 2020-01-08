#include "graphics.h"
#include <stdio.h>

void printTitle() {
    printf(CYAN);
    printf("._____________    .__  .__         __________                      ");
    printf("\n");
    printf("|__\\__    ___/___ |  | |  | ___.__.\\______   \\_____ __  _  ______  ");
    printf("\n");
    printf("|  | |    |_/ __ \\|  | |  |<   |  | |     ___/\\__  \\\\ \\/ \\/ /    \\ ");
    printf("\n");
    printf("|  | |    |\\  ___/|  |_|  |_\\___  | |    |     / __ \\     /   |   \\");
    printf("\n");
    printf("|__| |____| \\___  >____/____/ ____| |____|    (____  /\\/\\_/|___|  /");
    printf("\n");
    printf("                \\/          \\/                     \\/           \\/ ");
    printf("\n");
    printf("\tFatto da Alberto Campini e Lorenzo Bergadano\n\n");
    printf(RESET_COLOR);

    printf("Premi un tasto per iniziare il gioco\n");
    getc(stdin);
}

void printMatrix(int *matrix, const int base, const int higth) {
    int i;
    printf("\n|");
    for(i = 0; i < (base * higth); i++) {
        if((i % base) == 0 && i != 0) {
            printf("\n");
            printf("|");
        }
        if(matrix[i] < 0) {
            printf("F");
            printf("|");
        } else if(matrix[i] == 0) {
            printf(" |");
        } else {
            switch (matrix[i]){
                case 1:
                    printf(RED);
                    PRINT_MATRIX;
                    printf(RESET_COLOR);
                    printf("|");
                    break;
                case 2:
                    printf(YELLOW);
                    PRINT_MATRIX;
                    printf(RESET_COLOR);
                    printf("|");
                    break;
                case 3:
                    printf(BLU);
                    PRINT_MATRIX;
                    printf(RESET_COLOR);
                    printf("|");
                    break;
                case 4:
                    printf(GREEN);
                    PRINT_MATRIX;
                    printf(RESET_COLOR);
                    printf("|");
                    break;
                default:
                    printf(RESET_COLOR);
                    PRINT_MATRIX;
                    printf("|");
                    break;
            }
        }
    }
    printf("\n");
}
