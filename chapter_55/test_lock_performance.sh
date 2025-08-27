#!/bin/bash

TEST_FILE="locktest.dat"
ITERATIONS=10000

if ! pgrep -f "lock_acquirer.*$TEST_FILE" > /dev/null; then
    echo "Start lock_acquirer first: ./lock_acquirer $TEST_FILE &"
    exit 1
fi

for n in 0 10000 20000 30000 40000; do
    echo "N=$n (byte $((n * 2)))"
    time ./lock_tester "$TEST_FILE" "$n" "$ITERATIONS"
    echo
done
