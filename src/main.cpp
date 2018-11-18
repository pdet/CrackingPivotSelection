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
#include <queue>
#include "structs.h"
#include "data/file_manager.h"
#include "cracking/avl_tree.h"
#include "cracking/standard_cracking.h"

#pragma clang diagnostic ignored "-Wformat"

//Settings Pivot Types
const int PIVOT_WORKLOAD = 0;
const int PIVOT_RANDOM_WITHIN_PIECE = 1;
const int PIVOT_RANDOM_MEDIAN_WITHIN_PIECE = 2;
const int PIVOT_MEDIAN_WITHIN_PIECE = 3;
const int PIVOT_RANDOM = 4;
const int PIVOT_RANDOM_MEDIAN = 5;
const int PIVOT_MEDIAN = 6;

string COLUMN_FILE_PATH, QUERIES_FILE_PATH, ANSWER_FILE_PATH;
int64_t  COLUMN_SIZE,NUM_QUERIES;
int PIVOT_TYPE;
// #define DEBUG

using namespace std;

void full_scan(Column& column, RangeQuery& rangeQueries, vector<int64_t> &answers, vector<double>& time) {
    chrono::time_point<chrono::system_clock> start, end;
    for (int i = 0; i < NUM_QUERIES; i++) {
        start = chrono::system_clock::now();
        int64_t sum = range_query_baseline(column.data, rangeQueries.leftpredicate[i], rangeQueries.rightpredicate[i]);
        end = chrono::system_clock::now();
        time[i] = chrono::duration<double>(end - start).count();
        if (sum != answers[i]){
            fprintf(stderr, "Incorrect Results on query %lld\n Expected : %lld    Got : %lld \n", i, answers[i], sum);
        }
#ifndef DEBUG
        cout << time[i] << "\n";
#endif
    }
}

AvlTree cracking_median(AvlTree T,IndexEntry * crackerindex,  std::queue<int_pair> * median){
    int64_t median_value =  find_median(crackerindex,  median->front().first,  median->front().second);
    T = standardCracking(crackerindex , COLUMN_SIZE, T, median_value,  -1);
    int64_t median_offset = lookup(median_value,T);
    median->push({median->front().first,median_offset});
    median->push({median_offset,median->front().second});
    median->pop();
    return T;

}

AvlTree cracking_median_random(AvlTree T,IndexEntry * crackerindex,  std::queue<int_pair> * median){
    int64_t median_value =  crackerindex[(median->front().first +median->front().second)/2].m_key;
    T = standardCracking(crackerindex , COLUMN_SIZE, T, median_value,  -1);
    int64_t median_offset = lookup(median_value,T);
    median->push({median->front().first,median_offset});
    median->push({median_offset,median->front().second});
    median->pop();
    return T;

}

AvlTree pivot_selection(AvlTree T,IndexEntry * crackerindex,  std::queue<int_pair> * median, int64_t left_query, int64_t right_query){
    int64_t aux_1,aux_2,left_pivot,right_pivot,offset1, offset2,median_value;
    IntPair p1,p2;
    switch(PIVOT_TYPE){
        case PIVOT_WORKLOAD:
            T = standardCracking(crackerindex , COLUMN_SIZE, T, left_query,  -1);
            T = standardCracking(crackerindex , COLUMN_SIZE, T, right_query,  -1);
            return T;
        case PIVOT_RANDOM_WITHIN_PIECE:
            p1 = FindNeighborsGTE(left_query, (AvlTree)T,COLUMN_SIZE-1);
            p2 = FindNeighborsLT(right_query, (AvlTree)T, COLUMN_SIZE-1);
            offset1 = p1->first;
            offset2 = p2->second;
            aux_1 = (rand() % offset2) + offset1;
            aux_2 = (rand() % offset2) + offset1;
            left_pivot = std::min(aux_1,aux_2);
            right_pivot = std::max(aux_1,aux_2);
            T = standardCracking(crackerindex , COLUMN_SIZE, T, left_pivot,  -1);
            T = standardCracking(crackerindex , COLUMN_SIZE, T, right_pivot,  -1);
            return T;
        case PIVOT_RANDOM_MEDIAN_WITHIN_PIECE:
            p1 = FindNeighborsGTE(left_query, (AvlTree)T,COLUMN_SIZE-1);
            p2 = FindNeighborsLT(right_query, (AvlTree)T, COLUMN_SIZE-1);
            median_value =  crackerindex[(p1->first+p1->second)/2].m_key;
            T = standardCracking(crackerindex , COLUMN_SIZE, T, median_value,  -1);
            median_value =  crackerindex[(p2->first+p2->second)/2].m_key;
            T = standardCracking(crackerindex , COLUMN_SIZE, T, median_value,  -1);
            return T;
        case PIVOT_MEDIAN_WITHIN_PIECE:
            p1 = FindNeighborsGTE(left_query, (AvlTree)T,COLUMN_SIZE-1);
            p2 = FindNeighborsLT(right_query, (AvlTree)T, COLUMN_SIZE-1);
            median_value =  find_median(crackerindex,  p1->first,  p1->second);
            T = standardCracking(crackerindex , COLUMN_SIZE, T, median_value,  -1);
            median_value =  find_median(crackerindex,  p2->first,  p2->second);
            T = standardCracking(crackerindex , COLUMN_SIZE, T, median_value,  -1);
            return T;
        case PIVOT_RANDOM:
            aux_1 = rand() % COLUMN_SIZE;
            aux_2 = rand() % COLUMN_SIZE;
            left_pivot = std::min(aux_1,aux_2);
            right_pivot = std::max(aux_1,aux_2);
            T = standardCracking(crackerindex , COLUMN_SIZE, T, left_pivot,  -1);
            T = standardCracking(crackerindex , COLUMN_SIZE, T, right_pivot,  -1);
            return T;
        case PIVOT_RANDOM_MEDIAN:
            T = cracking_median_random(T,crackerindex,median);
            T = cracking_median_random(T,crackerindex,median);
            return T;
        case PIVOT_MEDIAN:
            T = cracking_median(T,crackerindex,median);
            T = cracking_median(T,crackerindex,median);
            return T;
    }
}


void cracking(Column& column, RangeQuery& rangeQueries, vector<int64_t> &answers, vector<double>& time) {
    chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> elapsed_seconds;
    std::queue<int_pair> median;
    median.push(int_pair{1,COLUMN_SIZE-1});
    start = chrono::system_clock::now();
    IndexEntry *crackercolumn = (IndexEntry *) malloc(column.data.size() * 2 * sizeof(int64_t));
    //Creating Cracker Column
    for (size_t i = 0; i < column.data.size(); i++) {
        crackercolumn[i].m_key = column.data[i];
        crackercolumn[i].m_rowId = i;
    }
    //Initialitizing Cracker Index
    AvlTree T = NULL;
    end = chrono::system_clock::now();
    time[0] += chrono::duration<double>(end - start).count();

    for (size_t i = 0; i < NUM_QUERIES; i++) {
        start = chrono::system_clock::now();
        //Partitioning Column and Inserting in Cracker Indexing
        T = pivot_selection(T, crackercolumn, &median, rangeQueries.leftpredicate[i],rangeQueries.rightpredicate[i] );
        //Querying
        IntPair p1 = FindNeighborsGTE(rangeQueries.leftpredicate[i], (AvlTree)T, column.data.size()-1);
        IntPair p2 = FindNeighborsLT(rangeQueries.rightpredicate[i], (AvlTree)T, column.data.size()-1);
        int64_t offset1 = p1->first;
        int64_t offset2 = p2->second;
        free(p1);
        free(p2);
        int64_t sum = scanQuery(crackercolumn,rangeQueries.leftpredicate[i],rangeQueries.rightpredicate[i], offset1, offset2);
        end = chrono::system_clock::now();
        time[i] += chrono::duration<double>(end - start).count();
        if (sum != answers[i])
            fprintf(stderr, "Incorrect Results on query %zu\n Expected : %ld    Got : %ld \n", i,answers[i], sum );
#ifndef DEBUG
        cout << time[i] << "\n";
#endif

    }
    free(crackercolumn);
}



void print_help(int argc, char** argv) {
    fprintf(stderr, "Unrecognized command line option.\n");
    fprintf(stderr, "Usage: %s [args]\n", argv[0]);
    fprintf(stderr, "   --column-path\n");
    fprintf(stderr, "   --query-path\n");
    fprintf(stderr, "   --answers-path\n");
    fprintf(stderr, "   --num-queries\n");
    fprintf(stderr, "   --column-size\n");
    fprintf(stderr, "   --indexing-type\n");
    fprintf(stderr, "   --delta\n");
}

pair<string,string> split_once(string delimited, char delimiter) {
    auto pos = delimited.find_first_of(delimiter);
    return { delimited.substr(0, pos), delimited.substr(pos+1) };
}

int main(int argc, char** argv) {
    COLUMN_FILE_PATH = "column";
    QUERIES_FILE_PATH = "query";
    ANSWER_FILE_PATH = "answer";

    NUM_QUERIES = 1000;
    COLUMN_SIZE = 10000000;
    PIVOT_TYPE = 6;

    for(int i = 1; i < argc; i++) {
        auto arg = string(argv[i]);
        if (arg.substr(0,2) != "--") {
            print_help(argc, argv);
            exit(EXIT_FAILURE);
        }
        arg = arg.substr(2);
        auto p = split_once(arg, '=');
        auto& arg_name = p.first; auto& arg_value = p.second;
        if (arg_name == "column-path") {
            COLUMN_FILE_PATH = arg_value;
        } else if (arg_name == "query-path") {
            QUERIES_FILE_PATH = arg_value;
        }else if (arg_name == "answer-path") {
            ANSWER_FILE_PATH = arg_value;
        }  else if (arg_name == "num-queries") {
            NUM_QUERIES = atoi(arg_value.c_str());
        } else if (arg_name == "column-size") {
            COLUMN_SIZE = atoi(arg_value.c_str());
        } else if (arg_name == "pivot-type") {
            PIVOT_TYPE = atoi(arg_value.c_str());
        }
        else {
            print_help(argc, argv);
            exit(EXIT_FAILURE);
        }
    }

    chrono::time_point<chrono::system_clock> start, middle, end;
    chrono::duration<double> elapsed_seconds;
    start = chrono::system_clock::now();

    int total;

    RangeQuery rangequeries;
    load_queries(&rangequeries,QUERIES_FILE_PATH,NUM_QUERIES);
    Column c;
    c.data = vector<int64_t>(COLUMN_SIZE);
    load_column(&c,COLUMN_FILE_PATH,COLUMN_SIZE);

    vector<int64_t> answers;
    load_answers(&answers,ANSWER_FILE_PATH,NUM_QUERIES);

    vector<double> times(NUM_QUERIES);

    cracking(c, rangequeries, answers, times);
}
