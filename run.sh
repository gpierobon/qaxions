#/bin/zsh

TYPE=$1

case $TYPE in
    0)
        # Static
        ./qaxions --nmeas 200 --steps 10000 --meas 8 --nthr 10 --dt 0.00005 --fft 1 --dim 2 --N 1024 --dir o0
        ;;
    1)
        # MRE
        ./qaxions --nmeas 200 --steps 10000 --meas 8 --nthr 10 --ai 0.05 --readj --norm 1.0 --dt 0.0001 --fft 1 --cosmo 1 --dir o1
        ;;
esac
