set terminal postscript eps enhanced color size 7,4
set grid 
set lmargin 8
set rmargin 18
set bmargin 8
set tmargin 9
set xtics font ", 50" offset 0,-1.5
set ytics 0,6,18 font ", 50" out
set ylabel font "helvetica, 50"
set xlabel font "helvetica, 50"
set xlabel "Sender array size (% of LLC)" offset 0,-2.8
#set ylabel "Bandwidth (Kbps)" offset -5
#set ytic auto
set yrange [0:18]
set y2label "Bit Error Rate" offset 9

set y2tics font ", 50"
set y2label font "helvetica, 55"
set log y2 10
set format y2 "10^{%L}"
set y2range [:1]

set ytics nomirror
set boxwidth 1.3 relative
set style data histograms
set style histogram cluster gap 1
set style fill solid border -1

set xrange [-0.48:4.45]
set y2tics (0.00000001, 0.000001, 0.0001, 0.01, 1)
set output "bwerror-spp-with-without.eps"

set key font ", 46"
#set key at 1, 20
set key horizontal tmargin left samplen 1.7 spacing 1.3
#set key horizontal at 2, 25 samplen 1.5 spacing 0.75

#plot 'bwerror_spp_with_without.txt' u 3:xtic(1) title "BW: ref" fillstyle pattern 5 lc rgb "#8080FF", '' u 6:xtic(1) title "no ref" fillstyle pattern 19 lc rgb "#FFC1BE", \
#'' u ($0 - 0.25):($4*100000) t "Err: ref" w linespoints lw 8 pt 7 ps 2.5 lc  rgb "#0000FF" axes x1y2, \
#'' u ($0 + 0.25):($7*100000) t "no ref" w linespoints lw 8 pt 4 ps 2.5 lc  rgb "#FF0000" axes x1y2

plot 'bwerror_spp_with_without.txt' u 3:xtic(1) title "BW(+ref)" fillstyle pattern 5 lc rgb "#8080FF", '' u 6:xtic(1) title "BW(-ref)" fillstyle pattern 19 lc rgb "#FFC1BE", \
'' u ($0 - 0.25):($4 + 0.0000000001) t "Err(+ref)" w linespoints lw 8 pt 7 ps 2.5 lc  rgb "#0000FF" axes x1y2, \
'' u ($0 + 0.25):($7 + 0.0000000001) t "Err(-ref)" w linespoints lw 8 pt 4 ps 2.5 lc  rgb "#FF0000" axes x1y2
