#!/bin/bash

echo "TEST: basic lockfile creation"
./simple_lockfile test.lock
if [ -f test.lock ]; then
    echo "SUCCESS: lockfile created"
    rm -f test.lock
else
    echo "FAILED: lockfile not created"
fi
echo ""

echo "TEST: lockfile already exists"
touch existing.lock
./simple_lockfile -r 0 existing.lock
if [ $? -ne 0 ]; then
    echo "SUCCESS: correctly failed when lockfile exists"
else
    echo "FAILED: should have failed when lockfile exists"
fi
rm -f existing.lock
echo ""


echo "TEST: multiple lockfiles"
./simple_lockfile lock1.lock lock2.lock lock3.lock
if [ -f lock1.lock ] && [ -f lock2.lock ] && [ -f lock3.lock ]; then
    echo "SUCCESS: all lockfiles created"
    rm -f lock1.lock lock2.lock lock3.lock
else
    echo "FAILED: not all lockfiles created"
    rm -f lock1.lock lock2.lock lock3.lock
fi
echo ""


echo "TEST: cleanup on partial failure"
touch existing.lock
./simple_lockfile -r 0 temp1.lock existing.lock temp2.lock
if [ ! -f temp1.lock ] && [ ! -f temp2.lock ]; then
    echo "SUCCESS: cleanup worked on partial failure"
else
    echo "FAILED: cleanup didn't work"
    rm -f temp1.lock temp2.lock
fi
rm -f existing.lock
echo ""


echo "TEST: invert flag"
touch existing.lock
./simple_lockfile -! -r 0 existing.lock
if [ $? -eq 0 ]; then
    echo "SUCCESS: invert flag worked"
else
    echo "FAILED: invert flag didn't work"
fi
rm -f existing.lock
echo ""


echo "TEST: retry and timeout (creating competing lock)"
touch competing.lock &
LOCK_PID=$!
./simple_lockfile -r 2 -s 1 competing.lock &
TEST_PID=$!
sleep 3
kill $LOCK_PID 2>/dev/null
wait $TEST_PID
RESULT=$?
if [ $RESULT -ne 0 ]; then
    echo "SUCCESS: correctly timed out after retries"
else
    echo "FAILED: should have timed out"
fi
rm -f competing.lock
