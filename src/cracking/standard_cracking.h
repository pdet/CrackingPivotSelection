#include "stdio.h"
#include <cstdlib>
#include "avl_tree.h"
#include "../structs.h"
#include "../util.h"

AvlTree standardCrackingWithinPiece(IndexEntry *&c, int64_t dataSize, AvlTree T, int64_t lowKey, int64_t highKey, QueryOutput *qo);
AvlTree standardCracking(IndexEntry*& c, int dataSize, AvlTree T, int lowKey, int highKey);