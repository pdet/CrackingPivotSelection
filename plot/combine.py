import os
import sys
import inspect

SCRIPT_PATH =  os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe()))) # script directory

def combine_results(directory, base,output):
	global first_file
	current_directory = os.path.join(base, directory)
	print current_directory
	files = os.listdir(current_directory)
	for f in files:
		fullpath = os.path.join(current_directory, f)
		if os.path.isdir(fullpath):
			combine_results(f, current_directory,output)
		elif f == 'results.csv':
			with open(fullpath, 'r') as new_file:
				for line in new_file:
					if line[-1] != '\n':
						continue
					if line.count(';') == 4:
						splits = line.split(';')
						splits.insert(len(splits)-1, '')
						line = ';'.join(splits)
					if line.count(';') != 5:
						continue
					try:
						float(line.split(';')[-1])
						output.write(line)
					except:
						pass
			output.flush()

def run(directory,base):
	output = open('output'+base+'.csv', 'w+')
	output.write("algorithm;repetition;query_number;query_pattern;column_pattern;query_time\n")

	if len(sys.argv) > 1:
		os.chdir(sys.argv[1])
	combine_results(directory,base,output)

def plot(experiment_directory,experiment_base,final_folder):
	run(experiment_directory,experiment_base)
	os.system("mkdir "+final_folder)
	os.system("mv output"+experiment_base+".csv "+final_folder+"/output.csv")
	os.system("cp plot.r "+final_folder+"/")
	os.chdir(os.path.join(SCRIPT_PATH,final_folder))
	os.system('Rscript plot.r')
	os.system('rm plot.r')
	os.chdir(os.path.join(SCRIPT_PATH))

os.chdir(SCRIPT_PATH)
os.chdir("../")
BASE_PATH = os.path.join(os.getcwd(),'ResultsCSV')
print BASE_PATH
os.chdir(SCRIPT_PATH)
if os.path.exists(os.path.join(BASE_PATH,"1_1")):
	plot(os.path.join(BASE_PATH,"1_1"),'1_1',"RandomQueries")
if os.path.exists(os.path.join(BASE_PATH,"1_2")):
	plot(os.path.join(BASE_PATH,"1_2"),'1_2',"SequentialQueries")
if os.path.exists(os.path.join(BASE_PATH,"1_3")):
	plot(os.path.join(BASE_PATH,"1_3"),'1_3',"SkewedQueries")
if os.path.exists(os.path.join(BASE_PATH,"1_4")):
	plot(os.path.join(BASE_PATH,"1_4"),'1_4',"MixedQueries")
if os.path.exists(os.path.join(BASE_PATH,"3_3")):
	plot(os.path.join(BASE_PATH,"3_3"),'3_3',"SkewedColumn")