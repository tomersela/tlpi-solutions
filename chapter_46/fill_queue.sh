#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 <msqid>"
    exit 1
fi

MSQID="$1"
COUNT=0

MSG=$(head -c 1023 < /dev/zero | tr '\0' A)

while ./svmsg_send -n "$MSQID" 1 "$MSG"; do
    COUNT=$((COUNT + 1))
done

echo "Queue $MSQID filled after $COUNT messages."
