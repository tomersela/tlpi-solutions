#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 <command>"
    exit 1
fi

command="$@"
runs=10
total_real=0

for i in $(seq 1 $runs); do
    echo -n "Run $i: "
    result=$( (time -p $command) 2>&1 | grep real | awk '{print $2}' )
    echo "Elapsed time: ${result}s"
    total_real=$(echo "$total_real + $result" | bc)
done

average_real=$(echo "scale=3; $total_real / $runs" | bc)
echo "----------------------------------"
echo "Average real time: $average_real seconds"
