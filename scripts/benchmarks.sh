#!/bin/bash

make clean && make -j4

for nthr in {1..20} 
do
    ./qaxions --N 128 --nthr $nthr --steps 1000 \
              --dir linux_128_${nthr}thr
done

for t in total kick drift poisson fft
do
    source scripts/collect-times.sh linux_128 $t
done
