# include "structs.h"
#include <vector>
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <ctime>
#include <ios>
#include <iomanip>
#include <climits>
#include <cassert>
#include <cmath>
#include <string>
#include <fstream>
#include <algorithm>
#include <stdlib.h>

int64_t range_query_baseline(const std::vector<int64_t>& array, int64_t min_bounds, int64_t max_bounds);
int64_t scanQuery(IndexEntry * crackerindex, int64_t min_bounds, int64_t max_bounds, int64_t initial_offset, int64_t final_offset);
void exchange(IndexEntry*& c, int64_t x1, int64_t x2);
int64_t find_median(IndexEntry * crackerindex, int64_t lower_limit, int64_t upper_limit);
void generate_partitions_order(int64_t * partitions, int64_t min, int64_t max);
int64_t scanQuery(IndexEntry * crackerindex,int64_t final_offset);