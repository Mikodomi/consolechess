#ifndef CHESS_MOVE_CHECKING_H
#define CHESS_MOVE_CHECKING_H
#include "chess_types.h"


typedef enum {
   CORRECT,
   INVALID,
   NO_EXIST,
   WRONG_INPUT,
} MoveStatus;

typedef enum {
   MATE,
   STALEMATE,
   ONGOING,
   DRAW
} GameStatus;

MoveStatus check_move(Board* board, char* move, color_type who);

MoveStatus move_pawn(Board* board, char* move, color_type who, int checks); 
MoveStatus move_knight(Board* board, char* move, color_type who, int checks); 
MoveStatus move_rook(Board* board, char* move, color_type who, int checks); 
MoveStatus move_bishop(Board* board, char* move, color_type who, int checks); 
MoveStatus move_queen(Board* board, char* move, color_type who, int checks); 
MoveStatus move_king(Board* board, char* move, color_type who, int checks); 
MoveStatus move_castles(Board* board, char* move, color_type who, int checks);

int is_check(Board* board, color_type who); //checks if 'who' king is in check
int is_attacked(Board* board, int row, int column, color_type who);
GameStatus is_stale_mate(Board* board, color_type who); 
#endif