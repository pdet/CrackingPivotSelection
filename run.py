import os
import inspect

SCRIPT_PATH =  os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe()))) # script directory
os.chdir(SCRIPT_PATH)

# Setting Values For Distributions
RANDOM = "1"
SEQUENTIAL = "2"  # Sequential only works for queries
SKEWED = "3"
MIXED = "4" # Mixed only works for queries


# Setting Values For Pivoting
PIVOT_WORKLOAD = "0";
PIVOT_RANDOM_WITHIN_PIECE = "1";
PIVOT_RANDOM_MEDIAN_WITHIN_PIECE = "2";
PIVOT_MEDIAN_WITHIN_PIECE = "3";
PIVOT_RANDOM = "4";
PIVOT_RANDOM_MEDIAN = "5";
PIVOT_MEDIAN = "6";

COLUMN_SIZE = 10000000
UPPERBOUND = 100000000
NUM_QUERIES = 1000
QUERY_SELECTIVITY = 0.01


print("Compiling")
os.environ['OPT'] = 'true'
if os.system('cmake -DCMAKE_BUILD_TYPE=Release && make') != 0:
    print("Make Failed")
    exit()

def column_path(COLUMN_DISTRIBUTION,COLUMN_SIZE,COLUMN_UPPERBOUND):
    path = "generated_data/" +str(COLUMN_DISTRIBUTION) + "_" + str(COLUMN_SIZE) + "_" + str(COLUMN_UPPERBOUND)
    os.system('mkdir -p '+ path)
    return path+ "/"

def query_path(EXPERIMENT_PATH, SELECTIVITY_PERCENTAGE,QUERIES_PATTERN):
    return EXPERIMENT_PATH + "query_" + str(SELECTIVITY_PERCENTAGE) + "_" + str(QUERIES_PATTERN)

def answer_path(EXPERIMENT_PATH, SELECTIVITY_PERCENTAGE,QUERIES_PATTERN):
    return EXPERIMENT_PATH + "answer_" + str(SELECTIVITY_PERCENTAGE) + "_" + str(QUERIES_PATTERN)

def generate_column(COLUMN_DISTRIBUTION,COLUMN_SIZE,COLUMN_UPPERBOUND,COLUMN_PATH):
    print("Generating Column")
    codestr = "./generate_column --column-pattern=" + str(COLUMN_DISTRIBUTION) + " --column-size=" + str(COLUMN_SIZE) + " --column-upperbound=" + str(COLUMN_UPPERBOUND) + " --column-path=" + str(COLUMN_PATH)
    print (codestr)
    if os.system(codestr) != 0:
        print("Generating Column Failed")
        exit()

def generate_query(NUM_QUERIES,COLUMN_SIZE, COLUMN_UPPERBOUND, COLUMN_PATH, QUERY_PATH,ANSWER_PATH, SELECTIVITY_PERCENTAGE,QUERIES_PATTERN):
    print("Generating Queries")
    codestr = "./generate_query --num-queries=" + str(NUM_QUERIES) + " --column-size=" + str(COLUMN_SIZE) + " --column-upperbound=" + str(COLUMN_UPPERBOUND) \
              + " --column-path=" + str(COLUMN_PATH)+ "column" + " --query-path=" + str(QUERY_PATH) + " --answer-path=" + str(ANSWER_PATH) + " --selectivity=" \
              + str(SELECTIVITY_PERCENTAGE) + " --queries-pattern=" +  str(QUERIES_PATTERN)

    print (codestr)
    if os.system(codestr) != 0:
        print("Generating Queries Failed")
        exit()

# Uniform Random Column Distribution
experiment_path = column_path(RANDOM,COLUMN_SIZE,UPPERBOUND)
generate_column(RANDOM,COLUMN_SIZE,UPPERBOUND,experiment_path + "column")
#All Query Patterns
QUERY_PATTERNS = [RANDOM,SEQUENTIAL,SKEWED,MIXED]
for query in QUERY_PATTERNS:
    q_path = query_path(experiment_path,QUERY_SELECTIVITY,query)
    a_path = answer_path(experiment_path,QUERY_SELECTIVITY,query)
    generate_query(NUM_QUERIES,COLUMN_SIZE,UPPERBOUND,experiment_path,q_path,a_path,QUERY_SELECTIVITY,query)

# Skewed Column Distribution
experiment_path = column_path(SKEWED,COLUMN_SIZE,UPPERBOUND)
generate_column(SKEWED,COLUMN_SIZE,UPPERBOUND,experiment_path + "column")
# Skewed Queries
q_path = query_path(experiment_path,QUERY_SELECTIVITY,MIXED)
a_path = answer_path(experiment_path,QUERY_SELECTIVITY,MIXED)
generate_query(NUM_QUERIES,COLUMN_SIZE,UPPERBOUND,experiment_path,q_path,a_path,QUERY_SELECTIVITY,MIXED)
