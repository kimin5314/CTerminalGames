#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)

#include <conio.h>
#include <windows.h>

#define CLEAR_SCREEN "cls"
#else
#include <unistd.h>
#include <termios.h>
#define CLEAR_SCREEN "clear"
int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}
#endif

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20
#define BORDER 1
#define PRELOAD 4
#define STATUS_WIDTH 4
#define WIDTH_BORDER (BOARD_WIDTH + BORDER * 2)
#define HEIGHT_BORDER (BOARD_HEIGHT + BORDER + PRELOAD)

#define SCORE_PER_LINE 10
#define ACTION_PER_FRAME 4

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

int status[BOARD_HEIGHT + BORDER][STATUS_WIDTH + BORDER] = {0};

typedef enum {
    LEFT, RIGHT, DOWN, UP, ROTATE_CLOCKWISE, ROTATE_COUNTER_CLOCKWISE, DROP
} Action;

typedef struct {
    int x, y, w;
    int shape, rotation;
    int color;
} Tetromino;

void showKIMIN();

void initGameScreen();

void initTetromino(Tetromino* t);

void showNextTetromino(Tetromino* t);

int calcWidth(Tetromino* t);

void move(Tetromino* t, Action a);

void clearTetromino(Tetromino* t);

void drawTetromino(Tetromino* t);

int checkCollision(Tetromino* t);

int deleteLine();

void printColorBlock(int color);

void showBoard();

void responseAction(Tetromino* t);

void gameLoop(int interval);

int main(int argc, char* argv[]) {
    srand(time(NULL));
    initGameScreen();

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
    printf("\033[%d;%dH\033[0m", BOARD_HEIGHT + BORDER + 1, 1);
    return 0;
}

void showKIMIN() {
    system(CLEAR_SCREEN);
    printf("\033[47m\033[36m+-+ +-+ +-+ +--+    +--+ +-+ +--+  +-+\n");
    printf("| |/ /  | | |   \\  /   | | | |   \\ | |\n");
    printf("|   |   | | | |\\ \\/ /| | | | | |\\ \\| |\n");
    printf("| |\\ \\  | | | | \\  / | | | | | | \\   |\n");
    printf("+-+ +-+ +-+ +-+  ++  +-+ +-+ +-+  +--+\033[0m\033[40m\n");
    system("pause");
}

void initGameScreen() {
    showKIMIN();
    system(CLEAR_SCREEN);

    for (int i = 0; i < HEIGHT_BORDER; ++i) {
        board[i][0] = board[i][WIDTH_BORDER - 1] = -1;
    }
    for (int i = 1; i < WIDTH_BORDER - 1; ++i) {
        board[HEIGHT_BORDER - 1][i] = -1;
    }

    for (int i = 0; i < BOARD_HEIGHT + BORDER; ++i) {
        status[i][STATUS_WIDTH + BORDER - 1] = -1;
    }
    for (int i = 0; i < STATUS_WIDTH + BORDER - 1; ++i) {
        status[0][i] = status[BOARD_HEIGHT + BORDER - 1][i] = status[5][i] = -1;
    }

    for (int i = PRELOAD; i < HEIGHT_BORDER; ++i) {
        for (int j = 0; j < WIDTH_BORDER; ++j) {
            printColorBlock(board[i][j]);
        }
        for (int j = 0; j < STATUS_WIDTH + BORDER; ++j) {
            printColorBlock(status[i - PRELOAD][j]);
        }
        printf("\n");
    }

    printf("\033[8;26H");
    printf("SCORE:");
    printf("\033[9;26H");
    printf("0");
}

void showNextTetromino(Tetromino* t) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            printf("\033[%d;%dH  ", 2 + i, 25 + j * 2);
        }
    }
    for (int i = 0; i < 4; ++i) {
        printf("\033[%d;%dH",
               2 + shapes[t->shape][t->rotation][i][0], 25 + shapes[t->shape][t->rotation][i][1] * 2);
        printColorBlock(t->color);
    }
}

void initTetromino(Tetromino* t) {
    t->shape = RANDINT(0, 6);
    t->rotation = RANDINT(0, 3);
    t->w = calcWidth(t);
    t->x = RANDINT(BORDER, WIDTH_BORDER - BORDER - t->w);
    t->y = 0;
    t->color = RANDINT(1, 6);
    showNextTetromino(t);
}

int calcWidth(Tetromino* t) {
    int w = 0;
    for (int i = 0; i < 4; ++i) {
        w = max(w, shapes[t->shape][t->rotation][i][1] + 1);
    }
    return w;
}

void move(Tetromino* t, Action a) {
    switch (a) {
        case LEFT:
            t->x--;
            break;
        case RIGHT:
            t->x++;
            break;
        case DOWN:
            t->y++;
            break;
        case UP:
            t->y--;
            break;
        case ROTATE_CLOCKWISE:
            t->rotation = (t->rotation + 1) % 4;
            break;
        case ROTATE_COUNTER_CLOCKWISE:
            t->rotation = (t->rotation + 3) % 4;
            break;
        case DROP:
            while (!checkCollision(t)) {
                t->y++;
            }
            t->y--;
            break;
        default:
            break;
    }
}

void clearTetromino(Tetromino* t) {
    for (int i = 0; i < 4; ++i) {
        board[t->y + shapes[t->shape][t->rotation][i][0]][t->x + shapes[t->shape][t->rotation][i][1]] = 0;
    }
}

void drawTetromino(Tetromino* t) {
    for (int i = 0; i < 4; ++i) {
        board[t->y + shapes[t->shape][t->rotation][i][0]][t->x + shapes[t->shape][t->rotation][i][1]] = t->color;
    }
}

int checkCollision(Tetromino* t) {
    for (int i = 0; i < 4; ++i) {
        if (board[t->y + shapes[t->shape][t->rotation][i][0]][t->x + shapes[t->shape][t->rotation][i][1]]) {
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
                buffer[i][j] = board[i][j];

                printf("\033[%d;%dH", i - PRELOAD + 1, j * 2 + 1); // move cursor
                printColorBlock(board[i][j]);
            }
        }
        printf("\033[%d;%dH", BOARD_HEIGHT + BORDER, 1);
    }
}

void responseAction(Tetromino* t) {
    if (kbhit()) {
        clearTetromino(t);
        switch (getch()) {
            case 'a':
                move(t, LEFT);
                if (checkCollision(t)) {
                    move(t, RIGHT);
                }
                break;
            case 'd':
                move(t, RIGHT);
                if (checkCollision(t)) {
                    move(t, LEFT);
                }
                break;
            case 's':
                move(t, DROP);
                if (checkCollision(t)) {
                    move(t, UP);
                }
                break;
            case 'w':
                move(t, ROTATE_CLOCKWISE);
                if (checkCollision(t)) {
                    move(t, ROTATE_COUNTER_CLOCKWISE);
                }
                break;
            case 'p':
                printf("\033[11;26H");
                printf("PAUSED");
                while (1) {
                    if (getch() == 'p') {
                        printf("\033[11;26H");
                        printf("      ");
                        break;
                    }
                }
                break;
            default:
                break;
        }
        drawTetromino(t);
        showBoard();
    }
}

void gameLoop(int interval) {
    int score = 0;
    Tetromino cur;
    Tetromino nxt;
    initTetromino(&nxt);
    cur = nxt;

    while (1) {
        for (int i = 0; i < ACTION_PER_FRAME; ++i) {
            responseAction(&cur);
        }
        clearTetromino(&cur);
        move(&cur, DOWN);
        if (cur.y == PRELOAD - 1) {
            initTetromino(&nxt);
            showNextTetromino(&nxt);
        }

        if (checkCollision(&cur)) {
            move(&cur, UP);
            drawTetromino(&cur);
            score += deleteLine() * SCORE_PER_LINE;
            printf("\033[8;26H");
            printf("SCORE:");
            printf("\033[9;26H");
            printf("%d", score);
            if (cur.y < PRELOAD) {
                printf("\033[13;27H");
                printf("\033[31mGAME");
                printf("\033[14;27H");
                printf("\033[31mOVER");
                break;
            }
            cur = nxt;
        }
        drawTetromino(&cur);
        showBoard();
        Sleep(interval);
    }
}
