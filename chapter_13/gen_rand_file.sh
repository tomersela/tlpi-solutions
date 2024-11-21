#!/bin/bash

# Function to generate a file with random content
generate_random_file() {
    local file_name=$1
    local size_in_mb=$2

    # Validate inputs
    if [[ -z "$file_name" || -z "$size_in_mb" || ! "$size_in_mb" =~ ^[0-9]+$ ]]; then
        echo "Usage: $0 <file_name> <size_in_mb>"
        exit 1
    fi

    # Convert MB to bytes (1 MB = 1048576 bytes)
    local size_in_bytes=$((size_in_mb * 1048576))

    # Use dd to create a file with random content
    dd if=/dev/urandom of="$file_name" bs=1M count="$size_in_mb" status=progress

    echo "File '$file_name' with size ${size_in_mb}MB generated."
}

# Script execution
generate_random_file "$1" "$2"
