#include <stdio.h>
#include "chess_types.h"


Piece make_piece(Board* board, piece_type which, color_type who, int row, int column) {
    Piece current;
    current.type = which;
    current.color = who;
    current.moves = 0;
    switch (who) {
        case WHITE: 
            board->white_pieces++;
            break;
        case BLACK:
            board->black_pieces++;
            break;
    }
    current.posRow = row;
    current.posColumn = column;
    int value;
    switch (which) {
        case PAWN: 
            value = 1;
            break;
        case KNIGHT:
        case BISHOP:
            value = 3;
            break;
        case ROOK:
            value = 5;
            break;
        case QUEEN:
            value = 9;
            break;
        default:
            value = 0;
            break;
    }
    current.value = value;
    return current;
}

void make_king_row(Board* board, color_type who) {
    int column = 0;
    int row = (who == WHITE) ? 0 : 7;
    board->pieces[row][column] = make_piece(board, ROOK, who, row, column);
    column++;
    board->pieces[row][column] = make_piece(board, KNIGHT, who, row, column);
    column++;
    board->pieces[row][column] = make_piece(board, BISHOP, who, row, column);
    column++;
    board->pieces[row][column] = make_piece(board, QUEEN, who, row, column);
    column++;
    board->pieces[row][column] = make_piece(board, KING, who, row, column);
    column++;
    board->pieces[row][column] = make_piece(board, BISHOP, who, row, column);
    column++;
    board->pieces[row][column] = make_piece(board, KNIGHT, who, row, column);
    column++;
    board->pieces[row][column] = make_piece(board, ROOK, who, row, column);
}

Board empty_board() {
    Board board;
    board.white_pieces = 0;
    board.black_pieces = 0;
    for (int row = 0; row < 8; row++) {
        for (int column = 0; column < 8; column++) {
            Piece temp = make_piece(&board, NONE, EMPTY_COLOR, row, column);
            board.pieces[row][column] = temp; 
        }
    }
    return board;
}
