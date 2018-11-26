#include <vector>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <cassert>
#include <map>
#include <chrono>
#include <algorithm>
#include "file_manager.h"

using namespace std;

int64_t COLUMN_SIZE, UPPERBOUND, DATA_COLUMN_PATTERN;
string COLUMN_FILE_PATH;
//#define DEBUG

void random_pattern(vector<int64_t> *data){
    vector<int64_t> allPossibleValues;

    int64_t lower_value = 0;
    int64_t upper_value = UPPERBOUND * 0.01;
    for (size_t k = 0; k < 100; k ++){
        for (size_t i = lower_value; i < upper_value ; i++){
            allPossibleValues.push_back(i);
        }
        random_shuffle(allPossibleValues.begin(), allPossibleValues.end());
        for (size_t i = 0; i < COLUMN_SIZE*0.01 ; i++){
            data->push_back(allPossibleValues[i]);
        }
        allPossibleValues.erase(allPossibleValues.begin(),allPossibleValues.end());
        lower_value = upper_value;
        upper_value += UPPERBOUND * 0.01;
    }
    random_shuffle(data->begin(), data->end());
}

void skewed_pattern(vector<int64_t> *data){
    vector<int64_t> allPossibleValues;
    int64_t middle_value = UPPERBOUND/2;
    int64_t bottom_value = middle_value - COLUMN_SIZE * 0.4;
    int64_t upper_value = middle_value + COLUMN_SIZE * 0.4;

//        10% of the values are randomly distributed on top
//        80% on middle
//        10% on bottom
    for (size_t i = 0; i < bottom_value ; i++){
        allPossibleValues.push_back(i);
    }
    random_shuffle(allPossibleValues.begin(), allPossibleValues.end());
    for (size_t i = 0; i < COLUMN_SIZE * 0.1 ; i++){
        data->push_back(allPossibleValues[i]);
    }
    allPossibleValues.erase(allPossibleValues.begin(), allPossibleValues.end());
    for (size_t i = bottom_value; i < upper_value ; i++){
        data->push_back(i);
    }
    for (size_t i = upper_value; i < UPPERBOUND ; i++){
        allPossibleValues.push_back(i);
    }
    random_shuffle(allPossibleValues.begin(), allPossibleValues.end());
    size_t i = 0;
    while(data->size() != COLUMN_SIZE){
        data->push_back(allPossibleValues[i]);
        i++;
    }
    random_shuffle(data->begin(), data->end());
}

vector<int64_t> generate_column() {
    chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> elapsed_seconds;

    FILE* f = fopen(COLUMN_FILE_PATH.c_str(), "w+");

    start = chrono::system_clock::now();
    vector<int64_t> data;
    if(DATA_COLUMN_PATTERN == 1){
        random_pattern(&data);
    }
    else if(DATA_COLUMN_PATTERN == 3){
        skewed_pattern(&data);
    }
    end = chrono::system_clock::now();
    #ifdef DEBUG
    for (size_t i = 0; i < COLUMN_SIZE; i ++)
        cout << i+1 << ";" << data[i] << "\n";
    #endif

    fwrite(&data[0], sizeof(int64_t), COLUMN_SIZE, f);
    fclose(f);
    elapsed_seconds = end-start;
    cout << "Creating column data: " << elapsed_seconds.count() << "s\n";
    return data;
}

void print_help(int argc, char** argv) {
    fprintf(stderr, "Unrecognized command line option.\n");
    fprintf(stderr, "Usage: %s [args]\n", argv[0]);
    fprintf(stderr, "   --column-path\n");
    fprintf(stderr, "   --column-size\n");
    fprintf(stderr, "   --column-pattern\n");
    fprintf(stderr, "   --column-upperbound\n");
}

pair<string,string> split_once(string delimited, char delimiter) {
    auto pos = delimited.find_first_of(delimiter);
    return { delimited.substr(0, pos), delimited.substr(pos+1) };
}

int main(int argc, char** argv) {
    COLUMN_FILE_PATH =  "column";
    COLUMN_SIZE = 100000000;
    DATA_COLUMN_PATTERN = 1;
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
        }  else if (arg_name == "column-size") {
            COLUMN_SIZE = atoi(arg_value.c_str());
        } else if (arg_name == "column-upperbound") {
            UPPERBOUND = atoi(arg_value.c_str());
        }  else if (arg_name == "column-pattern") {
            DATA_COLUMN_PATTERN = atoi(arg_value.c_str());
        } else {
            print_help(argc, argv);
            exit(EXIT_FAILURE);
        }
    }
    if (!file_exists(COLUMN_FILE_PATH)){
        generate_column();
    }
    else{
        fprintf(stderr,"File already exists, delete it first if you want to generate it again.\n");
    }

}