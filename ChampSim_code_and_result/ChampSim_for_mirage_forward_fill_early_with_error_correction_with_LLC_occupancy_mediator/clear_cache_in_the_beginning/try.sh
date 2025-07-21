a=`grep -nri "fence: 1" result_random_1_champsim.trace_clear_cache.gz.txt | head -1 | awk '{ print $11}'`
echo "$a"
a1=`grep -nr "Retiring instr_id: ${a} " result_random_1_champsim.trace_clear_cache.gz.txt | head -1 | awk '{print $10}'`
echo "$a1"
