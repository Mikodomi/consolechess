#ifndef CHESS_TYPES_H
#define CHESS_TYPES_H
#include <stdint.h>
typedef enum {
    KING = 'K',
    QUEEN = 'Q',
    ROOK = 'R',
    KNIGHT = 'N',
    BISHOP = 'B',
    PAWN = 'p',
    NONE = ' ',
} piece_type;

typedef enum {
    WHITE = 1,
    BLACK = -1,
    EMPTY_COLOR = 0,
} color_type;

typedef struct {
    piece_type type;
    color_type color;
    int8_t value;
    int8_t posRow;
    int8_t posColumn;
    int moves;
} Piece;

typedef struct {
    Piece pieces[8][8]; // [number][letter]
    // pieces are stored in the order: a1, a2, a3,..., b1, b2, b3,...
    int8_t black_pieces;
    int8_t white_pieces;
    Piece* last_moved;
} Board;

void printBoard(Board const* board);
void make_king_row(Board* board, color_type who);
Board starting_board();

Piece make_piece(Board* board, piece_type which, color_type who, int row, int column);

#endif