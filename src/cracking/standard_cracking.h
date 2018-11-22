#include "stdio.h"
#include <cstdlib>
#include "avl_tree.h"
#include "../structs.h"
#include "../util.h"

int64_t crackPieceWithBothQueryPredicate(IndexEntry*& c, int64_t posL, int64_t posH,  int64_t low, int64_t high, int64_t* sum, int64_t pivot);
int64_t scan_middle_pieces (IndexEntry*& c, int64_t posL, int64_t posH);
int64_t crackPieceWithLeftPredicate(IndexEntry*& c, int64_t posL, int64_t posH,  int64_t low, int64_t* sum, int64_t pivot);
int64_t crackPieceWithRightPredicate(IndexEntry*& c, int64_t posL, int64_t posH, int64_t high, int64_t* sum, int64_t pivot);
int64_t crackPieceOutsideQuery(IndexEntry*& c, int64_t posL, int64_t posH, int64_t pivot);
int64_t crackPieceMiddleQuery(IndexEntry*& c, int64_t posL, int64_t posH, int64_t* sum, int64_t pivot);
int64_t scan_left_piece (IndexEntry*& c, int64_t posL, int64_t posH, int64_t low);
int64_t scan_right_piece (IndexEntry*& c, int64_t posL, int64_t posH, int64_t high);
int64_t scan_left_right_piece (IndexEntry*& c, int64_t posL, int64_t posH, int64_t low, int64_t high);