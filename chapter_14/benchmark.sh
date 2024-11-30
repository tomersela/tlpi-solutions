#!/bin/bash

# Check for correct usage
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <benchmark_command> <target_folder>"
    echo "Example: $0 'sudo ./fs_benchmark' /mnt/jfs-drive"
    exit 1
fi

# Read the parameters
BENCHMARK_COMMAND=$1
TARGET_FOLDER=$2

# Output CSV file for results
OUTPUT_FILE="results.csv"

# Write the CSV header
echo "File Count,Avg Create Time (s),Avg Delete Time (s)" > $OUTPUT_FILE

# Total number of iterations (file counts)
total_iterations=$(seq 1000 1000 20000 | wc -l)
current_iteration=0

# Iterate through file counts from 1000 to 20000 with a step of 1000
for file_count in $(seq 1000 1000 20000); do
    # Update the current iteration count
    current_iteration=$((current_iteration + 1))
    
    total_create_time=0
    total_delete_time=0

    # Run the benchmark 10 times
    for i in {1..10}; do
        # Display dynamic status at the bottom
        printf "\rProcessing $file_count files... ($current_iteration/$total_iterations), Run $i/10"
        result=$($BENCHMARK_COMMAND "$file_count" "$TARGET_FOLDER")

        # Extract the create and delete times
        create_time=$(echo "$result" | grep "Create:" | awk '{print $2}')
        delete_time=$(echo "$result" | grep "Delete:" | awk '{print $2}')
        
        # Accumulate the times
        total_create_time=$(echo "$total_create_time + $create_time" | bc)
        total_delete_time=$(echo "$total_delete_time + $delete_time" | bc)
    done

    # Calculate the averages
    avg_create_time=$(echo "scale=2; $total_create_time / 10" | bc)
    avg_delete_time=$(echo "scale=2; $total_delete_time / 10" | bc)

    # Write the results to the CSV file
    echo "$file_count,$avg_create_time,$avg_delete_time" >> $OUTPUT_FILE

    # Notify completion of this file count
    printf "\rCompleted benchmark for $file_count files. ($current_iteration/$total_iterations)\n"
done

echo "All benchmarks completed. Results saved to $OUTPUT_FILE."
