# Set terminal and output
set terminal postscript eps enhanced color size 9,3.5
set output 'replacement_llc_miss_io_calls.eps'

set grid
set lmargin 20
set rmargin 6
set bmargin 8
set tmargin 3
set xtics font ", 50" offset 0,-0.9
set ytics font ", 50"
set ylabel font "helvetica, 50"
set xlabel font "helvetica, 50"

# Set titles and labels
#set title "Sample Bar Chart"
set xlabel "Array Size" offset 0,-3
set ylabel "Normalized\n\n\n LLC misses" offset -8.5


# Define custom x-tics
#set xtics ("1" 1, "2" 2, "3" 3, "4" 4, "5" 5)
set ytic auto
set yrange [0.8:3]
# Adjust the range to control the distance of the first bar from the y-axis
set xrange [-0.5:4.5]

# Set style for bars
set style data histogram
set style histogram clustered gap 1
set style fill solid border -1

set key font ",48"
set key left top

#set label 100 font ",50" "|DS|" at 1.5,33.5
#set arrow 1 from first 1.4,33  to 1.08, 33 lt 1 lw 2 front size .25, 18
#set arrow 2 from first -0.3,10.65  to 0.75, 9.1 lt 1 lw 2 front size .25, 18
#set label 1 font ",50" "17" at -0.29,54
#set label 2 font ",50" "13" at 0.66,33
#set label 3 font ",50" "15" at 1.66,17.5


# Set the box width
set boxwidth 0.9

# Plot the data
plot 'llc_miss_io_calls.txt' using 3:xtic(2) title "2 Iterations" fillstyle pattern 5 lc rgb "#0000FF", \
     '' using 4:xtic(2) title '4 Iterations'  fillstyle pattern 4 lc rgb "#0000FF", \
     '' using 5:xtic(2) title '6 Iterations'  fillstyle pattern 17 lc rgb "#0000FF", \
     '' using 6:xtic(2) title '8 Iterations'  fillstyle pattern 19 lc rgb "#FFC1BE"


