#!/bin/bash

output="gadi_256.txt"

for dir in mac_o_128_*thr/; do
    # Strip trailing slash, then extract the number before 'thr'
    dirname="${dir%/}"
    nthr=$(echo "$dirname" | sed 's/.*_\([0-9]*\)thr$/\1/')

    logfile="$dir/profile.log"
    if [[ -f "$logfile" ]]; then
        runtime=$(grep "Total" "$logfile" | awk '{print $3}')
        echo "$nthr $runtime" >> "$output"
    else
        echo "WARNING: $logfile not found" >&2
    fi
done

sort -t',' -k1 -n -o "$output" "$output" <(head -1 "$output")
echo "Done. Results in $output"
