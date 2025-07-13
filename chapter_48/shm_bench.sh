#!/usr/bin/env bash
IN=$1
OUT=$2
SIZES="1024 2048 4096 8192 16384 32768"

# cleanup - remove shared-memory and semaphore IPC instances
ipcrm -M 0x1234 -S 0x5678 2>/dev/null || true

TIMEFORMAT=%R
printf "%8s  %10s\n" "BUF" "seconds"
printf "%8s  %10s\n" "----" "-------"

for B in $SIZES; do
    # rebuild with BUF_SIZE
    make -s clean
    make -s BUF_SIZE=$B

    ./svshm_xfr_writer < "$IN" 2> "writer.$B.log" &
    W=$!

    T=$( { time ./svshm_xfr_reader > "$OUT" 2> "reader.$B.log"; } 2>&1 )
    wait $W                                   # writer fully gone

    # per-iteration cleanup - remove shared-memory and semaphore IPC instances
    ipcrm -M 0x1234 -S 0x5678 2>/dev/null || true

    printf "%8d  %10s\n" "$B" "$T"
done
