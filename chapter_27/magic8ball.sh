# Define Magic 8-Ball responses (one per line)
RESPONSES=$(cat <<EOF
Yes
No
Maybe
Ask again later
Definitely
I don't think so
It is certain
Very doubtful
EOF
)

# Count number of responses
TOTAL=$(echo "$RESPONSES" | wc -l)

# Generate a random number using awk (since POSIX sh lacks $RANDOM)
random_number() {
    awk -v max="$TOTAL" 'BEGIN { srand(); print int(rand() * max) + 1 }'
}

# Pick a random response
random_response() {
    echo "$RESPONSES" | awk "NR == $(random_number)"
}

# Ask user for input
echo "🎱 Ask the Magic 8-Ball a yes/no question:"
read question

# Wait for suspense...
echo "Thinking..."
sleep 2

# Show the answer
echo "Magic 8-Ball says: $(random_response)"

