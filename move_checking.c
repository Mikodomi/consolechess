#include "move_checking.h"
#include <string.h>
MoveStatus check_move(Board* board, char* move, color_type who) {
    int checks = (move[strlen(move)-1] == '+');
    if (checks) {
        move[strlen(move)-1] = '\0';
    }
    switch(move[0]) {
        case 'N': {
            return move_knight(board, move+1, who, checks);
        }
        case 'B': {
            return move_bishop(board, move+1, who, checks);
        }
        case 'R': {
            return move_rook(board, move+1, who, checks);
        }
        case 'Q': {
            return move_queen(board, move+1, who, checks);
        }
        case 'K': {
            return move_king(board, move+1, who, checks);
        }
        case 'O': {
            return move_castles(board, move, who, checks);
        }
        case '\0': {
            return WRONG_INPUT;
        }
        default: {
            return move_pawn(board, move, who, checks);
        }
    }
}

static MoveStatus _move_piece(Board* board, Piece* current, int row, int column, int checks) {
    Piece curr = *current;
    Piece temp = board->pieces[row][column];
    if (temp.color == curr.color) {
        return INVALID;
    }
    current->posRow = row;
    current->posColumn = column;
    board->pieces[row][column] = *current;
    board->pieces[curr.posRow][curr.posColumn] = make_piece(board, NONE, EMPTY_COLOR, curr.posRow, curr.posColumn);
    int checking_white = is_check(board, WHITE);
    int checking_black = is_check(board, BLACK);
    if (((curr.color == WHITE) && (checking_white)) || 
        ((curr.color == BLACK) && (checking_black))) {
        board->pieces[row][column] = temp;
        board->pieces[curr.posRow][curr.posColumn] = curr;    
        return INVALID; 
    }
    if ((checking_white || checking_black) && !checks) {
        board->pieces[row][column] = temp;
        board->pieces[curr.posRow][curr.posColumn] = curr;    
        return INVALID; 
    }
    board->pieces[row][column].checking = checks;
    board->pieces[row][column].posRow = row;
    board->pieces[row][column].posColumn = column;
    board->pieces[row][column].moves++;
    switch (temp.color) {
        case WHITE:
            board->white_pieces--;
            break;
        case BLACK:
            board->black_pieces--;
            break;
    }
    board->last_moved = &(board->pieces[row][column]);
    return CORRECT;
}

static int in_bounds(int row, int column) {
    return (row >= 0) && (row <= 7) && (column >= 0) && (column <= 7);
}

static MoveStatus enpassant(Board* board, Piece* attacker, Piece* defender, int checks) {
    MoveStatus took = _move_piece(board, attacker, 
                                        (defender->posRow)+attacker->color,
                                        (defender->posColumn),
                                        checks);
    if (took == CORRECT) {
        board->pieces[defender->posRow][defender->posColumn] = make_piece(board, 
                                                                            NONE, EMPTY_COLOR, 
                                                                            defender->posRow, 
                                                                            defender->posColumn);
        return took;                                                                            
    }
    return took;
}

static Piece* math_pawn(Board* board,int row, int column, color_type who, int can_twice) {
    Piece* expected_pawn = &(board->pieces[row-who-(can_twice*who)][column]);
    if (expected_pawn->type != PAWN) {
        return NULL;
    }
    if (expected_pawn->color != who) {
        return NULL;
    }
    return expected_pawn;
}

// todo: enpassant

MoveStatus move_pawn(Board* board, char* move, color_type who, int checks) {
    int takes = 0;
    int column = move[0]-'a';
    
    int row;
    if (move[1] != 'x') {
        row = move[1]-'0';
    } else {
        takes = 1;
        row = move[3]-'0';
    }
    row--;
    if (!in_bounds(row, column)) {
        return WRONG_INPUT;
    }
    int can_twice = ((row == 3) && (who == WHITE)) || ((row == 4) && (who == BLACK));
    Piece* is_once = math_pawn(board, row, column, who, 0);
    Piece* is_twice = NULL;
    if (can_twice) {
        is_twice = math_pawn(board, row, column, who, can_twice);
    }
    
    if (!is_twice && !is_once) {
        return INVALID;
    }
    if (is_twice && takes && !is_once) {
        return INVALID;
    }
    if (is_twice && !takes) {
        return _move_piece(board, is_twice, row, column, checks);
    }
    if (is_once && !takes) {
        return _move_piece(board, is_once, row, column, checks);
    }
    if (board->pieces[row][(move[2]-'a')].type != NONE) {
        return _move_piece(board, is_once, row, move[2]-'a', checks);
    }
    // enpassant
    if ((who == WHITE && row == 5) || (who == BLACK && row == 2)) {
        Piece* is_enpassant = &(board->pieces[row-who][move[2]-'a']);
        if (board->last_moved == is_enpassant) {
            return enpassant(board, is_once, is_enpassant, checks);
        }
    }
    return INVALID;
}

static Piece* math_knight(Board* board,
                       int row,
                       int column,
                       int rowOffset,
                       int columnOffset, 
                       char* const move,
                       int len,
                       int takes,
                       color_type who) {
    if ((row + rowOffset <= 7) && (row + rowOffset >= 0) && (column+columnOffset <= 7) && (column+columnOffset >= 0)) {
        Piece* current = &(board->pieces[row + rowOffset][column + columnOffset]);
        if (current->type == KNIGHT && current->color == who) {
            if (len == 3 + takes) {
                if ((move[0] - '0' - 1 == row + rowOffset) || (move[0] - 'a' == column + columnOffset)) {
                    return current;
               }
            }
            else if (len == 4 + takes) {
                if ((move[1] - '0' - 1 == row + rowOffset) && (move[0] - 'a' == column + columnOffset)) {
                    return current;
                }
            }
            else {
                return current;
            }
        }
        return NULL;
    }
    return NULL;
}
 

static Piece* all_knight(Board* board,
                       int row,
                       int column, 
                       char* const move,
                       int len,
                       int takes,
                       color_type who) {
    int correctAmount = 0; 
    Piece* correctKnight = NULL;
    Piece* temp = math_knight(board, row, column, -2, -1, move, len, takes, who);
    correctAmount += (temp) ? 1 : 0;   
    correctKnight = (temp) ? temp : correctKnight;
    
    temp = math_knight(board, row, column, -2, 1, move, len, takes, who);
    correctAmount += (temp) ? 1 : 0;   
    correctKnight = (temp) ? temp : correctKnight;
    
    temp = math_knight(board, row, column, -1, 2, move, len, takes, who);
    correctAmount += (temp) ? 1 : 0;   
    correctKnight = (temp) ? temp : correctKnight;

    temp = math_knight(board, row, column, 1, 2, move, len, takes, who); 
    correctAmount += (temp) ? 1 : 0;   
    correctKnight = (temp) ? temp : correctKnight;

    temp = math_knight(board, row, column, 2, 1, move, len, takes, who);
    correctAmount += (temp) ? 1 : 0;   
    correctKnight = (temp) ? temp : correctKnight;

    temp = math_knight(board, row, column, 2, -1, move, len, takes, who);
    correctAmount += (temp) ? 1 : 0;   
    correctKnight = (temp) ? temp : correctKnight;

    temp = math_knight(board, row, column, 1, -2, move, len, takes, who);
    correctAmount += (temp) ? 1 : 0;   
    correctKnight = (temp) ? temp : correctKnight;

    temp = math_knight(board, row, column, -1, -2, move, len, takes, who);
    correctAmount += (temp) ? 1 : 0;   
    correctKnight = (temp) ? temp : correctKnight;
    return (correctAmount == 1) ? correctKnight : NULL;
}
MoveStatus move_knight(Board* board, char* move, color_type who, int checks) {
    int len = strlen(move);
    Piece* correctKnight;
    int column = move[len-2] - 'a';
    int row = move[len-1] - '0'-1;
    int takes = (move[len-3] == 'x');

    if (column > 7 || column < 0) {
        return WRONG_INPUT;
    }
    if (row > 7 || row < 0) {
        return WRONG_INPUT;
    }
    
    if (takes && (board->pieces[row][column].type == NONE)) { 
        return INVALID;
    } 

    // this is so ugly but i think its still the cleanest
    // edit: not so ugly anymore but still pretty ugly
    correctKnight = all_knight(board, row, column, move, len, takes, who);
    if (correctKnight != NULL) {
        return _move_piece(board, correctKnight, row, column, checks);
    }
    return INVALID;
}

static Piece* linear_math(Board* board, 
                       int row,
                       int column,
                       int rowDir,
                       int columnDir,
                       char* const move,
                       int len,
                       int takes,
                       color_type who,
                       piece_type what) 
{
    row += rowDir;
    column += columnDir;
    int8_t isKing = 1;
    while (in_bounds(row, column) && isKing) {
        if (board->pieces[row][column].type != NONE) {
            break;
        }
        if (what == KING) {
            break;
        }
        // isKing = 0;
        row += rowDir;
        column += columnDir;
    } 
    if (!in_bounds(row, column)) {
        return NULL;
    }
    Piece* current = &(board->pieces[row][column]);
    if (current->type == what && current->color == who) {
        if (isKing && is_check(board, who)) {
            return NULL;
        }
        if (len == 3 + takes) {
            if ((row == move[0] - '0' - 1) || (column == move[0] - 'a')) {
                return current;
            }
        } else if (len == 4 + takes) {
            if ((row == move[1] - '0' - 1) && (column == move[0] - 'a')) {
                return current;
            }
        } else {
            return current;
        }
    }
    return NULL;
}

static Piece* all_bishop(Board* board, 
                       int row,
                       int column,
                       char* const move,
                       int len,
                       int takes,
                       color_type who) {
    int correctAmount = 0;
    Piece* correctBishop = NULL; 
    Piece* temp = linear_math(board, row, column, -1, -1, move, len, takes, who, BISHOP);
    correctAmount += (temp) ? 1 : 0;
    correctBishop = (temp) ? temp : correctBishop;

    temp = linear_math(board, row, column, -1, 1, move, len, takes, who, BISHOP);
    correctAmount += (temp) ? 1 : 0;
    correctBishop = (temp) ? temp : correctBishop;

    temp = linear_math(board, row, column, 1, 1, move, len, takes, who, BISHOP);
    correctAmount += (temp) ? 1 : 0;
    correctBishop = (temp) ? temp : correctBishop;

    temp = linear_math(board, row, column, 1, -1, move, len, takes, who, BISHOP);
    correctAmount += (temp) ? 1 : 0;
    correctBishop = (temp) ? temp : correctBishop;
    return (correctAmount == 1) ? correctBishop : NULL;
}

MoveStatus move_bishop(Board* board, char* move, color_type who, int checks) {
    int len = strlen(move);
    Piece* correctBishop;
    int column = move[len-2] - 'a';
    int row = move[len-1] - '0'-1;
    int takes = (move[len-3] == 'x');

    if (column > 7 || column < 0) {
        return WRONG_INPUT;
    }
    if (row > 7 || row < 0) {
        return WRONG_INPUT;
    }
    
    if (takes && ((board->pieces[row][column].type == NONE) || (board->pieces[row][column].color == who))) { 
        return INVALID;
    } 
    correctBishop = all_bishop(board, row, column, move, len, takes, who);

    if (correctBishop != NULL) {
        return _move_piece(board, correctBishop, row, column, checks);
    }
    return INVALID;
}

static Piece* all_rook(Board* board, 
                       int row,
                       int column,
                       char* const move,
                       int len,
                       int takes,
                       color_type who) {
    int correctAmount = 0;
    Piece* correctRook = NULL; 
    Piece* temp = linear_math(board, row, column, 1, 0, move, len, takes, who, ROOK);
    correctAmount += (temp) ? 1 : 0;
    correctRook = (temp) ? temp : correctRook;

    temp = linear_math(board, row, column, -1, 0, move, len, takes, who, ROOK);
    correctAmount += (temp) ? 1 : 0;
    correctRook = (temp) ? temp : correctRook;

    temp = linear_math(board, row, column, 0, 1, move, len, takes, who, ROOK);
    correctAmount += (temp) ? 1 : 0;
    correctRook = (temp) ? temp : correctRook;

    temp = linear_math(board, row, column, 0, -1, move, len, takes, who, ROOK);
    correctAmount += (temp) ? 1 : 0;
    correctRook = (temp) ? temp : correctRook;
    return (correctAmount == 1) ? correctRook : NULL;
}

MoveStatus move_rook(Board* board, char* move, color_type who, int checks) {
    int len = strlen(move);
    Piece* correctRook = 0;
    int column = move[len-2] - 'a';
    int row = move[len-1] - '0'-1;
    int takes = (move[len-3] == 'x');

    if (column > 7 || column < 0) {
        return WRONG_INPUT;
    }
    if (row > 7 || row < 0) {
        return WRONG_INPUT;
    }
    
    if (takes && ((board->pieces[row][column].type == NONE) || (board->pieces[row][column].color == who))) { 
        return INVALID;
    } 

    correctRook = all_rook(board, row, column, move, len, takes, who);

    if (correctRook != NULL) {
        return _move_piece(board, correctRook, row, column, checks);
    } 
    return INVALID;
}

static Piece* all_queen(Board* board, 
                       int row,
                       int column,
                       char* const move,
                       int len,
                       int takes,
                       color_type who) {
    int correctAmount = 0;
    Piece* correctQueen = NULL;
    // vertical and horizontal
    Piece* temp = linear_math(board, row, column, 1, 0, move, len, takes, who, QUEEN);
    correctAmount += (temp) ? 1 : 0;
    correctQueen = (temp) ? temp : correctQueen;

    temp = linear_math(board, row, column, -1, 0, move, len, takes, who, QUEEN);
    correctAmount += (temp) ? 1 : 0;
    correctQueen = (temp) ? temp : correctQueen;

    temp = linear_math(board, row, column, 0, 1, move, len, takes, who, QUEEN);
    correctAmount += (temp) ? 1 : 0;
    correctQueen = (temp) ? temp : correctQueen;

    temp = linear_math(board, row, column, 0, -1, move, len, takes, who, QUEEN);
    correctAmount += (temp) ? 1 : 0;
    correctQueen = (temp) ? temp : correctQueen;
    // diagonal
    temp = linear_math(board, row, column, -1, -1, move, len, takes, who, QUEEN);
    correctAmount += (temp) ? 1 : 0;
    correctQueen = (temp) ? temp : correctQueen;

    temp = linear_math(board, row, column, -1, 1, move, len, takes, who, QUEEN);
    correctAmount += (temp) ? 1 : 0;
    correctQueen = (temp) ? temp : correctQueen;

    temp = linear_math(board, row, column, 1, 1, move, len, takes, who, QUEEN);
    correctAmount += (temp) ? 1 : 0;
    correctQueen = (temp) ? temp : correctQueen;

    temp = linear_math(board, row, column, 1, -1, move, len, takes, who, QUEEN);
    correctAmount += (temp) ? 1 : 0;
    correctQueen = (temp) ? temp : correctQueen;

    return (correctAmount == 1) ? correctQueen: NULL;

}

MoveStatus move_queen(Board* board, char* move, color_type who, int checks) {
    int len = strlen(move);
    Piece* correctQueen;
    int column = move[len-2] - 'a';
    int row = move[len-1] - '0'-1;
    int takes = (move[len-3] == 'x');

    if (column > 7 || column < 0) {
        return WRONG_INPUT;
    }
    if (row > 7 || row < 0) {
        return WRONG_INPUT;
    }
    
    if (takes && ((board->pieces[row][column].type == NONE) || (board->pieces[row][column].color == who))) { 
        return INVALID;
    } 

    correctQueen = all_queen(board, row, column, move, len, takes, who);

    if (correctQueen != NULL) {
        return _move_piece(board, correctQueen, row, column, checks);
    }
    return INVALID;
}

static Piece* all_king(Board* board, 
                       int row,
                       int column,
                       char* const move,
                       int len,
                       int takes,
                       color_type who) {
    int correctAmount = 0;
    Piece* correctKing = NULL;
    // vertical and horizontal
    Piece* temp = linear_math(board, row, column, 1, 0, move, len, takes, who, KING);
    correctAmount += (temp) ? 1 : 0;
    correctKing = (temp) ? temp : correctKing;

    temp = linear_math(board, row, column, -1, 0, move, len, takes, who, KING);
    correctAmount += (temp) ? 1 : 0;
    correctKing = (temp) ? temp : correctKing;

    temp = linear_math(board, row, column, 0, 1, move, len, takes, who, KING);
    correctAmount += (temp) ? 1 : 0;
    correctKing = (temp) ? temp : correctKing;

    temp = linear_math(board, row, column, 0, -1, move, len, takes, who, KING);
    correctAmount += (temp) ? 1 : 0;
    correctKing = (temp) ? temp : correctKing;
    // diagonal
    temp = linear_math(board, row, column, -1, -1, move, len, takes, who, KING);
    correctAmount += (temp) ? 1 : 0;
    correctKing = (temp) ? temp : correctKing;

    temp = linear_math(board, row, column, -1, 1, move, len, takes, who, KING);
    correctAmount += (temp) ? 1 : 0;
    correctKing = (temp) ? temp : correctKing;

    temp = linear_math(board, row, column, 1, 1, move, len, takes, who, KING);
    correctAmount += (temp) ? 1 : 0;
    correctKing = (temp) ? temp : correctKing;

    temp = linear_math(board, row, column, 1, -1, move, len, takes, who, KING);
    correctAmount += (temp) ? 1 : 0;
    correctKing = (temp) ? temp : correctKing;

    return (correctAmount == 1) ? correctKing: NULL;
}


MoveStatus move_king(Board* board, char* move, color_type who, int checks) {
    int len = strlen(move);
    Piece* correctKing;
    int column = move[len-2] - 'a';
    int row = move[len-1] - '0'-1;
    int takes = (move[len-3] == 'x');

    if (column > 7 || column < 0) {
        return WRONG_INPUT;
    }
    if (row > 7 || row < 0) {
        return WRONG_INPUT;
    }
    
    if (takes && ((board->pieces[row][column].type == NONE) || (board->pieces[row][column].color == who))) { 
        return INVALID;
    } 

    correctKing = all_king(board, row, column, move, len, takes, who);

    if (correctKing != NULL) {
        return _move_piece(board, correctKing, row, column, checks);
    }
    return INVALID;
}

int is_check(Board* board, color_type who) { //checks if 'who' king is in check
    Piece* current;
    int found = 0;
    int row = 0;
    int column = 0;
    for (int row = 0; row<8 && !found; row++) {
        for (int column = 0; column<8; column++) {
            current = &(board->pieces[row][column]);
            if (current->type == KING && current->color == who) {
                found = 1;
                break;
            }
        }
    }
    if (!found) {
        return -1; // king not on the board
    }
    char move[3];
    move[0] = current->posColumn + 'a';
    move[1] = current->posRow + '0' + 1;
    // Piece* pawn1 = math_pawn(board, current->posRow, current->posColumn, -who, 0); 
    // Piece* pawn2 = math_pawn(board, current->posRow, current->posColumn, -who, 1);
    Piece* pawn1 = &(board->pieces[current->posRow+who][current->posColumn-1]);
    if (pawn1->type != PAWN) {
        pawn1 = NULL;
    }
    if (pawn1 && pawn1->color == who) {
        pawn1 = NULL;
    }
    Piece* pawn2 = &(board->pieces[current->posRow+who][current->posColumn+1]);
    if (pawn2->type != PAWN) {
        pawn2 = NULL;
    }
    if (pawn2 && pawn2->color == who) {
        pawn2 = NULL;
    }
    
    Piece* knights = all_knight(board, current->posRow, current->posColumn, move, 2, 0, -who);
    Piece* bishops = all_bishop(board, current->posRow, current->posColumn, move, 2, 0, -who);
    Piece* rooks = all_rook(board, current->posRow, current->posColumn, move, 2, 0, -who);
    Piece* queens = all_queen(board, current->posRow, current->posColumn, move, 2, 0, -who);
    return ((pawn1 != NULL) +
            (pawn2 != NULL) + 
            (knights != NULL) +
            (bishops != NULL) +
            (rooks != NULL) +
            (queens != NULL));
}
//todo
//generalised version of is_check for any square (used primarily in is_mate (horrible code btw))
//checks if the provided coordinates are attacked by the opposite color
int is_attacked(Board* board, int row, int column, color_type who) {
    if (row < 0 || row > 7) {
        return 1; // can't get there so its the same as being attacked (from kings perspective)
    }
    if (column < 0 || column > 7) {
        return 1; // same as above
    }
    char move[3];
    move[0] = column + 'a';
    move[1] = row + '0' + 1;
    Piece* pawn1 = math_pawn(board, row, column, -who, 0); 
    Piece* pawn2 = math_pawn(board, row, column, -who, 1);
    Piece* knights = all_knight(board, row, column, move, 2, 0, -who);
    Piece* bishops = all_bishop(board, row, column, move, 2, 0, -who);
    Piece* rooks = all_rook(board, row, column, move, 2, 0, -who);
    Piece* queens = all_queen(board, row, column, move, 2, 0, -who);
    return (pawn1 != NULL ||
            pawn2 != NULL || 
            knights != NULL ||
            bishops != NULL ||
            rooks != NULL ||
            queens != NULL);
}

// looks for piece that is checking 'who' king
// function should only return the pointer of 1 piece since
// it only gets called under the condition that
// 1) king is in check
// 2) checked by one piece
// this has bugs in it for sure. who cares....
static Piece* find_checker(Board* board, color_type who) {
    int row = 0;
    int column = 0;
    int found = 0;
    for (; row < 8 && !found; row++) {
        for (; column < 8; column++) {
            if (board->pieces[row][column].checking && board->pieces[row][column].color == -who) {
                return &(board->pieces[row][column]);
            }
        }
    }
    return NULL;
}

// todo: fix mate conditions
// in its current state, the function doesn't take into consideration
// the possibility of blocking a check or taking the piece that is attacking the king
// which is wrong
GameStatus is_stale_mate(Board* board, color_type who) {
    int row = 0;
    int column = 0;
    int found = 0;
    int attackedSquares = 0;
    for (; row < 8 && !found; row++) {
        for (; column < 8 && !found; column++) {
            Piece current = board->pieces[row][column];
            if (current.type == KING && current.color == who) {
                break;
            }
        }
    }
    attackedSquares += is_attacked(board, row+1, column, who);
    attackedSquares += is_attacked(board, row-1, column, who);
    attackedSquares += is_attacked(board, row, column+1, who);
    attackedSquares += is_attacked(board, row, column-1, who);
    attackedSquares += is_attacked(board, row+1, column+1, who);
    attackedSquares += is_attacked(board, row+1, column-1, who);
    attackedSquares += is_attacked(board, row-1, column+1, who);
    attackedSquares += is_attacked(board, row-1, column-1, who);
    if (attackedSquares != 8) {
        return ONGOING;
    }
    int checking = is_check(board, who);
    if (checking > 1) {
        return MATE;
    }
    if (checking == 0) {
        switch(who) {
            case WHITE:
                if (board->white_pieces == 1) {
                    return STALEMATE;
                }
            case BLACK:
                if (board->black_pieces == 1) {
                    return STALEMATE;
                }
        }
    }
    Piece* attacker = find_checker(board, who);
    if (attacker == NULL) {
        return ONGOING;
    }
    if (attacker->type == KNIGHT) {
        return MATE;
    }
    if (is_attacked(board, attacker->posRow, attacker->posColumn, who)) {
        return ONGOING;
    }
    if (attacker->type == PAWN) {
        if (is_attacked(board, attacker->posRow, attacker->posColumn, who)) {
            return MATE; 
        }
    }
}

static int math_castle(Board* board, color_type who, int len) {
    Piece* king = &(board->pieces[(who == WHITE) ? 0 : 7][4]);
    if (king->type != KING) {
        return 0;
    }
    if (king->color != who) {
        return 0;
    }
    if (king->moves > 0) {
        return 0;
    }
    Piece* rook = &(board->pieces[(who == WHITE ? 0 : 7)][(len == 3) ? 7 : 0]);
    if (rook->type != ROOK || rook->color != who || rook->moves > 0) {
        return 0;
    }
    if (len == 3) {
        if (board->pieces[((who == WHITE) ? 0 : 7)][5].type != NONE || 
            board->pieces[((who == WHITE) ? 0 : 7)][6].type != NONE) {
            return 0;
        }
        if (is_attacked(board, ((who == WHITE) ? 0 : 7), 5, who) || 
            is_attacked(board, ((who == WHITE) ? 0 : 7), 6, who)) {
            return 0;
        }
    } else {
        if (board->pieces[((who == WHITE) ? 0 : 7)][1].type != NONE ||
            board->pieces[((who == WHITE) ? 0 : 7)][2].type != NONE || 
            board->pieces[((who == WHITE) ? 0 : 7)][3].type != NONE) {
                return 0;
        }
        if (is_attacked(board, ((who == WHITE) ? 0 : 7), 1, who) ||
            is_attacked(board, ((who == WHITE) ? 0 : 7), 2, who) ||
            is_attacked(board, ((who == WHITE) ? 0 : 7), 3, who)) {
                return 0;
            }
    }
    return 1;
}

MoveStatus move_castles(Board* board, char* move, color_type who, int checks) {
    int len = strlen(move);
    // ugly comparison of strings... who cares...
    if (move[0] != 'O' || move[2] != 'O') {
        return WRONG_INPUT;
    }
    if (move[1] != '-') {
        return WRONG_INPUT;
    }
    if (len == 5) {
        if (move[3] != '-' || move[4] != 'O') {
            return WRONG_INPUT;
        }
    }
    int can_castle = math_castle(board, who, len);
    if (!can_castle) {
        return INVALID;
    }
    Piece* king = &(board->pieces[(who == WHITE) ? 0 : 7][4]);
    Piece* rook = &(board->pieces[(who == WHITE) ? 0 : 7][(len == 3) ? 7 : 0]);
    if (_move_piece(board, king, ((who == WHITE) ? 0 : 7), ((len == 3) ? 5 : 2), checks) == INVALID) {
        return INVALID;
    }
    if (_move_piece(board, rook, ((who == WHITE) ? 0 : 7), ((len == 3) ? 4 : 3), checks) == INVALID) {
        _move_piece(board, king, ((who == WHITE) ? 0 : 7), 4, checks);
        return INVALID;
    }
    return CORRECT;
    }