#pragma once

#include <queue>
#include <climits>
#include <algorithm>

struct Column {
    std::vector<int64_t> data;
};

struct RangeQuery {
    std::vector<int64_t> leftpredicate;
    std::vector<int64_t> rightpredicate;
};
