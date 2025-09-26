#!/bin/bash

for i in {1..20}; do
    {
        echo "Connection $i starting"
        sleep 4  # leep connection alive to demonstrate semaphore limiting
        echo "Connection $i ending"
    } | nc -q 0 localhost 7 &
    echo "Started connection $i"
    sleep 0.1
done

# monitor for 6 seconds to see semaphore limiting the number of child processes created
echo "Number child processes:"
for i in {1..30}; do
    server_pid=$(pgrep -x is_echo_sv_mod | head -1)
    if [ -n "$server_pid" ]; then
        count=$(pgrep -P $server_pid 2>/dev/null | wc -l)
    else
        count=0
    fi
    echo "$(date '+%H:%M:%S') - Child processes: $count"
    sleep 0.2
done
echo "Done"