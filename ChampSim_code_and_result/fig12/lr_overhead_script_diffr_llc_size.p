set terminal postscript eps enhanced color size 8,4
set output 'overhead_lr_diffr_llc_size.eps'

set grid y
set lmargin 27
#set rmargin 27
set bmargin 9
set tmargin 2
set xtics font ", 60" offset 0,-1.5
set ytics 0,0.2,1 font ", 60"
set ylabel font "helvetica, 60"
set xlabel font "helvetica, 60"

# Set titles and labels
set xlabel "LLC size" offset 0,-4
set ylabel "Normalized \n \n \n \n cycles" offset -14,-2

# Adjust the range to control the distance of the first bar from the y-axis
set xrange [-0.5:2.5]

# Set style for bars
set style data histogram
set style histogram rowstacked
#set style histogram clustered gap 1
set style fill pattern border -1

set key font "-20,55" 
#set key outside right
set key Left
set key left top
set key invert
set key reverse
set key at -0.58,1.197

#set style line 2 lc rgb 'black' lt 1 lw 1
set boxwidth 0.5
set ytics nomirror
set yrange [:1.199]

#plot 'lr_cycles_normalized.txt' using ($2/100) t 'Data bits' ls 2, '' using ($3/100) t 'Marker bits' ls 2, '' using ($4/100):xtic(1) t 'Error correction' ls 2
#plot 'lr_cycles_normalized_diffr_llc_size.txt' using ($2) t 'Data bits' ls 2, '' using ($3/100) t 'Marker bits' fillstyle pattern 3 lc rgb "#EB4848", '' using ($4/100):xtic(1) t 'Error correction' fillstyle pattern 4 lc rgb "#8080FF"
plot 'lr_cycles_normalized_diffr_llc_size.txt' using ($2/100) t 'CRFill' ls 6, '' using ($3/100) t 'CRProbe' ls 10, '' using ($4/100) t 'Data bits' lc rgb "#F27E7E", '' using ($5/100) t 'Marker bits' fillstyle pattern lc rgb "#202048", '' using ($6/100):xtic(1) t 'Error correction' fillstyle pattern 4 lc rgb "#8080FF"
