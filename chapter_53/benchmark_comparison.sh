#!/bin/bash

echo "Semaphore Performance Comparison"
echo "================================"
echo
printf "| Operations | POSIX Time (s) | System V Time (s) | Speedup |\n"
printf "|------------|----------------|-------------------|----------|\n"

# Test with different operation counts
test_counts=(1000 10000 100000 1000000)

for count in "${test_counts[@]}"; do
    # Run POSIX benchmark and extract elapsed time
    posix_time=$(./benchmark_posix_sem $count | grep "Elapsed time:" | awk '{print $3}')
    
    # Run System V benchmark and extract elapsed time  
    sysv_time=$(./benchmark_sysv_sem $count | grep "Elapsed time:" | awk '{print $3}')
    
    # Calculate speedup ratio
    speedup=$(echo "scale=1; $sysv_time / $posix_time" | bc -l)
    
    printf "| %10s | %14s | %17s | %7.1fx |\n" "$count" "$posix_time" "$sysv_time" "$speedup"
done
