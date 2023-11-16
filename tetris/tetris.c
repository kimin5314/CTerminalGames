#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <conio.h>

#define WIDTH 10
#define HEIGHT 20
#define BORDER 1
#define PRELOAD 4
#define WIDTH_BORDER (WIDTH + BORDER * 2)
#define HEIGHT_BORDER (HEIGHT + BORDER + PRELOAD)
#define SCORE_PER_LINE 10
#define ACTION_PER_FRAME 3

#define RANDINT(min, max) (rand() % (max - min + 1) + min)

int shapes[7][4][4][2] = {
        {
                {{0, 0}, {0, 1}, {0, 2}, {0, 3}},
                {{0, 0}, {1, 0}, {2, 0}, {3, 0}},
                {{0, 0}, {0, 1}, {0, 2}, {0, 3}},
                {{0, 0}, {1, 0}, {2, 0}, {3, 0}},
        },
        {
                {{0, 1}, {1, 1}, {2, 0}, {2, 1}},
                {{0, 0}, {1, 0}, {1, 1}, {1, 2}},
                {{0, 0}, {0, 1}, {1, 0}, {2, 0}},
                {{0, 0}, {0, 1}, {0, 2}, {1, 2}},
        },
        {
                {{0, 0}, {1, 0}, {2, 0}, {2, 1}},
                {{0, 0}, {0, 1}, {0, 2}, {1, 0}},
                {{0, 0}, {0, 1}, {1, 1}, {2, 1}},
                {{0, 2}, {1, 0}, {1, 1}, {1, 2}},
        },
        {
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
        },
        {
                {{0, 1}, {0, 2}, {1, 0}, {1, 1}},
                {{0, 0}, {1, 0}, {1, 1}, {2, 1}},
                {{0, 1}, {0, 2}, {1, 0}, {1, 1}},
                {{0, 0}, {1, 0}, {1, 1}, {2, 1}},
        },
        {
                {{0, 0}, {0, 1}, {0, 2}, {1, 1}},
                {{0, 1}, {1, 0}, {1, 1}, {2, 1}},
                {{0, 1}, {1, 0}, {1, 1}, {1, 2}},
                {{0, 0}, {1, 0}, {1, 1}, {2, 0}},
        },
        {
                {{0, 0}, {0, 1}, {1, 1}, {1, 2}},
                {{0, 1}, {1, 0}, {1, 1}, {2, 0}},
                {{0, 0}, {0, 1}, {1, 1}, {1, 2}},
                {{0, 1}, {1, 0}, {1, 1}, {2, 0}},
        },
};

int board[HEIGHT_BORDER][WIDTH_BORDER] = {0};
int buffer[HEIGHT_BORDER][WIDTH_BORDER] = {0};

typedef enum {
    LEFT, RIGHT, DOWN, UP, ROTATE_CLOCKWISE, ROTATE_COUNTER_CLOCKWISE, DROP
} Action;

typedef struct {
    int x, y, w;
    int shape, rotation;
    int color;
} Tetromino;

Tetromino t;

void initBoard();

void initTetromino();

int calcWidth();

void move(Action a);

void clearTetromino();

void drawTetromino();

int checkCollision();

int deleteLine();

void printColorBlock(int color);

void showBoard();

void gameLoop(int interval);

int main(int argc, char *argv[]) {
    srand((unsigned) time(NULL));
    initTetromino();
    initBoard();

    switch (argc) {
        case 1:
            gameLoop(200);
            break;
        case 2:
            if (argv[1][0] == '-') {
                switch (argv[1][1]) {
                    case 'e':
                        gameLoop(300);
                        break;
                    case 'n':
                        gameLoop(200);
                        break;
                    case 'd':
                        gameLoop(100);
                        break;
                    case 'h':
                        printf("Tetris\n");
                        printf("Usage: tetris [OPTION]\n");
                        printf("Options:\n");
                        printf("  -e\t\tEasy mode\n");
                        printf("  -n\t\tNormal mode\n");
                        printf("  -d\t\tDifficult mode\n");
                        break;
                    case 'v':
                        printf("Tetris 1.0\n");
                        break;
                    default:
                        printf("unknown command! use -h to get help!\n");
                        break;
                }
            } else {
                gameLoop(200);
            }
            break;
        default:
            gameLoop(200);
            break;
    }
    return 0;
}

void initBoard() {
    system("cls");
    printf("\033[47m\033[36m+-+ +-+ +-+ +--+    +--+ +-+ +--+  +-+\n");
    printf("| |/ /  | | |   \\  /   | | | |   \\ | |\n");
    printf("|   |   | | | |\\ \\/ /| | | | | |\\ \\| |\n");
    printf("| |\\ \\  | | | | \\  / | | | | | | \\   |\n");
    printf("+-+ +-+ +-+ +-+  ++  +-+ +-+ +-+  +--+\033[0m\033[40m\n");
    system("pause");

    system("cls");
    for (int i = 0; i < HEIGHT_BORDER; ++i) {
        board[i][0] = board[i][WIDTH_BORDER - 1] = -1;
    }
    for (int i = 1; i < WIDTH_BORDER - 1; ++i) {
        board[HEIGHT_BORDER - 1][i] = -1;
    }
    for (int i = PRELOAD; i < HEIGHT_BORDER; ++i) {
        for (int j = 0; j < WIDTH_BORDER; ++j) {
            printColorBlock(board[i][j]);
        }
        printf("\n");
    }
}

void initTetromino() {
    t.shape = RANDINT(0, 6);
    t.rotation = RANDINT(0, 3);
    t.w = calcWidth();
    t.x = RANDINT(BORDER, WIDTH_BORDER - BORDER - t.w);
    t.y = 0;
    t.color = RANDINT(1, 6);
}

int calcWidth() {
    int w = 0;
    for (int i = 0; i < 4; ++i) {
        w = max(w, shapes[t.shape][t.rotation][i][1] + 1);
    }
    return w;
}

void move(Action a) {
    switch (a) {
        case LEFT:
            t.x--;
            break;
        case RIGHT:
            t.x++;
            break;
        case DOWN:
            t.y++;
            break;
        case UP:
            t.y--;
            break;
        case ROTATE_CLOCKWISE:
            t.rotation = (t.rotation + 1) % 4;
            break;
        case ROTATE_COUNTER_CLOCKWISE:
            t.rotation = (t.rotation + 3) % 4;
            break;
        case DROP:
            while (!checkCollision()) {
                t.y++;
            }
            t.y--;
            break;
        default:
            break;
    }
}

void clearTetromino() {
    for (int i = 0; i < 4; ++i) {
        board[t.y + shapes[t.shape][t.rotation][i][0]][t.x + shapes[t.shape][t.rotation][i][1]] = 0;
    }
}

void drawTetromino() {
    for (int i = 0; i < 4; ++i) {
        board[t.y + shapes[t.shape][t.rotation][i][0]][t.x + shapes[t.shape][t.rotation][i][1]] = t.color;
    }
}

int checkCollision() {
    for (int i = 0; i < 4; ++i) {
        if (board[t.y + shapes[t.shape][t.rotation][i][0]][t.x + shapes[t.shape][t.rotation][i][1]]) {
            return 1;
        }
    }
    return 0;
}

int deleteLine() {
    int count = 0;
    for (int i = HEIGHT_BORDER - 2; i >= 0; --i) {
        int full = 1;
        for (int j = 1; j < WIDTH_BORDER - 1; ++j) {
            if (!board[i][j]) {
                full = 0;
                break;
            }
        }
        if (full) {
            ++count;
            for (int j = i; j > 0; --j) {
                for (int k = 1; k < WIDTH_BORDER - 1; ++k) {
                    board[j][k] = board[j - 1][k];
                }
            }
            for (int j = 1; j < WIDTH_BORDER - 1; ++j) {
                board[0][j] = 0;
            }
            ++i;
        }
    }
    return count;
}

void printColorBlock(int color) {
    switch (color) {
        case -1:
            printf("\033[47m  \033[0m");
            break;
        case 0:
            printf("  ");
            break;
        case 1:
            printf("\033[41m  \033[0m");
            break;
        case 2:
            printf("\033[42m  \033[0m");
            break;
        case 3:
            printf("\033[43m  \033[0m");
            break;
        case 4:
            printf("\033[44m  \033[0m");
            break;
        case 5:
            printf("\033[45m  \033[0m");
            break;
        case 6:
            printf("\033[46m  \033[0m");
            break;
        default:
            break;
    }
}

void showBoard() {
    for (int i = PRELOAD; i < HEIGHT_BORDER; ++i) {
        for (int j = 0; j < WIDTH_BORDER; ++j) {
            if (board[i][j] != buffer[i][j]) {
                printf("\033[%d;%dH", i - PRELOAD + 1, j * 2 + 1);
                buffer[i][j] = board[i][j];
                printColorBlock(board[i][j]);
            }
        }
    }
}

void getAction() {
    if (kbhit()) {
        clearTetromino();
        switch (getch()) {
            case 'a':
                move(LEFT);
                if (checkCollision()) {
                    move(RIGHT);
                }
                break;
            case 'd':
                move(RIGHT);
                if (checkCollision()) {
                    move(LEFT);
                }
                break;
            case 's':
                move(DROP);
                if (checkCollision()) {
                    move(UP);
                }
                break;
            case 'w':
                move(ROTATE_CLOCKWISE);
                if (checkCollision()) {
                    move(ROTATE_COUNTER_CLOCKWISE);
                }
                break;
            case 'p':
                printf("Game Paused! Press p to continue...\n");
                while (1) {
                    if (getch() == 'p') {
                        break;
                    }
                }
                break;
            default:
                break;
        }
        drawTetromino();
        showBoard();
    }
}

void gameLoop(int interval) {
    int score = 0;
    while (1) {
        for (int i = 0; i < ACTION_PER_FRAME; ++i) {
            getAction();
        }
        clearTetromino();
        move(DOWN);
        if (checkCollision()) {
            move(UP);
            drawTetromino();
            score += deleteLine() * SCORE_PER_LINE;
            if (t.y < PRELOAD) {
                printf("\033[%d;1H", HEIGHT_BORDER - 1);
                printf("Game Over!\nYour score is %d!\n", score);
                break;
            }
            initTetromino();
        }
        drawTetromino();
        showBoard();
        Sleep(interval);
    }
}
