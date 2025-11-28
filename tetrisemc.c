#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tetris_stack.h"

Piece pecas[NUM_PIECES] = {
    {{{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}}, 4, 1},
    {{{1,1,0,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}}, 2, 2},
    {{{0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}}, 3, 2},
    {{{1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}}, 3, 2},
    {{{0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}}, 3, 2},
    {{{0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}}, 3, 2},
    {{{1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}}, 3, 2}
};

Piece obterPecaAleatoria() {
    static int inicializado = 0;
    if (!inicializado) {
        srand(time(NULL));
        inicializado = 1;
    }
    return pecas[rand() % NUM_PIECES];
}

TetrisGame* criarJogo() {
    TetrisGame *game = (TetrisGame*)malloc(sizeof(TetrisGame));
    if (game == NULL) {
        printf("Erro ao alocar memoria para o jogo!\n");
        return NULL;
    }
    
    game->score = 0;
    game->gameOver = 0;
    game->queueFront = 0;
    
    inicializarTabuleiro(game);
    
    for (int i = 0; i < QUEUE_SIZE; i++) {
        game->pieceQueue[i] = obterPecaAleatoria();
    }
    
    gerarProximaPeca(game);
    return game;
}

void destruirJogo(TetrisGame *game) {
    if (game != NULL) {
        free(game);
    }
}

void inicializarTabuleiro(TetrisGame *game) {
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            game->board[i][j] = EMPTY;
        }
    }
}

void gerarProximaPeca(TetrisGame *game) {
    Piece proximaPeca = game->pieceQueue[game->queueFront];
    
    game->queueFront = (game->queueFront + 1) % QUEUE_SIZE;
    
    game->pieceQueue[(game->queueFront + QUEUE_SIZE - 1) % QUEUE_SIZE] = obterPecaAleatoria();
    
    memcpy(game->currentPiece, proximaPeca.shape, sizeof(proximaPeca.shape));
    
    game->currentX = WIDTH / 2 - proximaPeca.width / 2;
    game->currentY = 0;
    
    if (verificarColisao(game, game->currentX, game->currentY, game->currentPiece)) {
        game->gameOver = 1;
    }
}

int verificarColisao(TetrisGame *game, int newX, int newY, int piece[4][4]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (piece[i][j]) {
                int boardX = newX + j;
                int boardY = newY + i;
                
                if (boardX < 0 || boardX >= WIDTH || boardY >= MAX_HEIGHT) {
                    return 1;
                }
                
                if (boardY >= 0 && game->board[boardY][boardX]) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

void fixarPeca(TetrisGame *game) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (game->currentPiece[i][j]) {
                int boardX = game->currentX + j;
                int boardY = game->currentY + i;
                
                if (boardY >= 0 && boardY < MAX_HEIGHT && boardX >= 0 && boardX < WIDTH) {
                    game->board[boardY][boardX] = FILLED;
                }
            }
        }
    }
    
    int linhasRemovidas = verificarLinhasCompletas(game);
    
    switch (linhasRemovidas) {
        case 1: game->score += 100; break;
        case 2: game->score += 300; break;
        case 3: game->score += 500; break;
        case 4: game->score += 800; break;
    }
    
    gerarProximaPeca(game);
}

int verificarLinhasCompletas(TetrisGame *game) {
    int linhasCompletas = 0;
    
    for (int i = MAX_HEIGHT - 1; i >= 0; i--) {
        int linhaCompleta = 1;
        
        for (int j = 0; j < WIDTH; j++) {
            if (!game->board[i][j]) {
                linhaCompleta = 0;
                break;
            }
        }
        
        if (linhaCompleta) {
            removerLinha(game, i);
            linhasCompletas++;
            i++;
        }
    }
    
    return linhasCompletas;
}

void removerLinha(TetrisGame *game, int linha) {
    for (int i = linha; i > 0; i--) {
        for (int j = 0; j < WIDTH; j++) {
            game->board[i][j] = game->board[i-1][j];
        }
    }
    
    for (int j = 0; j < WIDTH; j++) {
        game->board[0][j] = EMPTY;
    }
}

void rotacionarPeca(TetrisGame *game) {
    int temp[4][4];
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            temp[j][3-i] = game->currentPiece[i][j];
        }
    }
    
    if (!verificarColisao(game, game->currentX, game->currentY, temp)) {
        memcpy(game->currentPiece, temp, sizeof(temp));
    }
}

void moverPeca(TetrisGame *game, int dx, int dy) {
    int newX = game->currentX + dx;
    int newY = game->currentY + dy;
    
    if (!verificarColisao(game, newX, newY, game->currentPiece)) {
        game->currentX = newX;
        game->currentY = newY;
    } else if (dy > 0) {
        fixarPeca(game);
    }
}

void desenharJogo(TetrisGame *game) {
    system("clear");
    
    printf("=== TETRIS STACK - Vanessa Coco ===\n");
    printf("Pontuacao: %d\n\n", game->score);
    
    int tempBoard[MAX_HEIGHT][WIDTH];
    memcpy(tempBoard, game->board, sizeof(game->board));
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (game->currentPiece[i][j]) {
                int boardX = game->currentX + j;
                int boardY = game->currentY + i;
                
                if (boardY >= 0 && boardY < MAX_HEIGHT && boardX >= 0 && boardX < WIDTH) {
                    tempBoard[boardY][boardX] = FILLED;
                }
            }
        }
    }
    
    printf("+");
    for (int j = 0; j < WIDTH; j++) printf("-");
    printf("+\n");
    
    for (int i = 0; i < MAX_HEIGHT; i++) {
        printf("|");
        for (int j = 0; j < WIDTH; j++) {
            if (tempBoard[i][j]) {
                printf("#");
            } else {
                printf(" ");
            }
        }
        printf("|\n");
    }
    
    printf("+");
    for (int j = 0; j < WIDTH; j++) printf("-");
    printf("+\n");
    
    printf("\nControles: A (esquerda), D (direita), S (baixo), R (rotacionar), Q (sair)\n");
    
    if (game->gameOver) {
        printf("\n*** GAME OVER! ***\n");
        printf("Pontuacao final: %d\n", game->score);
    }
}

void imprimirTabuleiro(TetrisGame *game) {
    printf("\n=== ESTADO DO TABULEIRO ===\n");
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            printf("%d ", game->board[i][j]);
        }
        printf("\n");
    }
}

int main() {
    printf("=== DESAFIO TETRIS STACK ===\n");
    printf("Desenvolvido por: Vanessa Coco\n");
    printf("Matricula: 202302001301\n");
    printf("GitHub: VanessaCoco\n");
    printf("Email: santos.vanessacoco@outlook.com.br\n\n");
    
    TetrisGame *game = criarJogo();
    
    if (game == NULL) {
        return 1;
    }
    
    char comando;
    
    while (!game->gameOver) {
        desenharJogo(game);
        
        printf("Comando: ");
        scanf(" %c", &comando);
        
        switch (comando) {
            case 'a':
            case 'A':
                moverPeca(game, -1, 0);
                break;
            case 'd':
            case 'D':
                moverPeca(game, 1, 0);
                break;
            case 's':
            case 'S':
                moverPeca(game, 0, 1);
                break;
            case 'r':
            case 'R':
                rotacionarPeca(game);
                break;
            case 'q':
            case 'Q':
                printf("Saindo do jogo...\n");
                game->gameOver = 1;
                break;
            case 'p':
            case 'P':
                imprimirTabuleiro(game);
                printf("Pressione Enter para continuar...");
                getchar(); getchar();
                break;
            default:
                printf("Comando invalido!\n");
        }
    }
    
    desenharJogo(game);
    destruirJogo(game);
    
    return 0;
}