//
// Created by Pedro Holanda on 16/11/18.
//

#include "util.h"

extern int64_t NUM_QUERIES;
int64_t num_partitions = 0;
void exchange(IndexEntry*& c, int64_t x1, int64_t x2){
    IndexEntry tmp = *(c+x1);
    *(c+x1) = *(c+x2);
    *(c+x2) = tmp;
}

int64_t pivot(IndexEntry * crackerindex, int64_t low, int64_t high, int64_t pivot, int64_t pivot_position)
{
//  This method only works if we use the last element as the pivot
//  So we change the pivot to the last position
    exchange(crackerindex, pivot_position, high);

    int64_t i = low - 1;

    for (int64_t j = low; j < high; ++j)
    {
        if (crackerindex[j] <= pivot)
        {
            ++i;
            exchange(crackerindex, i, j);
        }
    }
    exchange(crackerindex, i + 1, high);
    return i + 1;
}

int64_t find_median(IndexEntry * crackerindex, int64_t lower_limit, int64_t upper_limit)
{
    int64_t low = lower_limit;
    int64_t high = upper_limit;
    int64_t position, element;

    do
    {
        element = crackerindex[(high+low)/2].m_key;
        position = pivot(crackerindex, low, high, element, (high+low)/2);

        if (position <= low)
        {
            ++low;
        }
        else if (position >= high)
        {
            --high;
        }
        else
        {
            if (position < (lower_limit + upper_limit) / 2)
                low = position;
            else
                high = position;
        }
    } while (position != (lower_limit + upper_limit) / 2);

    for (; position > lower_limit; --position)
    {
        if (crackerindex[position - 1] != crackerindex[position])
            break;
    }

    return element;
}

int64_t scanQuery(IndexEntry * crackerindex, int64_t min_bounds, int64_t max_bounds, int64_t initial_offset, int64_t final_offset) {
    int64_t sum = 0;
    for(size_t i = initial_offset; i <= final_offset; i++) {
        int matching =  (crackerindex[i].m_key >= min_bounds) && (crackerindex[i].m_key < max_bounds);
        sum += matching * crackerindex[i].m_key;
    }
    return sum;
}

int64_t range_query_baseline(const std::vector<int64_t>& array, int64_t min_bounds, int64_t max_bounds) {
    int64_t sum = 0;
    for(size_t i = 0; i < array.size(); i++) {
        int matching =  (array[i] >= min_bounds) && (array[i] < max_bounds);
        sum += matching * array[i];
    }
    return sum;
}

void generate_partitions_order(int64_t * partitions, int64_t min, int64_t max){
    int64_t medium = (max+min)/2;
    partitions[num_partitions++] = medium;
    if (num_partitions > NUM_QUERIES*2-1)
        return;
    generate_partitions_order(partitions , min, medium);
    generate_partitions_order(partitions , medium, max);

}
