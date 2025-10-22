
for ds in 5750 3000 1000
do
    for uf in 16 32 64 128
    do
        python3 plot_for_0_and_1_whole_suite.py 0 $uf $ds
    done
done
