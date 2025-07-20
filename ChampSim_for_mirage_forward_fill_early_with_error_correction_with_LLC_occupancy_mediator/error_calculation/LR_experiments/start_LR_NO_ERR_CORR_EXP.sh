for unroll_fact in 1 2 4 8 16 32
do

    # Print the value
    echo “Iteration for $unroll_fact”
    ./calibration_and_variables.sh $unroll_fact
done
