#!/bin/sh

aprun -N 1 -n 2 ./shoms --input test_bw.txt --minsize 4096 | tee test_bw_results.txt
aprun -N 1 -n 2 ./shoms --input test_latency.txt --maxsize 32 | tee test_lat_results.txt
aprun -N 1 -n 2 ./shoms --input test_others.txt --maxsize 8 | tee test_others_results.txt
aprun -N 1 -n 2 ./shoms --affinity --maxsize 32 | tee test_affinity_results.txt
