import os
import inspect

SCRIPT_PATH =  os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe()))) # script directory
os.chdir(SCRIPT_PATH)

# Setting Values For Distributions
RANDOM = "1"
SEQUENTIAL = "2"  # Sequential only works for queries
SKEWED = "3"
MIXED = "4" # Mixed only works for queries

#Settings Pivot Types
PIVOT_EXACT_PREDICATE = "0"
PIVOT_WITHIN_QUERY_PREDICATE="1"
PIVOT_WITHIN_QUERY="2"
PIVOT_WITHIN_COLUMN="3"

#Settings Pieces To Crack Types ( Work for PIVOT_WITHIN_QUERY PIVOT_WITHIN_COLUMN)
ANY_PIECE = "0"
BIGGEST_PIECE = "1"

#Settings Pivot Selection Types (Work for : PIVOT_WITHIN_QUERY_PREDICATE PIVOT_WITHIN_QUERY PIVOT_WITHIN_COLUMN)
RANDOM_P = "0"
MEDIAN = "1"
APPROXIMATE_MEDIAN = "2"

COLUMN_SIZE = 10000000
UPPERBOUND = 100000000
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

def run_experiment(COLUMN_PATTERN,COLUMN_SIZE,COLUMN_UPPERBOUND,QUERY_PATTERN,QUERY_SELECTIVITY,PIVOT_TYPE, PIVOT_SELECTION_TYPE=0,PIECE_TO_CRACK_TYPE=0,CORRECTNESS=True):
    COLUMN_PATH = column_path(COLUMN_PATTERN,COLUMN_SIZE,COLUMN_UPPERBOUND)
    QUERY_PATH = query_path(COLUMN_PATH,QUERY_SELECTIVITY,QUERY_PATTERN)
    ANSWER_PATH = answer_path(COLUMN_PATH,QUERY_SELECTIVITY,QUERY_PATTERN)
    codestr ="./main --num-queries=" + str(NUM_QUERIES) + " --column-size=" + str(COLUMN_SIZE) + \
         " --pivot-type="+str(PIVOT_TYPE)+ " --column-path=" + str(COLUMN_PATH + "column") + " --query-path=" \
         + str(QUERY_PATH) + " --answer-path=" + str(ANSWER_PATH) + " --pivot-selection=" + str(PIVOT_SELECTION_TYPE) + " --piece-to-crack="+ str(PIECE_TO_CRACK_TYPE)
    print(codestr)
    if CORRECTNESS:
        if os.system(codestr) != 0:
            print("Failed!")
    else:
        result = os.popen(codestr).read()
        file = create_output()
        generate_output(file,result,repetition,PIVOT_TYPE,QUERY_PATTERN,COLUMN_PATTERN)

def run_all_workloads(PIVOT_TYPE, PIVOT_SELECTION_TYPE=0,PIECE_TO_CRACK_TYPE=0):
        run_experiment(RANDOM,COLUMN_SIZE,UPPERBOUND,RANDOM,QUERY_SELECTIVITY,PIVOT_TYPE,PIVOT_SELECTION_TYPE,PIECE_TO_CRACK_TYPE)
        run_experiment(RANDOM,COLUMN_SIZE,UPPERBOUND,SEQUENTIAL,QUERY_SELECTIVITY,PIVOT_TYPE,PIVOT_SELECTION_TYPE,PIECE_TO_CRACK_TYPE)
        run_experiment(RANDOM,COLUMN_SIZE,UPPERBOUND,SKEWED,QUERY_SELECTIVITY,PIVOT_TYPE,PIVOT_SELECTION_TYPE,PIECE_TO_CRACK_TYPE)
        run_experiment(RANDOM,COLUMN_SIZE,UPPERBOUND,MIXED,QUERY_SELECTIVITY,PIVOT_TYPE,PIVOT_SELECTION_TYPE,PIECE_TO_CRACK_TYPE)
        run_experiment(SKEWED,COLUMN_SIZE,UPPERBOUND,SKEWED,QUERY_SELECTIVITY,PIVOT_TYPE,PIVOT_SELECTION_TYPE,PIECE_TO_CRACK_TYPE)

def test_correctness():
    # PIVOT_TYPES_LIST = [PIVOT_EXACT_PREDICATE,PIVOT_WITHIN_QUERY_PREDICATE,PIVOT_WITHIN_QUERY]
    PIVOT_TYPES_LIST = [PIVOT_WITHIN_QUERY]

    PIVOT_SELECTION_LIST = [RANDOM_P,MEDIAN,APPROXIMATE_MEDIAN]
    PIECE_TO_CRACK_LIST = [ANY_PIECE,BIGGEST_PIECE]
    for pivot_type in PIVOT_TYPES_LIST:
        if pivot_type == PIVOT_EXACT_PREDICATE:
            run_all_workloads(pivot_type)
        if pivot_type == PIVOT_WITHIN_QUERY_PREDICATE:
            for pivot_selection in PIVOT_SELECTION_LIST:
                run_all_workloads(pivot_type,pivot_selection)
        if pivot_type == PIVOT_WITHIN_QUERY:
            for pivot_selection in PIVOT_SELECTION_LIST:
                for piece_to_crack in PIECE_TO_CRACK_LIST:
                    run_all_workloads(pivot_type,pivot_selection,piece_to_crack)
        if pivot_type == PIVOT_WITHIN_COLUMN:
            for pivot_selection in PIVOT_SELECTION_LIST:
                for piece_to_crack in PIECE_TO_CRACK_LIST:
                    run_all_workloads(pivot_type,pivot_selection,piece_to_crack)

def run():
    EXPERIMENTS = [PIVOT_WORKLOAD,PIVOT_RANDOM_WITHIN_PREDICATE_PIECE,PIVOT_APPROX_MEDIAN_WITHIN_PREDICATE_PIECE,PIVOT_MEDIAN_WITHIN_PREDICATE_PIECE]

    for experiment_test in EXPERIMENTS:
        run_experiment(RANDOM,COLUMN_SIZE,UPPERBOUND,RANDOM,QUERY_SELECTIVITY,experiment_test)
        run_experiment(RANDOM,COLUMN_SIZE,UPPERBOUND,SEQUENTIAL,QUERY_SELECTIVITY,experiment_test)
        run_experiment(RANDOM,COLUMN_SIZE,UPPERBOUND,SKEWED,QUERY_SELECTIVITY,experiment_test)
        run_experiment(RANDOM,COLUMN_SIZE,UPPERBOUND,MIXED,QUERY_SELECTIVITY,experiment_test)
        run_experiment(SKEWED,COLUMN_SIZE,UPPERBOUND,SKEWED,QUERY_SELECTIVITY,experiment_test)

test_correctness()