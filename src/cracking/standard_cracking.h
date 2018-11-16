#include "stdio.h"
#include <cstdlib>
#include "avl_tree.h"
#include "../structs.h"
#include "../util.h"

AvlTree standardCracking(IndexEntry*& c, int dataSize, AvlTree T, int lowKey, int highKey);
