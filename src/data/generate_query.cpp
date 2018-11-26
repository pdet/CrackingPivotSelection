#include <vector>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <cassert>
#include <map>
#include <chrono>
#include <algorithm>
#include "file_manager.h"

string COLUMN_FILE_PATH , QUERIES_FILE_PATH , ANSWER_FILE_PATH;
double SELECTIVITY_PERCENTAGE , ZIPF_ALPHA ;
int64_t NUM_QUERIES, COLUMN_SIZE, QUERIES_PATTERN, UPPERBOUND;

//#define verify
//#define debug

int64_t apply_selectivity(int64_t leftQuery,int64_t* answer, vector<int64_t> *c){
    int64_t selec = 0;
    *answer = 0;
    int aux = c->size()*SELECTIVITY_PERCENTAGE;
    for (size_t i = 0; i < c->size(); i++) {
        if (c->at(i) >= leftQuery){
            selec++;
            *answer += c->at(i);
        }
        if ((selec >= aux && c->at(i) != c->at(i+1))|| i ==  c->size()-2){
            return c->at(i+1);
        }
    }
}

void random(vector<int64_t> *leftQuery, vector<int64_t> *rightQuery,vector<int64_t> *queryAnswer, vector<int64_t> *orderedColumn, int64_t maxLeftQueryVal) {
    for (int i = 0; i < NUM_QUERIES; i++) {
        int64_t answer;
        int64_t q1 = rand() % maxLeftQueryVal;
        int64_t q2 = apply_selectivity(q1,&answer,orderedColumn);
        leftQuery->push_back(q1);
        rightQuery->push_back(q2);
        queryAnswer->push_back(answer);
    }
}

void sequential(vector<int64_t> *leftQuery,vector<int64_t> *rightQuery,vector<int64_t> *queryAnswer, vector<int64_t> *orderedColumn, int64_t maxLeftQueryVal){
    int64_t lKey = orderedColumn->at(1);
    int64_t rKey;
    int64_t jump = UPPERBOUND * 0.01; // Variance of 1%
    int64_t answer;

    for(int i = 0; i < NUM_QUERIES; ++i){
        rKey = apply_selectivity(lKey,&answer,orderedColumn);
        leftQuery->push_back(lKey);
        rightQuery->push_back(rKey);
        queryAnswer->push_back(answer);
        lKey += jump;
        if (lKey > maxLeftQueryVal)
            lKey = rand()%jump;
        // check if query is still in the domain
    }

}

int64_t zipf(double alpha, int64_t n)
{
    static int first = true;      // Static first time flag
    static double c = 0;          // Normalization constant
    double z;                     // Uniform random number (0 < z < 1)
    double sum_prob;              // Sum of probabilities
    double zipf_value = 0.0;      // Computed exponential value to be returned
    int64_t    i;                     // Loop counter

    // Compute normalization constant on first call only
    if (first == true)
    {
        for (i=1; i<=n; i++)
            c = c + (1.0 / pow((double) i, alpha));
        c = 1.0 / c;
        first = false;
    }

    // Pull a uniform random number (0 < z < 1)
    do
    {
        z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    }
    while ((z == 0) || (z == 1));

    // Map z to the value
    sum_prob = 0;
    for (i=1; i<=n; i++)
    {
        sum_prob = sum_prob + c / pow((double) i, alpha);
        if (sum_prob >= z)
        {
            zipf_value = i;
            break;
        }
    }

    // Assert that zipf_value is between 1 and N
    assert((zipf_value >=1) && (zipf_value <= n));

    return zipf_value;
}

void generate_skewed_data(vector<int64_t> *data, int64_t size, int64_t maxLeftQueryVal) {
    // the focus should be in the center of the dataset
    int64_t hotspot = UPPERBOUND  / 2;

    // compute zipf distribution
    typedef map<int64_t, int64_t> result_t;
    typedef result_t::iterator result_iterator_t;

    result_t result;
    for(size_t i = 0; i < size; ++i) {
        int64_t nextValue = zipf(ZIPF_ALPHA, UPPERBOUND);
        result_iterator_t it = result.find(nextValue);
        if(it != result.end()) {
            ++it->second;
        }
        else {
            result.insert(make_pair(nextValue, 1));
        }
    }

    int64_t zoneSize = hotspot / result.size();

    int64_t zone = 0;
    for(result_iterator_t it = result.begin(); it != result.end(); ++it) {
        for(int i = 0; i < it->second; ++i) {
            int64_t direction = rand() % 2 == 0 ? 1 : -1;
            int64_t zoneBegin = hotspot + (zone * zoneSize * direction);
            int64_t zoneEnd = zoneBegin + (zoneSize * direction);
            if(direction == -1) {
                int64_t tmp = zoneBegin;
                zoneBegin = zoneEnd;
                zoneEnd = tmp;
            }
            int64_t predicate = rand() % (zoneEnd - zoneBegin + 1) + zoneBegin;
            while(predicate > maxLeftQueryVal){
                direction = rand() % 2 == 0 ? 1 : -1;
                zoneBegin = hotspot + (zone * zoneSize * direction);
                zoneEnd = zoneBegin + (zoneSize * direction);
                if(direction == -1) {
                    int64_t tmp = zoneBegin;
                    zoneBegin = zoneEnd;
                    zoneEnd = tmp;
                }
                predicate = rand() % (zoneEnd - zoneBegin + 1) + zoneBegin;
            }
            if( UPPERBOUND  / 2 > maxLeftQueryVal){
                fprintf(stderr, "This Selectivity is to high to a centered skewed workload");
                assert(0);
            }
            data->push_back(predicate);
        }
        ++zone;
    }

    random_shuffle(data->begin(), data->end());

}


void skewed(vector<int64_t> *leftQuery,vector<int64_t> *rightQuery, vector<int64_t> *queryAnswer, vector<int64_t> *orderedColumn, int64_t maxLeftQueryVal) {
    int64_t answer;
    generate_skewed_data(leftQuery,NUM_QUERIES,maxLeftQueryVal);
    for(int64_t q1 : *leftQuery) {
        int64_t q2 = apply_selectivity(q1,&answer,orderedColumn);
        rightQuery->push_back(q2);
        queryAnswer->push_back(answer);
    }

}

void mixed(vector<int64_t> *leftQuery,vector<int64_t> *rightQuery, vector<int64_t> *queryAnswer,vector<int64_t> *orderedColumn, int64_t maxLeftQueryVal) {
    //One sided range queries
    for (int i = orderedColumn->size()-1; i >= 0; i--) {
        if (maxLeftQueryVal < orderedColumn->size()*0.21)
            maxLeftQueryVal++;
        else{
            maxLeftQueryVal = orderedColumn->at(i);
            break;
        }
    }
    int64_t answer;
    int switch_workload = 10;
    if ( NUM_QUERIES %switch_workload != 0){
        fprintf(stderr, "Num Queries must be multiple of 10");
        assert(0);
    }
    for (size_t i = 1; i <= NUM_QUERIES/switch_workload; i ++){
        if(i%3==0){

            vector<int64_t> auxQuery;
            generate_skewed_data(&auxQuery,switch_workload,maxLeftQueryVal);
            for(int64_t q1 : auxQuery) {
                SELECTIVITY_PERCENTAGE = 0.01 + (double)(rand() % 9)/100;
                int64_t q2 = apply_selectivity(q1,&answer,orderedColumn);
                leftQuery->push_back(q1);
                rightQuery->push_back(q2);
                queryAnswer->push_back(answer);
            }
        }
        else if (i%3==2){
            for (size_t q = 0; q < switch_workload; q++){
                SELECTIVITY_PERCENTAGE = 0.01 + (double)(rand() % 9)/100;
                int64_t q1 = rand() % maxLeftQueryVal;
                int64_t q2 = apply_selectivity(q1,&answer,orderedColumn);
                leftQuery->push_back(q1);
                rightQuery->push_back(q2);
                queryAnswer->push_back(answer);
            }
        }
        else{
            int64_t jump = UPPERBOUND * 0.01; // Variance of 1%
            int64_t lKey;
            if( i == 1)
                lKey = orderedColumn->at(1);
            else
                lKey = rightQuery[0][i*switch_workload-1];
            if (lKey > maxLeftQueryVal)
                lKey = rand()%jump;
            int64_t rKey;
            for (size_t q = 0; q < switch_workload; q++){
                SELECTIVITY_PERCENTAGE = 0.01 + (double)(rand() % 9)/100;
                rKey = apply_selectivity(lKey,&answer,orderedColumn);
                leftQuery->push_back(lKey);
                rightQuery->push_back(rKey);
                queryAnswer->push_back(answer);
                lKey = rKey;
                lKey += jump;
                while (lKey > maxLeftQueryVal)
                    lKey = rand()%jump;
            }
        }
    }

}

void generate_query(vector<int64_t> *orderedColumn) {
    chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> elapsed_seconds;
    vector<int64_t> leftQuery;
    vector<int64_t> rightQuery;
    vector<int64_t> queryAnswer;
    int64_t maxLeftQueryVal = 0;
    start = chrono::system_clock::now();
    for (int i = orderedColumn->size()-1; i >= 0; i--) {
        if (maxLeftQueryVal < orderedColumn->size()*SELECTIVITY_PERCENTAGE)
            maxLeftQueryVal++;
        else{
            maxLeftQueryVal = orderedColumn->at(i);
            break;
        }
    }
    if(QUERIES_PATTERN == 1)
        random(&leftQuery, &rightQuery, &queryAnswer,orderedColumn, maxLeftQueryVal);
    else if(QUERIES_PATTERN == 2)
        sequential(&leftQuery, &rightQuery, &queryAnswer, orderedColumn, maxLeftQueryVal);
    else if(QUERIES_PATTERN == 3)
        skewed(&leftQuery, &rightQuery, &queryAnswer, orderedColumn, maxLeftQueryVal);
    else if(QUERIES_PATTERN == 4)
        mixed(&leftQuery, &rightQuery, &queryAnswer, orderedColumn, maxLeftQueryVal);
    end = chrono::system_clock::now();
    #ifdef VERIFY
        verifySelectivity(orderedColumn,&leftQuery,&rightQuery, SELECTIVITY_PERCENTAGE);
    #endif

    #ifdef DEBUG
        for (size_t i = 0; i < NUM_QUERIES; i ++)
            cout << i+1 << ";" << leftQuery[i] << ";" << rightQuery[i] << "\n";
    #endif

    FILE* f = fopen(QUERIES_FILE_PATH.c_str(), "w+");
    fwrite(&leftQuery[0], sizeof(int64_t), NUM_QUERIES, f);
    fwrite(&rightQuery[0], sizeof(int64_t), NUM_QUERIES, f);
    fclose(f);
    FILE* f_2 = fopen(ANSWER_FILE_PATH.c_str(), "w+");
    fwrite(&queryAnswer[0], sizeof(int64_t), NUM_QUERIES, f_2);
    fclose(f_2);

    elapsed_seconds = end-start;
    cout << "Creating Query Attr: " << elapsed_seconds.count() << "s\n";

}

void print_help(int argc, char** argv) {
    fprintf(stderr, "Unrecognized command line option.\n");
    fprintf(stderr, "Usage: %s [args]\n", argv[0]);
    fprintf(stderr, "   --column-path\n");
    fprintf(stderr, "   --query-path\n");
    fprintf(stderr, "   --answer-path\n");
    fprintf(stderr, "   --selectiviy\n");
    fprintf(stderr, "   --zipf-alpha\n");
    fprintf(stderr, "   --num-queries\n");
    fprintf(stderr, "   --queries-pattern\n");
    fprintf(stderr, "   --point-query\n");
    fprintf(stderr, "   --column-upperbound\n");
}

pair<string,string> split_once(string delimited, char delimiter) {
    auto pos = delimited.find_first_of(delimiter);
    return { delimited.substr(0, pos), delimited.substr(pos+1) };
}


int main(int argc, char** argv) {
    COLUMN_FILE_PATH =  "column";
    QUERIES_FILE_PATH = "query";
    ANSWER_FILE_PATH = "answer";
    SELECTIVITY_PERCENTAGE = 0.01;
    ZIPF_ALPHA = 2.0;
    NUM_QUERIES = 150;
    COLUMN_SIZE = 100000000;
    QUERIES_PATTERN = 4;
    UPPERBOUND = 1000000000;


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
        } else if (arg_name == "answer-path") {
            ANSWER_FILE_PATH = arg_value;
        } else if (arg_name == "selectivity") {
            SELECTIVITY_PERCENTAGE = atof(arg_value.c_str());
        } else if (arg_name == "num-queries") {
            NUM_QUERIES = atoi(arg_value.c_str());
        } else if (arg_name == "column-size") {
            COLUMN_SIZE = atoi(arg_value.c_str());
        } else if (arg_name == "column-upperbound") {
            UPPERBOUND = atoi(arg_value.c_str());
        } else if (arg_name == "queries-pattern") {
            QUERIES_PATTERN = atoi(arg_value.c_str());
        } else {
            print_help(argc, argv);
            exit(EXIT_FAILURE);
        }
    }
    if (!file_exists(QUERIES_FILE_PATH)){
        Column c;
        c.data = vector<int64_t>(COLUMN_SIZE);
        load_column(&c,COLUMN_FILE_PATH,COLUMN_SIZE);
        sort(c.data.begin(), c.data.end());
        generate_query(&c.data);
    }
    else{
        fprintf(stderr,"File already exists, delete it first if you want to generate it again.\n");
    }

}
