set terminal postscript eps enhanced color size 9.5,4.5
set grid 
set lmargin 20
set rmargin 28
set bmargin 11
set tmargin 4
set xtics font ", 60" offset 0,-2.5
set ytics 0,4,16 font ", 60" offset -2
set ylabel font "helvetica, 65"
set xlabel font "helvetica, 65"
set xlabel "Disturbance Set Size" offset 0,-5
set ylabel "Bandwidth (Kbps)" offset -9,-3
#set ytic auto
set yrange [0:16]
set y2tics 0,0.04,0.16 font ", 60"
set y2range [0:0.16]
set y2label font "helvetica, 65"
set y2label "Bit Error Rate" offset 14, -2
set ytics nomirror

set key font ",60"
set key right center

set output "bwerror-lr-ds.eps"

set label 1 font ",60" "105x10^{-5}" at 1.55,2.5 
set label 2 font ",60" "11x10^{-5}" at 2.75,2.5 

plot 'bwerror_ds.txt' u 2:xtic(1) title "Bandwidth" w linespoints lw 7 pt 5 ps 5.5 lc rgb "#0000FF", \
'' u 3:xtic(1) t "BER" w linespoints lw 7 pt 3 ps 5.5 lc  rgb "#FF0000" axes x1y2
