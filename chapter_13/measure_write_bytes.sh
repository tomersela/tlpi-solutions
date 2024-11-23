#!/bin/bash

# Ensure the output file argument is provided
if [ $# -ne 1 ]; then
    echo "Usage: $0 <output_file>"
    exit 1
fi

output_file=$1  # First argument to the script is the output file name
csv_file="results.csv"

# Configuration
buffer_sizes=(1 2 4 8 16 32 64 128 256 512 1024 4096 16284 65536)
iterations=20
total_tests=$(( ${#buffer_sizes[@]} * iterations ))
current_test=0
file_size_bytes=$((32 * 1024 * 1024))

# Write CSV header
echo "buffer_size,elapsed_time,total_cpu_time,user_cpu_time,system_cpu_time" > $csv_file

# Function to compute averages
compute_averages() {
    local times=("$@")
    local total=0
    for time in "${times[@]}"; do
        total=$(echo "$total + $time" | bc -l)
    done
    echo "scale=4; $total / ${#times[@]}" | bc -l
}

# Main loop
for buffer_size in "${buffer_sizes[@]}"; do
        
    elapsed_times=()
    user_cpu_times=()
    system_cpu_times=()

    for ((i = 1; i <= iterations; i++)); do
        # Increment the test count
        current_test=$((current_test + 1))
        progress=$((current_test * 100 / total_tests))

        # Print progress
        echo -ne "Running: Buffer Size=$buffer_size, Iteration=$i/$iterations, Progress=$progress% \r"

        # Run the write_bytes program and capture timing details

        TIME_OUTPUT=$( (time ./write_bytes "$output_file" "$file_size_bytes" "$buffer_size") 2>&1)
        
        # Parse the time output
        elapsed=$(echo "$TIME_OUTPUT" | grep real | awk '{print $2}' | sed 's/m/:/g' | awk -F: '{print ($1 * 60) + $2}')
        user_cpu=$(echo "$TIME_OUTPUT" | grep user | awk '{print $2}' | sed 's/m/:/g' | awk -F: '{print ($1 * 60) + $2}')
        system_cpu=$(echo "$TIME_OUTPUT" | grep sys | awk '{print $2}' | sed 's/m/:/g' | awk -F: '{print ($1 * 60) + $2}')

        # Calculate total CPU time (user + system)
        cpu=$(echo "$user_cpu + $system_cpu" | bc)

        # Print the results of the current iteration
        echo "    [buff=$buffer_size] Iteration $i results: Elapsed=${elapsed}s, Total CPU=${cpu}s, User CPU=${user_cpu}s, System CPU=${system_cpu}s"

        elapsed_times+=("$elapsed")
        user_cpu_times+=("$user_cpu")
        system_cpu_times+=("$system_cpu")
    done

    # Compute averages
    avg_elapsed=$(compute_averages "${elapsed_times[@]}")
    avg_user_cpu=$(compute_averages "${user_cpu_times[@]}")
    avg_system_cpu=$(compute_averages "${system_cpu_times[@]}")
    avg_total_cpu=$(echo "$avg_user_cpu + $avg_system_cpu" | bc -l)

    # Append to CSV
    echo "$buffer_size,$avg_elapsed,$avg_total_cpu,$avg_user_cpu,$avg_system_cpu" >> $csv_file
done

# Final message
echo -e "\nResults written to $csv_file"
