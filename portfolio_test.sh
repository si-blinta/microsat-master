#!/bin/bash

output="results.csv"

echo "File,DPUs,Time" > $output

dir="cnf/"

for file in $dir*.cnf
do
  for i in $(seq 1 100 1000)
  do
    echo "Running $file with $i DPUs"

    start=$(date +%s%N)
    ./bin/host $file $i
    end=$(date +%s%N)

    duration=$((($end - $start)/1000000))

    echo "Time taken for $file with $i DPUs: $duration ms"
    echo "$file,$i,$duration" >> $output
  done
done
