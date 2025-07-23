# Set terminal and output
set terminal postscript eps enhanced color size 13,4
set output 'region_identification_best_dyn.eps'

set grid
set lmargin 28
set rmargin 9
set bmargin 11
set tmargin 3
set xtics font ", 60" offset 0.9,-1.6
set ytics 60,20,100 font ", 60" offset -1
set ylabel font "helvetica, 70"
set xlabel font "helvetica, 70"

# Set titles and labels
#set title "Sample Bar Chart"
set xlabel "Steps through Occupancy Sequence" offset 0,-5
set ylabel "LLC blocks \n\n\n\n occupied(%)" offset -13,-4.5


# Define custom x-tics
#set xtics ("1" 1, "2" 2, "3" 3)
#set ytic auto
set yrange [60:100]
# Adjust the range to control the distance of the first bar from the y-axis
#set xrange [-0.5:2.5]

# Set style for bars
#set style data histogram
#set style histogram clustered gap 1
#set style fill solid border -1

set key font ",60"
set key right bottom


# Set the box width
#set boxwidth 0.9

# Plot the data
#plot 'best_dyn.txt' using 2:xtic(1) title "Estimated" fillstyle pattern 5 lc rgb "#0000FF", \
#     '' using 4:xtic(1) title 'Experimented'

plot 'best_dyn.txt' u 2:xtic(1) title "Estimated" w linespoints lw 14 pt 5 ps 6.5 lc rgb "#0000FF", \
'' u 3:xtic(1) t "Experimented" w linespoints lw 14 pt 3 ps 6.5 lc  rgb "#FF0000"

