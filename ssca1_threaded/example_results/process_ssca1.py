#!/usr/bin/python

import re
import sys

elapsed_filter = re.compile("\tElapsed time:")
number_filter = re.compile("[0-9]+")
thread_filter = re.compile("Running with OpenMP, thread count: ([0-9]+)")

def shred_ssca1_input(file_path):
    input_lines = []

    with open(file_path,'r') as result_file:
        input_lines = result_file.readlines()

    kernel1_time = filter(lambda x: elapsed_filter.match(x), input_lines)[1]
    time_array = map(lambda x: int(number_filter.search(x).group(0)), kernel1_time.split(","))

    elapsed_seconds = time_array[0] * 60 * 60 + time_array[1] * 60 + time_array[2]
    if time_array[3] > 500:
        elapsed_seconds += 1

    thread_line = filter(lambda x: thread_filter.match(x), input_lines)[0]
    thread_count = int(number_filter.search(thread_line).group(0))

    return (thread_count, elapsed_seconds)

def read_input_list(input_file_path):
    files_to_read = []
    with open(input_file_path) as input_file:
        files_to_read = input_file.readlines()
    return map(lambda x: x.strip(), files_to_read)

def write_dat(output_file_path, output_data):
    lines = map(lambda x: ("%d\t%d"% (x[0], x[1])), output_data)
    file_content = "\n".join(lines)
    with open(output_file_path, 'w') as output_file:
        output_file.write(file_content)

ssca1_files = read_input_list(sys.argv[1])
times = map(lambda x: shred_ssca1_input(x), ssca1_files)
write_dat("ssca1_times.dat", times)
