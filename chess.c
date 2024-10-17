#include "chess.h"
#include <stdio.h>
#include <string.h>
#define RESET   "\033[0m"  // Reset to default color 
#define RED_C     "\033[31m" // Red text
#define WHITE_C   "\033[37m" // White text
#define CLEAR_SCREEN "\e[1;1H\e[2J" // clears the terminal screen  
void printBoard(Board const* board) {
    printf("\n   _   _   _   _   _   _   _   _");
    for (int row = 7; row >= 0; row--) {
        printf("\n%d| ", row+1);
        for (int column = 0; column < 8; column++) {
            Piece const* current_piece = &(board->pieces[row][column]);
            switch(current_piece->color) {
                case WHITE: {
                    printf(WHITE_C);
                    break;
                }
                case BLACK: {
                    printf(RED_C);
                    break;
                }
                default: {
                    printf("");
                }
            }            
            printf("%c"RESET,current_piece->type);
            printf(" | ");
        }
        printf("\n |---+---+---+---+---+---+---+---|");
    }
    printf("\n   a   b   c   d   e   f   g   h\n");
  
}

Board starting_board() {
    Board board;
    board.black_pieces = 8;
    board.white_pieces = 8;
    board.status = ONGOING;
    // white pieces first
    color_type who = WHITE;
    int row = 0;
    int column = 0;
    make_king_row(&board, WHITE);
    row = 1;
    column = 0;
    for (; column < 8; column++) {
        board.pieces[row][column] = make_piece(&board, PAWN, who, row, column);
    }
    row++;
    column = 0;
    for (; row<6; row++) {
        for (; column<8; column++) {
            board.pieces[row][column] = make_piece(&board, NONE, EMPTY_COLOR, row, column);
        }
        column = 0;
    }
    who = BLACK;
    for (; column < 8; column++) {
        board.pieces[row][column] = make_piece(&board, PAWN, who, row, column);
    }
    row++;
    make_king_row(&board, BLACK);
    return board;
}

MoveStatus game_turn(Board* board, color_type who) {
    char move[9];
    MoveStatus done;
    printf(CLEAR_SCREEN);
    printBoard(board);
    switch(who) {
        case WHITE:
            printf("WHITE");
            break;
        case BLACK:
            printf("BLACK");
            break;
        default:
            printf("GAME OVER"); //presumably
    }
    printf(" TO MOVE\n");
    do {
        fgets(move, 8, stdin);
        move[strlen(move)-1] = '\0';
        // ugly string comparison... who cares....
        if ((move[0] == 'r') && (move[1] == 'e') && (move[2] == 's')) {
            board->status = RESIGNED; 
            done = CORRECT;
            break;
        }
        done = check_move(board, move, who);
    } while (done != CORRECT);
    return done;
}

int game() {
    Board board = starting_board();
    GameStatus status = ONGOING;
    color_type who = WHITE;
    do {
        game_turn(&board, who);
        who *= -1;
        status = is_stale_mate(&board, who);
    } while (status == ONGOING);
    printBoard(&board);
    if (status == RESIGNED) {
        switch (who) {
            case WHITE: {
                printf("White");
                break;
            }
            case BLACK: {
                printf("Black");
                break;
            }
            default: {
                printf("Nobody");
            }
        }
        printf(" has won by resignation\n");
    }
    if (status == MATE) {
        switch (-who) {
            case WHITE: {
                printf("White");
                break;
            }
            case BLACK: {
                printf("Black");
                break;
            }
            default: {
                printf("Nobody ");
            }
        }
        printf(" has won");
        printf("\n");
    }
    getc(stdin);
}
