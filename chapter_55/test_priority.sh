#!/bin/bash
echo "Starting at: $(date '+%H:%M:%S')"

touch testfile

echo ""
echo "Starting exclusive lock holder (15 seconds)..."
./t_flock testfile x 15 &

echo ""
echo "Queueing lock requests in order:"
./t_flock testfile s 3 &
./t_flock testfile x 3 &
./t_flock testfile s 3 &
./t_flock testfile x 3 &
./t_flock testfile s 3 &
./t_flock testfile x 3 &
./t_flock testfile s 3 &
./t_flock testfile x 3 &

# At this point all processes are queued.

echo ""
echo "Waiting for all background processes to complete..."
wait
