#!/bin/bash

# Ensure correct number of arguments
if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <source_file> <output_file> <device_path> <mount_folder>"
    exit 1
fi

# Input parameters
SOURCE_FILE=$1
OUTPUT_FILE_PARAM=$2
DEVICE_PATH=$3
MOUNT_FOLDER=$4

# Define the buffer sizes to test
BUF_SIZES=(1 2 4 8 16 32 64 128 256 512 1024 4096 16284 65536)

# Output file for the results
RESULTS_FILE="${SOURCE_FILE}_res.csv"

# Check if the source file exists
if [ ! -f "$SOURCE_FILE" ]; then
    echo "Error: Source file '${SOURCE_FILE}' does not exist."
    exit 1
fi

# Initialize the results file with a header
echo "Buffer Size,Elapsed Time (s),Total CPU Time (s),User CPU Time (s),System CPU Time (s)" > $RESULTS_FILE

# Loop over each buffer size
for BUF_SIZE in "${BUF_SIZES[@]}"; do
    echo "Measuring for BUF_SIZE=${BUF_SIZE}..."

    # Compile the program with the current BUF_SIZE
    cc -std=c99 -D_XOPEN_SOURCE=600 -D_DEFAULT_SOURCE -DBUF_SIZE=${BUF_SIZE} \
       -g -I../lib -pedantic -Wall -W -Wmissing-prototypes -Wno-sign-compare \
       -Wimplicit-fallthrough -Wno-unused-parameter copy.c ../libtlpi.a -o copy

    # Check if compilation was successful
    if [ $? -ne 0 ]; then
        echo "Compilation failed for BUF_SIZE=${BUF_SIZE}. Skipping..."
        continue
    fi

    # Initialize accumulators
    total_elapsed=0
    total_cpu=0
    total_user=0
    total_system=0

    # Run the program 20 times and collect statistics
    for i in {1..20}; do
        echo "  Running iteration $i/20 for BUF_SIZE=${BUF_SIZE}..."

        # Unmount and remount the drive to bypass OS cache
        echo "    Unmounting ${DEVICE_PATH} from ${MOUNT_FOLDER}..."
        sudo umount "$MOUNT_FOLDER"
        if [ $? -ne 0 ]; then
            echo "Error: Failed to unmount ${MOUNT_FOLDER}."
            exit 1
        fi

        echo "    Remounting ${DEVICE_PATH} to ${MOUNT_FOLDER}..."
        sudo mount "$DEVICE_PATH" "$MOUNT_FOLDER"
        if [ $? -ne 0 ]; then
            echo "Error: Failed to remount ${DEVICE_PATH}."
            exit 1
        fi

        # Measure time with the time command
        TIME_OUTPUT=$( (time ./copy "$SOURCE_FILE" "$OUTPUT_FILE_PARAM") 2>&1)
        
        # Parse the time output
        elapsed=$(echo "$TIME_OUTPUT" | grep real | awk '{print $2}' | sed 's/m/:/g' | awk -F: '{print ($1 * 60) + $2}')
        user=$(echo "$TIME_OUTPUT" | grep user | awk '{print $2}' | sed 's/m/:/g' | awk -F: '{print ($1 * 60) + $2}')
        system=$(echo "$TIME_OUTPUT" | grep sys | awk '{print $2}' | sed 's/m/:/g' | awk -F: '{print ($1 * 60) + $2}')

        # Calculate total CPU time (user + system)
        cpu=$(echo "$user + $system" | bc)

        # Print the results of the current iteration
        echo "    Iteration $i results: Elapsed=${elapsed}s, Total CPU=${cpu}s, User CPU=${user}s, System CPU=${system}s"

        # Accumulate the times
        total_elapsed=$(echo "$total_elapsed + $elapsed" | bc)
        total_cpu=$(echo "$total_cpu + $cpu" | bc)
        total_user=$(echo "$total_user + $user" | bc)
        total_system=$(echo "$total_system + $system" | bc)
    done

    # Compute averages
    avg_elapsed=$(echo "$total_elapsed / 20" | bc -l)
    avg_cpu=$(echo "$total_cpu / 20" | bc -l)
    avg_user=$(echo "$total_user / 20" | bc -l)
    avg_system=$(echo "$total_system / 20" | bc -l)

    # Append results to the output file
    echo "$BUF_SIZE,$avg_elapsed,$avg_cpu,$avg_user,$avg_system" >> $RESULTS_FILE
done

echo "Results saved to $RESULTS_FILE."
