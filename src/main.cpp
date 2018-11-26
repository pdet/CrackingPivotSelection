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
const int PIVOT_EXACT_PREDICATE = 0, PIVOT_WITHIN_QUERY_PREDICATE=1,PIVOT_WITHIN_QUERY=2,PIVOT_WITHIN_COLUMN=3;

// Which Piece to crack ( Work for PIVOT_WITHIN_QUERY PIVOT_WITHIN_COLUMN)
const int RANDOM_PIECE=0 ,BIGGEST_PIECE=1;

//Setting Pivot Selection Within Pieces (Work for : PIVOT_WITHIN_QUERY_PREDICATE PIVOT_WITHIN_QUERY PIVOT_WITHIN_COLUMN)
const int RANDOM = 0, MEDIAN = 1, APPROXIMATE_MEDIAN=2;

string COLUMN_FILE_PATH, QUERIES_FILE_PATH, ANSWER_FILE_PATH;
int64_t  COLUMN_SIZE,NUM_QUERIES;
int PIVOT_TYPE;
int PIVOT_SELECTION;
int PIECE_TO_CRACK;

#define DEBUG

using namespace std;

void cracking_exact_predicate(Column& column, RangeQuery& rangeQueries, vector<int64_t> &answers, vector<double>& time) {
    chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> elapsed_seconds;
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
        IntPair p1, p2;
        int64_t offset,sum=0;

        p1 = FindNeighborsLT(rangeQueries.leftpredicate[i], T, COLUMN_SIZE - 1);
        p2 = FindNeighborsLT(rangeQueries.rightpredicate[i], T, COLUMN_SIZE - 1);

        if (p1->first == p2->first && p1->second == p2->second) {
            offset = crackPieceWithBothQueryPredicate(crackercolumn, p1->first, p1->second, rangeQueries.leftpredicate[i], rangeQueries.rightpredicate[i], &sum, rangeQueries.leftpredicate[i]);
            T = Insert(offset, rangeQueries.leftpredicate[i], T);
        }
        else {
            offset = crackPieceWithLeftPredicate(crackercolumn, p1->first, p1->second, rangeQueries.leftpredicate[i], &sum, rangeQueries.leftpredicate[i]);
            T = Insert(offset, rangeQueries.leftpredicate[i], T);
            offset = crackPieceWithRightPredicate(crackercolumn, p2->first, p2->second, rangeQueries.rightpredicate[i], &sum, rangeQueries.rightpredicate[i]);
            T = Insert(offset, rangeQueries.leftpredicate[i], T);
            if (p1->second+1 != p2->first)
                sum += scan_middle_pieces(crackercolumn, p1->second+1, p2->first);
        }

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

void cracking_within_predicate_piece(Column& column, RangeQuery& rangeQueries, vector<int64_t> &answers, vector<double>& time) {
    chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> elapsed_seconds;
    start = chrono::system_clock::now();
    IndexEntry *crackercolumn = (IndexEntry *) malloc(column.data.size() * 2 * sizeof(int64_t));
    //Creating Cracker Column
    for (size_t i = 0; i < column.data.size(); i++) {
        crackercolumn[i].m_key = column.data[i];
        crackercolumn[i].m_rowId = i;
    }
    //Initialitizing Cracker Index
    AvlTree T = NULL;
    //Intializing Query Output
    end = chrono::system_clock::now();
    time[0] += chrono::duration<double>(end - start).count();

    for (int i = 0; i < NUM_QUERIES; i++) {
        start = chrono::system_clock::now();
        IntPair p1,p2;
        int64_t offset,sum=0;
        p1 = FindNeighborsLT(rangeQueries.leftpredicate[i], T, COLUMN_SIZE-1);
        p2 = FindNeighborsLT(rangeQueries.rightpredicate[i], T, COLUMN_SIZE-1);
        int64_t pivot_1 = 0, pivot_2 = 0;

        if(p1->first==p2->first && p1->second==p2->second){
            switch(PIVOT_SELECTION){
                case RANDOM:
                    pivot_1 = (rand() % (p1->second-p1->first)) + p1->first;
                    pivot_1 = crackercolumn[pivot_1].m_key;
                    break;
                case APPROXIMATE_MEDIAN:
                    pivot_1 =  crackercolumn[(p1->first+p1->second)/2].m_key;
                    break;
                case MEDIAN:
                    find_median(crackercolumn,  p1->first,  p1->second);
                    break;
            }
            offset = crackPieceWithBothQueryPredicate(crackercolumn, p1->first, p1->second, rangeQueries.leftpredicate[i], rangeQueries.rightpredicate[i], &sum, pivot_1);
            T = Insert(offset, pivot_1, T);
        }
        else{
            switch(PIVOT_SELECTION){
                case RANDOM:
                    if (p1->second !=p1->first)
                        pivot_1 = (rand() %  (p1->second-p1->first)) + p1->first;
                    else
                        pivot_1 = p1->first;
                    if (p2->second !=p2->first)
                        pivot_2 = (rand() %  (p2->second-p2->first)) + p2->first;
                    else
                        pivot_2 = p1->first;
                    pivot_1 = crackercolumn[pivot_1].m_key;
                    pivot_2 = crackercolumn[pivot_2].m_key;
                    break;
                case APPROXIMATE_MEDIAN:
                    pivot_1 = crackercolumn[(p1->first+p1->second)/2].m_key;
                    pivot_2 = crackercolumn[(p2->first+p2->second)/2].m_key;
                    break;
                case MEDIAN:
                    pivot_1 =  find_median(crackercolumn,  p1->first,  p1->second);
                    pivot_2 =  find_median(crackercolumn,  p2->first,  p2->second);
                    break;
            }

            offset = crackPieceWithLeftPredicate(crackercolumn, p1->first, p1->second, rangeQueries.leftpredicate[i], &sum, pivot_1);
            T = Insert(offset, pivot_1, T);
            offset = crackPieceWithRightPredicate(crackercolumn, p2->first, p2->second, rangeQueries.rightpredicate[i], &sum, pivot_2);
            T = Insert(offset, pivot_2, T);
            if (p1->second+1 != p2->first)
                sum += scan_middle_pieces(crackercolumn, p1->second+1, p2->first);
        }
        free(p1);
        free(p2);
        end = chrono::system_clock::now();
        time[i] += chrono::duration<double>(end - start).count();
        fprintf(stderr, "QUery %zu \n",i);
        if (sum != answers[i])
            fprintf(stderr, "Incorrect Results on query %zu\n Expected : %ld    Got : %ld \n", i,answers[i], sum );
#ifndef DEBUG
        cout << time[i] << "\n";
#endif

    }
    free(crackercolumn);
}

void crack_within_query(Column& column, RangeQuery& rangeQueries, vector<int64_t> &answers, vector<double>& time) {
    chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> elapsed_seconds;
    start = chrono::system_clock::now();
    IndexEntry *crackercolumn = (IndexEntry *) malloc(column.data.size() * 2 * sizeof(int64_t));
    //Creating Cracker Column
    for (size_t i = 0; i < column.data.size(); i++) {
        crackercolumn[i].m_key = column.data[i];
        crackercolumn[i].m_rowId = i;
    }
    //Initialitizing Cracker Index
    AvlTree T = NULL;
    //Intializing Query Output
    end = chrono::system_clock::now();
    time[0] += chrono::duration<double>(end - start).count();

    for (int i = 0; i < NUM_QUERIES; i++) {
        start = chrono::system_clock::now();
        IntPair p1,p2;
        IntPair pivotPiece1,pivotPiece2,auxPiece;
        int64_t offset,sum=0;
        p1 = FindNeighborsLT(rangeQueries.leftpredicate[i], T, COLUMN_SIZE-1);
        p2 = FindNeighborsLT(rangeQueries.rightpredicate[i], T, COLUMN_SIZE-1);
        int64_t pivot_1 = 0, pivot_2 = 0;
        switch(PIECE_TO_CRACK){
            case BIGGEST_PIECE:
                FindTwoBiggestPieces(rangeQueries.leftpredicate[i],rangeQueries.rightpredicate[i], T, COLUMN_SIZE-1, crackercolumn, pivotPiece1, pivotPiece2);
                if(pivotPiece2->first < pivotPiece1->first){
                    auxPiece = (IntPair) malloc(sizeof(struct int_pair));
                    auxPiece = pivotPiece1;
                    pivotPiece1 = pivotPiece2;
                    pivotPiece2 = auxPiece;
                    free(auxPiece);
                }
                break;

            case RANDOM_PIECE:
                pivot_1 = (rand() %  (p2->second-p1->first)) + p1->first;
                pivot_2 = (rand() %  (p2->second-p1->first)) + p1->first;
                int64_t aux = std::max(pivot_1,pivot_2);
                pivot_1 = std::min(pivot_1,pivot_2);
                pivot_2 = aux;
                pivotPiece1 = FindNeighborsLT(crackercolumn[pivot_1].m_key, T, COLUMN_SIZE-1);
                pivotPiece2 = FindNeighborsLT(crackercolumn[pivot_2].m_key, T, COLUMN_SIZE-1);
                break;
        }
        if(pivotPiece1->first==pivotPiece2->first && pivotPiece1->second==pivotPiece2->second){
            switch(PIVOT_SELECTION){
                case RANDOM:
                    pivot_1 = (rand() % (pivotPiece1->second-pivotPiece1->first)) + pivotPiece1->first;
                    pivot_1 = crackercolumn[pivot_1].m_key;
                    break;
                case APPROXIMATE_MEDIAN:
                    pivot_1 =  crackercolumn[(pivotPiece1->first+pivotPiece1->second)/2].m_key;
                    break;
                case MEDIAN:
                    pivot_1 = find_median(crackercolumn,  pivotPiece1->first,  pivotPiece1->second);
                    break;
            }
            if (pivotPiece1->first==p1->first && pivotPiece1->second==p1->second && pivotPiece1->first==p2->first && pivotPiece1->second==p2->second){
                offset = crackPieceWithBothQueryPredicate(crackercolumn, p1->first, p1->second, rangeQueries.leftpredicate[i], rangeQueries.rightpredicate[i], &sum, pivot_1);
            }

            else if (pivotPiece1->first==p1->first && pivotPiece1->second==p1->second){
                offset = crackPieceWithLeftPredicate(crackercolumn, p1->first, p1->second, rangeQueries.leftpredicate[i], &sum, pivot_1);
                sum += scan_right_piece(crackercolumn, p1->second+1, p2->second,rangeQueries.rightpredicate[i]);
            }
            else if (pivotPiece1->first==p2->first && pivotPiece1->second==p2->second){
                offset = crackPieceWithRightPredicate(crackercolumn, pivotPiece1->first, pivotPiece1->second, rangeQueries.rightpredicate[i], &sum, pivot_1);
                sum += scan_left_piece(crackercolumn, p1->first, pivotPiece1->first-1,rangeQueries.leftpredicate[i]);
            }
            else{
                offset = crackPieceMiddleQuery(crackercolumn, pivotPiece1->first, pivotPiece1->second,&sum, pivot_1);
                sum += scan_left_piece(crackercolumn, p1->first, pivotPiece1->first-1,rangeQueries.leftpredicate[i]);
                sum += scan_right_piece(crackercolumn, pivotPiece1->second+1, p2->second,rangeQueries.rightpredicate[i]);
            }
            T = Insert(offset, pivot_1, T);
        }

        else{
            switch(PIVOT_SELECTION){
                case RANDOM:
                    if (pivotPiece1->second !=pivotPiece1->first)
                        pivot_1 = (rand() %  (pivotPiece1->second-pivotPiece1->first)) + pivotPiece1->first;
                    else
                        pivot_1 = pivotPiece1->first;
                    if (pivotPiece2->second !=pivotPiece2->first)
                        pivot_2 = (rand() %  (pivotPiece2->second-pivotPiece2->first)) + pivotPiece2->first;
                    else
                        pivot_2 = pivotPiece2->first;
                    pivot_1 = crackercolumn[pivot_1].m_key;
                    pivot_2 = crackercolumn[pivot_2].m_key;
                    break;
                case APPROXIMATE_MEDIAN:
                    pivot_1 = crackercolumn[(pivotPiece1->first+pivotPiece1->second)/2].m_key;
                    pivot_2 = crackercolumn[(pivotPiece2->first+pivotPiece2->second)/2].m_key;
                    break;
                case MEDIAN:
                    pivot_1 =  find_median(crackercolumn,  pivotPiece1->first,  pivotPiece1->second);
                    pivot_2 =  find_median(crackercolumn,  pivotPiece2->first,  pivotPiece2->second);
                    break;
            }
            if (pivotPiece1->first==p1->first && pivotPiece1->second==p1->second && pivotPiece2->first==p2->first && pivotPiece2->second==p2->second){
                offset = crackPieceWithLeftPredicate(crackercolumn, p1->first, p1->second, rangeQueries.leftpredicate[i], &sum, pivot_1);
                T = Insert(offset, pivot_1, T);
                offset = crackPieceWithRightPredicate(crackercolumn, p2->first, p2->second, rangeQueries.rightpredicate[i], &sum, pivot_2);
                T = Insert(offset, pivot_2, T);
                sum += scan_middle_pieces(crackercolumn, p1->second+1, p2->first);
            }
            else if (pivotPiece1->first==p1->first && pivotPiece1->second==p1->second){
                offset = crackPieceWithLeftPredicate(crackercolumn, p1->first, p1->second, rangeQueries.leftpredicate[i], &sum, pivot_1);
                T = Insert(offset, pivot_1, T);
                offset = crackPieceMiddleQuery(crackercolumn, pivotPiece2->first, pivotPiece2->second, &sum, pivot_2);
                T = Insert(offset, pivot_2, T);
                sum += scan_middle_pieces(crackercolumn, p1->second+1, pivotPiece2->first);
                sum += scan_right_piece(crackercolumn, pivotPiece2->second+1, p2->second,rangeQueries.rightpredicate[i]);
            }
            else if (pivotPiece2->first==p2->first && pivotPiece2->second==p2->second){
                offset = crackPieceWithRightPredicate(crackercolumn, p2->first, p2->second, rangeQueries.rightpredicate[i], &sum, pivot_2);
                T = Insert(offset, pivot_2, T);
                offset = crackPieceMiddleQuery(crackercolumn, pivotPiece1->first, pivotPiece1->second, &sum, pivot_1);
                T = Insert(offset, pivot_1, T);
                sum += scan_left_piece(crackercolumn, p1->first, pivotPiece1->first-1, rangeQueries.leftpredicate[i]);
                sum += scan_middle_pieces(crackercolumn, pivotPiece1->second+1, p2->first);
            }
            else{
                offset = crackPieceMiddleQuery(crackercolumn, pivotPiece1->first, pivotPiece1->second, &sum, pivot_1);
                T = Insert(offset, pivot_1, T);
                offset = crackPieceMiddleQuery(crackercolumn, pivotPiece2->first, pivotPiece2->second, &sum, pivot_2);
                T = Insert(offset, pivot_2, T);
                sum += scan_left_piece(crackercolumn, p1->first, pivotPiece1->first-1, rangeQueries.leftpredicate[i]);
                sum += scan_middle_pieces(crackercolumn, pivotPiece1->second+1, pivotPiece2->first);
                sum += scan_right_piece(crackercolumn, pivotPiece2->second+1, p2->second, rangeQueries.rightpredicate[i]);
            }
        }
        free(p1);
        free(p2);
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

void crack_within_column(Column& column, RangeQuery& rangeQueries, vector<int64_t> &answers, vector<double>& time) {
    chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> elapsed_seconds;
    start = chrono::system_clock::now();
    IndexEntry *crackercolumn = (IndexEntry *) malloc(column.data.size() * 2 * sizeof(int64_t));
    //Creating Cracker Column
    for (size_t i = 0; i < column.data.size(); i++) {
        crackercolumn[i].m_key = column.data[i];
        crackercolumn[i].m_rowId = i;
    }
    //Initialitizing Cracker Index
    AvlTree T = NULL;
    //Intializing Query Output
    end = chrono::system_clock::now();
    time[0] += chrono::duration<double>(end - start).count();

    for (int i = 0; i < NUM_QUERIES; i++) {
        start = chrono::system_clock::now();
        IntPair p1,p2;
        IntPair pivotPiece1,pivotPiece2,auxPiece;
        int64_t offset,sum=0;
        p1 = FindNeighborsLT(rangeQueries.leftpredicate[i], T, COLUMN_SIZE-1);
        p2 = FindNeighborsLT(rangeQueries.rightpredicate[i], T, COLUMN_SIZE-1);
        int64_t pivot_1 = 0, pivot_2 = 0;
        switch(PIECE_TO_CRACK){
            case BIGGEST_PIECE:
                FindTwoBiggestPieces(0,INT64_MAX, T, COLUMN_SIZE-1, crackercolumn, pivotPiece1, pivotPiece2);
                if(pivotPiece2->first < pivotPiece1->first){
                    auxPiece = (IntPair) malloc(sizeof(struct int_pair));
                    auxPiece = pivotPiece1;
                    pivotPiece1 = pivotPiece2;
                    pivotPiece2 = auxPiece;
                    free(auxPiece);
                }
                break;

            case RANDOM_PIECE:
                pivot_1 = rand() %  COLUMN_SIZE-1;;
                pivot_2 = rand() %  COLUMN_SIZE-1;
                int64_t aux = std::max(pivot_1,pivot_2);
                pivot_1 = std::min(pivot_1,pivot_2);
                pivot_2 = aux;
                pivotPiece1 = FindNeighborsLT(crackercolumn[pivot_1].m_key, T, COLUMN_SIZE-1);
                pivotPiece2 = FindNeighborsLT(crackercolumn[pivot_2].m_key, T, COLUMN_SIZE-1);
                break;
        }
        if(pivotPiece1->first==pivotPiece2->first && pivotPiece1->second==pivotPiece2->second){
            switch(PIVOT_SELECTION){
                case RANDOM:
                    pivot_1 = (rand() % (pivotPiece1->second-pivotPiece1->first)) + pivotPiece1->first;
                    pivot_1 = crackercolumn[pivot_1].m_key;
                    break;
                case APPROXIMATE_MEDIAN:
                    pivot_1 =  crackercolumn[(pivotPiece1->first+pivotPiece1->second)/2].m_key;
                    break;
                case MEDIAN:
                    pivot_1 = find_median(crackercolumn,  pivotPiece1->first,  pivotPiece1->second);
                    break;
            }
            if (pivotPiece1->first==p1->first && pivotPiece1->second==p1->second && pivotPiece1->first==p2->first && pivotPiece1->second==p2->second){
                offset = crackPieceWithBothQueryPredicate(crackercolumn, p1->first, p1->second, rangeQueries.leftpredicate[i], rangeQueries.rightpredicate[i], &sum, pivot_1);
            }

            else if (pivotPiece1->first==p1->first && pivotPiece1->second==p1->second){
                offset = crackPieceWithLeftPredicate(crackercolumn, p1->first, p1->second, rangeQueries.leftpredicate[i], &sum, pivot_1);
                sum += scan_right_piece(crackercolumn, p1->second+1, p2->second,rangeQueries.rightpredicate[i]);
            }
            else if (pivotPiece1->first==p2->first && pivotPiece1->second==p2->second){
                offset = crackPieceWithRightPredicate(crackercolumn, pivotPiece1->first, pivotPiece1->second, rangeQueries.rightpredicate[i], &sum, pivot_1);
                sum += scan_left_piece(crackercolumn, p1->first, p1->second,rangeQueries.leftpredicate[i]);
            }
            else if (pivotPiece1->second < p1->first || pivotPiece1->first > p2->second){
                offset = crackPieceOutsideQuery(crackercolumn,pivotPiece1->first,pivotPiece2->second,pivot_1);

                if (p1->first == p2->first && p1->second == p2->second)
                    sum +=scan_left_right_piece(crackercolumn, p1->first, p1->second,rangeQueries.leftpredicate[i],rangeQueries.rightpredicate[i]);

                else{
                    sum += scan_left_piece(crackercolumn, p1->first, p1->second,rangeQueries.leftpredicate[i]);
                    sum += scan_middle_pieces(crackercolumn, p1->second+1, p2->first);
                    sum += scan_right_piece(crackercolumn, p2->first, p2->second,rangeQueries.rightpredicate[i]);
                }

            }
            else{
                offset = crackPieceMiddleQuery(crackercolumn, pivotPiece1->first, pivotPiece1->second,&sum, pivot_1);
                sum += scan_left_piece(crackercolumn, p1->first, pivotPiece1->first-1,rangeQueries.leftpredicate[i]);
                sum += scan_right_piece(crackercolumn, pivotPiece1->second+1, p2->second,rangeQueries.rightpredicate[i]);
            }
            T = Insert(offset, pivot_1, T);
        }

        else{
            switch(PIVOT_SELECTION){
                case RANDOM:
                    if (pivotPiece1->second !=pivotPiece1->first)
                        pivot_1 = (rand() %  (pivotPiece1->second-pivotPiece1->first)) + pivotPiece1->first;
                    else
                        pivot_1 = pivotPiece1->first;
                    if (pivotPiece2->second !=pivotPiece2->first)
                        pivot_2 = (rand() %  (pivotPiece2->second-pivotPiece2->first)) + pivotPiece2->first;
                    else
                        pivot_2 = pivotPiece2->first;
                    pivot_1 = crackercolumn[pivot_1].m_key;
                    pivot_2 = crackercolumn[pivot_2].m_key;
                    break;
                case APPROXIMATE_MEDIAN:
                    pivot_1 = crackercolumn[(pivotPiece1->first+pivotPiece1->second)/2].m_key;
                    pivot_2 = crackercolumn[(pivotPiece2->first+pivotPiece2->second)/2].m_key;
                    break;
                case MEDIAN:
                    pivot_1 =  find_median(crackercolumn,  pivotPiece1->first,  pivotPiece1->second);
                    pivot_2 =  find_median(crackercolumn,  pivotPiece2->first,  pivotPiece2->second);
                    break;
            }
            if ((pivotPiece1->second < p1->first || pivotPiece1->first > p2->second) && (pivotPiece2->second < p1->first || pivotPiece2->first > p2->second)){
                offset = crackPieceOutsideQuery(crackercolumn,pivotPiece1->first,pivotPiece1->second,pivot_1);
                T = Insert(offset, pivot_1, T);
                offset = crackPieceOutsideQuery(crackercolumn,pivotPiece2->first,pivotPiece2->second,pivot_2);
                T = Insert(offset, pivot_2, T);
                if (p1->first == p2->first && p1->second == p2->second)
                    sum +=scan_left_right_piece(crackercolumn, p1->first, p1->second,rangeQueries.leftpredicate[i],rangeQueries.rightpredicate[i]);
                else{
                    sum += scan_left_piece(crackercolumn, p1->first, p1->second,rangeQueries.leftpredicate[i]);
                    if(p1->second+1 != p2->first)
                        sum += scan_middle_pieces(crackercolumn, p1->second+1, p2->first);
                    sum += scan_right_piece(crackercolumn, p2->first, p2->second,rangeQueries.rightpredicate[i]);
                }
            }
            else if (pivotPiece1->second < p1->first || pivotPiece1->first > p2->second){
                offset = crackPieceOutsideQuery(crackercolumn,pivotPiece1->first,pivotPiece1->second,pivot_1);
                T = Insert(offset, pivot_1, T);

                if (pivotPiece2->first==p1->first && pivotPiece2->second==p1->second){
                    if (p1->first == p2->first && p1->second == p2->second){
                        offset = crackPieceWithBothQueryPredicate(crackercolumn, p1->first, p1->second, rangeQueries.leftpredicate[i],rangeQueries.rightpredicate[i], &sum, pivot_2);
                        T = Insert(offset, pivot_2, T);
                    }
                    else{
                        offset = crackPieceWithLeftPredicate(crackercolumn, p1->first, p1->second, rangeQueries.leftpredicate[i], &sum, pivot_2);
                        T = Insert(offset, pivot_2, T);
                        sum += scan_middle_pieces(crackercolumn, p1->second+1, p2->first);
                        sum += scan_right_piece(crackercolumn, p2->first, p2->second,rangeQueries.rightpredicate[i]);
                    }

                }
                else if (pivotPiece2->first==p2->first && pivotPiece2->second==p2->second){
                    offset = crackPieceWithRightPredicate(crackercolumn, p2->first, p2->second, rangeQueries.rightpredicate[i], &sum, pivot_2);
                    T = Insert(offset, pivot_2, T);
                    sum += scan_left_piece(crackercolumn, p1->first, p1->second,rangeQueries.leftpredicate[i]);
                    sum += scan_middle_pieces(crackercolumn, p1->second+1, p2->first);
                }
                else{
                    offset = crackPieceMiddleQuery(crackercolumn, pivotPiece2->first, pivotPiece2->second, &sum, pivot_2);
                    T = Insert(offset, pivot_2, T);
                    sum += scan_left_piece(crackercolumn, p1->first, p1->second, rangeQueries.leftpredicate[i]);
                    sum += scan_middle_pieces(crackercolumn, p1->second+1, pivotPiece2->first);
                    sum += scan_right_piece(crackercolumn, pivotPiece2->second+1, p2->second, rangeQueries.rightpredicate[i]);
                }
            }
            else if (pivotPiece2->second < p1->first || pivotPiece2->first > p2->second){
                offset = crackPieceOutsideQuery(crackercolumn,pivotPiece2->first,pivotPiece2->second,pivot_2);
                T = Insert(offset, pivot_2, T);
                if (pivotPiece1->first==p1->first && pivotPiece1->second==p1->second){
                    if (p1->first == p2->first && p1->second == p2->second)
                        offset = crackPieceWithBothQueryPredicate(crackercolumn, p1->first, p1->second, rangeQueries.leftpredicate[i], rangeQueries.rightpredicate[i], &sum, pivot_1);
                    else{
                        offset = crackPieceWithLeftPredicate(crackercolumn, p1->first, p1->second, rangeQueries.leftpredicate[i], &sum, pivot_1);
                        sum += scan_middle_pieces(crackercolumn, p1->second+1, p2->first);
                        sum += scan_right_piece(crackercolumn, p2->first, p2->second,rangeQueries.rightpredicate[i]);
                    }
                    T = Insert(offset, pivot_1, T);

                }
                else if (pivotPiece1->first==p2->first && pivotPiece1->second==p2->second){
                    offset = crackPieceWithRightPredicate(crackercolumn, p2->first, p2->second, rangeQueries.rightpredicate[i], &sum, pivot_1);
                    T = Insert(offset, pivot_1, T);
                    sum += scan_left_piece(crackercolumn, p1->first, p1->second,rangeQueries.leftpredicate[i]);
                    sum += scan_middle_pieces(crackercolumn, p1->second+1, p2->first);
                }
                else{
                    offset = crackPieceMiddleQuery(crackercolumn, pivotPiece1->first, pivotPiece1->second, &sum, pivot_1);
                    T = Insert(offset, pivot_1, T);
                    sum += scan_left_piece(crackercolumn, p1->first, p1->second, rangeQueries.leftpredicate[i]);
                    sum += scan_middle_pieces(crackercolumn, p1->second+1, pivotPiece1->first);
                    sum += scan_right_piece(crackercolumn, pivotPiece1->second+1, p2->second, rangeQueries.rightpredicate[i]);
                }
            }
            else if (pivotPiece1->first==p1->first && pivotPiece1->second==p1->second && pivotPiece2->first==p2->first && pivotPiece2->second==p2->second){
                offset = crackPieceWithLeftPredicate(crackercolumn, p1->first, p1->second, rangeQueries.leftpredicate[i], &sum, pivot_1);
                T = Insert(offset, pivot_1, T);
                offset = crackPieceWithRightPredicate(crackercolumn, p2->first, p2->second, rangeQueries.rightpredicate[i], &sum, pivot_2);
                T = Insert(offset, pivot_2, T);
                sum += scan_middle_pieces(crackercolumn, p1->second+1, p2->first);
            }
            else if (pivotPiece1->first==p1->first && pivotPiece1->second==p1->second){
                offset = crackPieceWithLeftPredicate(crackercolumn, p1->first, p1->second, rangeQueries.leftpredicate[i], &sum, pivot_1);
                T = Insert(offset, pivot_1, T);
                offset = crackPieceMiddleQuery(crackercolumn, pivotPiece2->first, pivotPiece2->second, &sum, pivot_2);
                T = Insert(offset, pivot_2, T);
                sum += scan_middle_pieces(crackercolumn, p1->second+1, pivotPiece2->first);
                sum += scan_right_piece(crackercolumn, pivotPiece2->second+1, p2->second,rangeQueries.rightpredicate[i]);
            }
            else if (pivotPiece2->first==p2->first && pivotPiece2->second==p2->second){
                offset = crackPieceWithRightPredicate(crackercolumn, p2->first, p2->second, rangeQueries.rightpredicate[i], &sum, pivot_2);
                T = Insert(offset, pivot_2, T);
                offset = crackPieceMiddleQuery(crackercolumn, pivotPiece1->first, pivotPiece1->second, &sum, pivot_1);
                T = Insert(offset, pivot_1, T);
                sum += scan_left_piece(crackercolumn, p1->first, pivotPiece1->first-1, rangeQueries.leftpredicate[i]);
                sum += scan_middle_pieces(crackercolumn, pivotPiece1->second+1, p2->first);
            }
            else{
                offset = crackPieceMiddleQuery(crackercolumn, pivotPiece1->first, pivotPiece1->second, &sum, pivot_1);
                T = Insert(offset, pivot_1, T);
                offset = crackPieceMiddleQuery(crackercolumn, pivotPiece2->first, pivotPiece2->second, &sum, pivot_2);
                T = Insert(offset, pivot_2, T);
                sum += scan_left_piece(crackercolumn, p1->first, pivotPiece1->first-1, rangeQueries.leftpredicate[i]);
                sum += scan_middle_pieces(crackercolumn, pivotPiece1->second+1, pivotPiece2->first);
                sum += scan_right_piece(crackercolumn, pivotPiece2->second+1, p2->second, rangeQueries.rightpredicate[i]);
            }
        }
        free(p1);
        free(p2);
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
    fprintf(stderr, "   --pivot-type\n");
    fprintf(stderr, "   --pivot-selection\n");
    fprintf(stderr, "   --piece-to-crack\n");

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
    PIVOT_TYPE = 0;
    PIVOT_SELECTION = 0;
    PIECE_TO_CRACK = 0;

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
        } else if (arg_name == "pivot-selection") {
            PIVOT_SELECTION = atoi(arg_value.c_str());
        } else if (arg_name == "piece-to-crack") {
            PIECE_TO_CRACK = atoi(arg_value.c_str());
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

    switch(PIVOT_TYPE){
        case PIVOT_EXACT_PREDICATE:
            cracking_exact_predicate(c, rangequeries, answers, times);
            break;
        case PIVOT_WITHIN_QUERY_PREDICATE:
            cracking_within_predicate_piece(c, rangequeries, answers, times);
            break;
        case PIVOT_WITHIN_QUERY:
            crack_within_query(c, rangequeries, answers, times);
            break;
        case PIVOT_WITHIN_COLUMN:
            crack_within_column(c, rangequeries, answers, times);
            break;
    }
}
