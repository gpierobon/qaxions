#!/bin/bash

INPUT=$1
TYPE=$2

output="${INPUT}_$TYPE.txt"
if [ -f $output ]; then
    rm $output
fi

for dir in ${INPUT}_*thr/; do
    # Strip trailing slash, then extract the number before 'thr'
    dirname="${dir%/}"
    nthr=$(echo "$dirname" | sed 's/.*_\([0-9]*\)thr$/\1/')

    logfile="$dir/profile.log"
    if [[ -f "$logfile" ]]; then
        case $2 in 
        total)
            runtime=$(grep "Total" "$logfile" | awk '{print $3}')
        ;;
        kick)
            runtime=$(grep "KICK" "$logfile" | awk '{print $2}')
        ;;
        drift) 
            runtime=$(grep "DRIFT" "$logfile" | awk '{print $2}')
        ;;
        poisson) 
            runtime=$(grep "POISSON" "$logfile" | awk '{print $2}')
        ;;
        fft) 
            runtime=$(grep "FFT" "$logfile" | awk '{print $2}' | awk 'NR==2')
        ;;
        esac
        echo "$nthr $runtime" >> "$output"
    else
        echo "WARNING: $logfile not found" >&2
    fi
done

sort -k1 -n -o "$output" "$output"
echo "Results in $output"
