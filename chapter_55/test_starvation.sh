#!/bin/bash

touch testfile

echo "Starting shared lock holder (15 seconds)..."
./t_flock testfile s 15 &

sleep 2

echo "Starting exclusive lock request (will wait)..."
./t_flock testfile x 10 &
EXCLUSIVE_PID=$!

sleep 2

echo "Adding more shared lock requests while exclusive waits..."
./t_flock testfile s 5 &
./t_flock testfile s 5 &
./t_flock testfile s 5 &

wait
