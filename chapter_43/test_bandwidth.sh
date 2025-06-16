#!/bin/sh

PROGRAM_NAME=$1

echo "Bandwidth Measurement Test: $PROGRAM_NAME"
echo "==============================="
echo

# function to determine number of blocks based on block size
# all tests use ~1.6GB total data for fair comparison
get_num_blocks() {
    case $1 in
        64)    echo 26000000 ;;  # ~1.6GB total
        256)   echo 6500000 ;;   # ~1.6GB total
        1024)  echo 1600000 ;;   # ~1.6GB total  
        4096)  echo 400000 ;;    # ~1.6GB total
        8192)  echo 200000 ;;    # ~1.6GB total
        16384) echo 100000 ;;    # ~1.6GB total
        32768) echo 50000 ;;     # ~1.6GB total
        65536) echo 25000 ;;     # ~1.6GB total
        *)     echo 50000 ;;     # default
    esac
}

BLOCK_SIZES="64 256 1024 4096 8192 16384 32768 65536"

echo "Testing with varying block numbers (more blocks for smaller sizes):"
echo

for size in $BLOCK_SIZES; do
    num_blocks=$(get_num_blocks $size)
    echo "----------------------------------------"
    echo "Block size: $size bytes (${num_blocks} blocks)"
    echo "----------------------------------------"
    ./$PROGRAM_NAME $num_blocks $size
    echo
done

echo "==============================="
echo "Test completed"
 