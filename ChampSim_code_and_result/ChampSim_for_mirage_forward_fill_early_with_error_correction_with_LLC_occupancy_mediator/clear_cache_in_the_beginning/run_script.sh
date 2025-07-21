num_itr=8
while [ $num_itr -ge 1 ];
do
    echo "================================= $num_itr ==================================="
    ./clear_cache_basic_script.sh
    num_itr=`expr ${num_itr} - 1`
done
