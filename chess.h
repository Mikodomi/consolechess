#ifndef CHESS_MAIN_H
#define CHESS_MAIN_H
#include "chess_types.h"
#include "move_checking.h"

void printBoard(Board const* board); 
Board starting_board();
Board empty_board();
MoveStatus game_turn(Board* board, color_type who);
int game();
#endif