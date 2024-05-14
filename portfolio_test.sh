#!/bin/bash

# Output CSV file
output="results.csv"

# Write the header to the CSV file
echo "File,DPUs,Time" > $output

# Directory containing the .cnf files
dir="cnf/"

# Loop over all .cnf files in the directory
for file in $dir*.cnf
do
  # Loop from 1 to 1000 with steps of 100
  for i in $(seq 1 100 1000)
  do
    # Print the file name and the number of DPUs
    echo "Running $file with $i DPUs"

    # Run the program and measure the time
    start=$(date +%s%N)
    ./bin/host $file $i
    end=$(date +%s%N)

    # Calculate the time difference
    duration=$((($end - $start)/1000000))

    # Print the result and append it to the CSV file
    echo "Time taken for $file with $i DPUs: $duration ms"
    echo "$file,$i,$duration" >> $output
  done
done
