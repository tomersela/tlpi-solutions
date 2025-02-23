#!/usr/bin/env python3
import re
import sys
import argparse
import matplotlib.pyplot as plt
import matplotlib.ticker as mticker

def main():
    parser = argparse.ArgumentParser(
        description="Analyze thread interleaving, lost updates, and overlapping (stale) updates from thread output."
    )
    parser.add_argument("input_file", help="Path to the text file with thread output")
    args = parser.parse_args()

    # Regular expression to match lines like: [<thread_id>] glob = <value>
    pattern = re.compile(r"\[(\d+)\]\s+glob\s*=\s*(\d+)")
    entries = []

    # Read and parse the file
    with open(args.input_file, "r") as f:
        for line in f:
            m = pattern.search(line)
            if m:
                tid = int(m.group(1))
                glob_val = int(m.group(2))
                entries.append((tid, glob_val))

    if not entries:
        print("No valid entries found in the file.")
        sys.exit(1)

    total_lines = len(entries)
    # Count context switches: when consecutive entries are from different threads
    context_switches = sum(
        1 for i in range(1, total_lines) if entries[i][0] != entries[i - 1][0]
    )

    # Count "lost update" events: ideally each update increases glob by 1
    lost_updates = sum(
        1 for i in range(1, total_lines) if (entries[i][1] - entries[i - 1][1]) != 1
    )

    # Count updates per thread
    count_by_thread = {}
    for tid, _ in entries:
        count_by_thread[tid] = count_by_thread.get(tid, 0) + 1

    # --- Overlap Analysis ---
    # Detect cases where a thread prints a stale global value (less than the maximum seen so far)
    # and then, in its next update from that thread, increments from that stale value.
    overlap_events = []

    # Pre-calculate the cumulative global maximum value up to each line.
    cumulative_global_max = []
    current_max = -1
    for (_, value) in entries:
        if value > current_max:
            current_max = value
        cumulative_global_max.append(current_max)

    # For each entry (except the first), if its value is less than the previous global max, it is stale.
    # Look for the next update from the same thread that increments exactly by 1.
    for i, (tid, value) in enumerate(entries):
        if i == 0:
            continue
        if value < cumulative_global_max[i - 1]:
            # This entry appears stale relative to earlier updates.
            # Look ahead for the next update from the same thread.
            for j in range(i + 1, len(entries)):
                if entries[j][0] == tid:
                    if entries[j][1] == value + 1:
                        overlap_events.append({
                            "thread": tid,
                            "stale_index": i,
                            "stale_value": value,
                            "next_index": j,
                            "new_value": entries[j][1]
                        })
                    break

    # --- Output Summary ---
    print("=== Analysis Summary ===")
    print(f"Total updates (lines): {total_lines}")
    print(f"Context switches: {context_switches}")
    print(f"Lost update events (non-sequential increments): {lost_updates}")
    print("Updates per thread:")
    for tid, count in count_by_thread.items():
        print(f"  Thread {tid}: {count} updates")
    print(f"Overlap events (stale read followed by an increment from that stale value): {len(overlap_events)}")
    if overlap_events:
        print("Details of overlap events:")
        for event in overlap_events:
            print(f"  Thread {event['thread']} at line {event['stale_index']+1} read stale value {event['stale_value']} "
                  f"and then at line {event['next_index']+1} incremented to {event['new_value']}.")

    # --- Plotting ---
    x = list(range(total_lines))
    y = [val for (_, val) in entries]

    plt.figure(figsize=(10, 6))
    plt.plot(x, y, marker=".", linestyle="-", label="Global Value")
    
    # Mark overlap events with red dashed lines
    for idx, event in enumerate(overlap_events):
        i = event["stale_index"]
        j = event["next_index"]
        plt.plot([i, j], [event["stale_value"], event["new_value"]],
                 marker="o", linestyle="--", color="red",
                 label="Overlap event" if idx == 0 else "")
    
    plt.xlabel("Log Entry Index")
    plt.ylabel("Global Value")
    plt.title("Global Variable Progression with Overlap Events")
    plt.grid(True)
    plt.legend()

    # Use custom formatters to always show full numbers on both axes without scientific notation
    ax = plt.gca()
    ax.yaxis.set_major_formatter(mticker.StrMethodFormatter('{x:,.0f}'))
    ax.xaxis.set_major_formatter(mticker.StrMethodFormatter('{x:,.0f}'))

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()
