#!/bin/bash

# Define arguments
FORK_COMMANDS=("fork" "vfork")
ALLOCATION_SIZES=(1782579 2831155 12268339)  # Converted from MB to bytes

# Number of runs per combination
RUNS=10

# Output file
OUTPUT_FILE="benchmark_results.txt"
echo "Benchmark results for fork_bench" > "$OUTPUT_FILE"
echo "---------------------------------" >> "$OUTPUT_FILE"

# Function to measure time
measure_time() {
    local fork_cmd=$1
    local alloc_size=$2
    local total_elapsed=0
    local total_cpu=0

    echo "Running: ./fork_bench $fork_cmd 100000 $alloc_size (10 runs)"
    echo "---------------------------------"

    for ((i=1; i<=RUNS; i++)); do
        echo "Iteration $i: ./fork_bench $fork_cmd 100000 $alloc_size"

        # Run the command and capture output
        result=$(./fork_bench "$fork_cmd" 100000 "$alloc_size")

        # Extract elapsed and CPU time from the program output
        elapsed=$(echo "$result" | grep "Elapsed time" | awk '{print $3}')
        cpu_time=$(echo "$result" | grep "CPU time" | awk '{print $3}')

        # Ensure values are numeric before summing
        if [[ $elapsed =~ ^[0-9]+(\.[0-9]+)?$ ]] && [[ $cpu_time =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
            # Print results for each iteration
            echo "Elapsed time: $elapsed sec, CPU time: $cpu_time sec"

            # Accumulate times
            total_elapsed=$(awk "BEGIN {print $total_elapsed + $elapsed}")
            total_cpu=$(awk "BEGIN {print $total_cpu + $cpu_time}")
        else
            echo "Error parsing output: $result"
            continue
        fi
    done

    # Calculate averages
    avg_elapsed=$(awk "BEGIN {print $total_elapsed / $RUNS}")
    avg_cpu=$(awk "BEGIN {print $total_cpu / $RUNS}")

    # Print and save results
    echo "Average elapsed time: $avg_elapsed sec"
    echo "Average CPU time: $avg_cpu sec"
    echo ""

    echo "$fork_cmd 100000 $alloc_size -> Elapsed: $avg_elapsed sec, CPU: $avg_cpu sec" >> "$OUTPUT_FILE"
}

# Iterate over all combinations
for fork_cmd in "${FORK_COMMANDS[@]}"; do
    for alloc_size in "${ALLOCATION_SIZES[@]}"; do
        measure_time "$fork_cmd" "$alloc_size"
    done
done

echo "Benchmarking complete. Results saved to $OUTPUT_FILE."
