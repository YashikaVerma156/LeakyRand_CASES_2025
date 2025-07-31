# Set terminal and output
set terminal postscript eps enhanced color size 6,4
set output 'bw_llc_size_sensitivity_high_UF.eps'

set grid
set lmargin 19
set rmargin 17
set bmargin 7
set tmargin 3
set xtics font ", 50" offset 0,-0.9
set ytics font ", 50"
set ylabel font "helvetica, 50"
set xlabel font "helvetica, 50"

# Set titles and labels
#set title "Sample Bar Chart"
set xlabel "LLC Size" offset 0,-3
set ylabel "Bandwidth (Kbps)" offset -8


# Define custom x-tics
#set xtics ("1" 1, "2" 2, "3" 3)
set ytic auto
set yrange [0:60]
# Adjust the range to control the distance of the first bar from the y-axis
set xrange [-0.5:2.5]

# Set style for bars
set style data histogram
set style histogram clustered gap 1
set style fill solid border -1

set key font ",48"
set key right top

set label 100 font ",50" "|DS|" at 1.5,33.5
set arrow 1 from first 1.4,33  to 1.08, 33 lt 1 lw 2 front size .25, 18
#set arrow 2 from first -0.3,10.65  to 0.75, 9.1 lt 1 lw 2 front size .25, 18
set label 1 font ",50" "17" at -0.29,54
set label 2 font ",50" "13" at 0.66,33
set label 3 font ",50" "15" at 1.66,17.5


# Set the box width
set boxwidth 0.9

# Plot the data
plot 'llc_size_sensitivity.txt' using 2:xtic(1) title "LeakyRand" fillstyle pattern 5 lc rgb "#0000FF", \
     '' using 4:xtic(1) title 'SPP'  fillstyle pattern 19 lc rgb "#FFC1BE"


