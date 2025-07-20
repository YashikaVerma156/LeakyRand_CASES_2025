
# Define a function that takes parameters
count_total_mem_src_access_btwn_two_lines() {
    local line1="$1"  # Get the first parameter and store it in a local variable
    local line2="$2"
    local pattern="$3"
    local file_name="$4"
    echo "Hello, $line1 $line2 $pattern ${file_name}!"
    sed -n "${line1}, ${line2} p" ${file_name} | grep ${pattern} | wc
}
