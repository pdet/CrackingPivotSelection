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

COLUMN_SIZE = 100000000
UPPERBOUND = 1000000000
NUM_QUERIES = 1000
QUERY_SELECTIVITY = 0.01
NUMBER_OF_REPETITIONS = 10


print("Compiling")
os.environ['OPT'] = 'true'
if os.system('cmake -DCMAKE_BUILD_TYPE=Release && make') != 0:
    print("Make Failed")
    exit()

def getFolderToSaveExperiments(folder=""):
    global PATH
    if os.path.exists("ResultsCSV/+folder") != 1:
        os.system('mkdir -p ResultsCSV/'+folder)
    experimentsList = os.listdir("ResultsCSV/"+folder)
    aux = 0
    for experiment in experimentsList:
        if experiment != ".DS_Store":
            if aux < int(experiment):
                aux = int(experiment)
    currentexperiment = aux + 1
    PATH = "ResultsCSV/"+  folder+ str(currentexperiment) + '/'
    os.system('mkdir -p ' + PATH)


def translate_alg(alg):
    if alg == '0':
        return 'pw'
    if alg == '1':
        return 'prwp'
    if alg == '2':
        return 'prmwp'
    if alg == '3':
        return 'pmwp'
    if alg == '4':
        return 'pr'
    if alg == '5':
        return 'prm'
    if alg == '6':
        return 'pm'
    return alg

#Output is a csv file with:
# "algorithm;repetition;query_number;query_pattern;column_pattern;query_time"
def generate_output(file,query_result,repetition,ALGORITHM,QUERY_PATTERN,COLUMN_PATTERN):
    query_result = query_result.split("\n")
    converged = ""
    for query in range(0, len(query_result)-1):
        if "CONVERGED" in query_result[query]:
            converged = query_result[query].split(':')[1]
            pass
        file.write(translate_alg(ALGORITHM) + ';' + str(repetition) + ";" + str(query) + ";" + str(QUERY_PATTERN) + ';' + str(COLUMN_PATTERN)  + ";"+ query_result[query])
        file.write('\n')
    file.close()

# Saving Experiments
def create_output():
    header = "algorithm;repetition;query_number;query_pattern;column_pattern;query_time"
    file = open(PATH + "results.csv", "w")
    file.write(header)
    file.write('\n')
    return file

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
q_path = query_path(experiment_path,QUERY_SELECTIVITY,SKEWED)
a_path = answer_path(experiment_path,QUERY_SELECTIVITY,SKEWED)
generate_query(NUM_QUERIES,COLUMN_SIZE,UPPERBOUND,experiment_path,q_path,a_path,QUERY_SELECTIVITY,SKEWED)


def run_experiment_correctness(COLUMN_PATTERN,COLUMN_SIZE,COLUMN_UPPERBOUND,QUERY_PATTERN,QUERY_SELECTIVITY,experiment_test):
    COLUMN_PATH = column_path(COLUMN_PATTERN,COLUMN_SIZE,COLUMN_UPPERBOUND)
    QUERY_PATH = query_path(COLUMN_PATH,QUERY_SELECTIVITY,QUERY_PATTERN)
    ANSWER_PATH = answer_path(COLUMN_PATH,QUERY_SELECTIVITY,QUERY_PATTERN)
    for algorithm in experiment_test:
        codestr ="./main --num-queries=" + str(NUM_QUERIES) + " --column-size=" + str(COLUMN_SIZE) + \
                 " --pivot-type="+str(algorithm)+ " --column-path=" + str(COLUMN_PATH + "column") + " --query-path=" \
                 + str(QUERY_PATH) + " --answer-path=" + str(ANSWER_PATH)
        print(codestr)
        if os.system(codestr) != 0:
            print("Failed!")

def run_experiment(COLUMN_PATTERN,COLUMN_SIZE,COLUMN_UPPERBOUND,QUERY_PATTERN,QUERY_SELECTIVITY,experiment_test):
    for repetition in range(NUMBER_OF_REPETITIONS):
        getFolderToSaveExperiments(COLUMN_PATTERN+"_"+QUERY_PATTERN+ "/")
        COLUMN_PATH = column_path(COLUMN_PATTERN,COLUMN_SIZE,COLUMN_UPPERBOUND)
        QUERY_PATH = query_path(COLUMN_PATH,QUERY_SELECTIVITY,QUERY_PATTERN)
        ANSWER_PATH = answer_path(COLUMN_PATH,QUERY_SELECTIVITY,QUERY_PATTERN)
        for algorithm in experiment_test:
            codestr ="./main --num-queries=" + str(NUM_QUERIES) + " --column-size=" + str(COLUMN_SIZE) + \
                     " --pivot-type="+str(algorithm)+ " --column-path=" + str(COLUMN_PATH + "column") + " --query-path=" \
                     + str(QUERY_PATH) + " --answer-path=" + str(ANSWER_PATH)
            print(codestr)
            result = os.popen(codestr).read()
            file = create_output()
            generate_output(file,result,repetition,algorithm,QUERY_PATTERN,COLUMN_PATTERN) 

def test_correctness():
    EXPERIMENTS = [PIVOT_WORKLOAD,PIVOT_RANDOM_WITHIN_PIECE,PIVOT_RANDOM_MEDIAN_WITHIN_PIECE,PIVOT_MEDIAN_WITHIN_PIECE,
                   PIVOT_RANDOM,PIVOT_RANDOM_MEDIAN,PIVOT_MEDIAN]
    for experiment_test in EXPERIMENTS:
        run_experiment_correctness(RANDOM,COLUMN_SIZE,UPPERBOUND,RANDOM,QUERY_SELECTIVITY,experiment_test)
        run_experiment_correctness(RANDOM,COLUMN_SIZE,UPPERBOUND,SEQUENTIAL,QUERY_SELECTIVITY,experiment_test)
        run_experiment_correctness(RANDOM,COLUMN_SIZE,UPPERBOUND,SKEWED,QUERY_SELECTIVITY,experiment_test)
        run_experiment_correctness(RANDOM,COLUMN_SIZE,UPPERBOUND,MIXED,QUERY_SELECTIVITY,experiment_test)
        run_experiment_correctness(SKEWED,COLUMN_SIZE,UPPERBOUND,SKEWED,QUERY_SELECTIVITY,experiment_test)

def run():
    EXPERIMENTS = [PIVOT_WORKLOAD,PIVOT_RANDOM_WITHIN_PIECE,PIVOT_RANDOM_MEDIAN_WITHIN_PIECE,PIVOT_MEDIAN_WITHIN_PIECE,
                   PIVOT_RANDOM,PIVOT_RANDOM_MEDIAN,PIVOT_MEDIAN]
    for experiment_test in EXPERIMENTS:
        run_experiment(RANDOM,COLUMN_SIZE,UPPERBOUND,RANDOM,QUERY_SELECTIVITY,experiment_test)
        run_experiment(RANDOM,COLUMN_SIZE,UPPERBOUND,SEQUENTIAL,QUERY_SELECTIVITY,experiment_test)
        run_experiment(RANDOM,COLUMN_SIZE,UPPERBOUND,SKEWED,QUERY_SELECTIVITY,experiment_test)
        run_experiment(RANDOM,COLUMN_SIZE,UPPERBOUND,MIXED,QUERY_SELECTIVITY,experiment_test)
        run_experiment(SKEWED,COLUMN_SIZE,UPPERBOUND,SKEWED,QUERY_SELECTIVITY,experiment_test)
