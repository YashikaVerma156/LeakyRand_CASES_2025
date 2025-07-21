set terminal postscript eps enhanced color size 13,6
set grid 
set lmargin 18
set rmargin 29
set bmargin 17
set tmargin 3
set xtics font ", 60" offset 0,-2.5
set ytics 0,10,31 font ", 60"
set ylabel font "helvetica, 65"
set xlabel font "helvetica, 65"
set xlabel "Unrolling Factor [|DS|]" offset 0,-11
set ylabel "Bandwidth (Kbps)" offset -9
#set ytic auto
set yrange [0:30]
set y2range [0:0.25]
set y2tics font ", 60"
set y2label font "helvetica, 65"
#set y2tics auto
set y2label "Bit Error Rate" offset 13
set ytics nomirror

set key font ",60"
set key left center
set offset 0.25,0
set output "bwerror-lr-unroll.eps"

set label 1 font ",60" "[12]" at -0.25,-5.5 
set label 2 font ",60" "[13]" at 0.75,-5.5 
set label 3 font ",60" "[13]" at 1.75,-5.5 
set label 4 font ",60" "[18]" at 2.75,-5.5 
set label 5 font ",60" "[30]" at 3.75,-5.5 


set label 101 font ",60" "1.1x10^{-5}" at -0.25,3 
set label 102 font ",60" "0.7x10^{-5}" at 0.75,3
set label 103 font ",60" "12.5x10^{-5}" at 1.75,3
#set label 2 font ",60" "11x10^{-5}" at 2.65,1.5 

plot 'bwerror_unroll.txt' u 2:xtic(1) title "Bandwidth" w linespoints lw 9 pt 5 ps 5.5 lc rgb "#0000FF", \
'' u 3:xtic(1) t "BER" w linespoints lw 9 pt 3 ps 5.5 lc  rgb "#FF0000" axes x1y2

