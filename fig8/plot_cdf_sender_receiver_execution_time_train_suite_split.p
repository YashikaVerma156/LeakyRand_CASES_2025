set terminal postscript eps enhanced color size 9.5,4.5
set datafile separator ","
set lmargin 20
set rmargin 16
set bmargin 11
set tmargin 4
set xtics 1,1,6 font ", 60" offset 0,-2.5
set ytics font ", 60" offset -2
set ylabel font "helvetica, 65"
set xlabel font "helvetica, 65"
set xlabel "Sender/Receiver execution cycles" offset 45,-5
set ylabel "CDF" offset -10,-3
#set ytic auto
set yrange [0:1]
#set ytics nomirror
#set xtics nomirror

set key font ",60"
set key at screen 0.5, 0.65 center

set output "sender-receiver-lr-cdf-train.eps"
set multiplot

set border 1+2+4    
set rmargin at screen 0.36

set xrange [0:1000]
set xtics 500
set grid
plot 'cdf_output_sender_train_absolute.txt' u 1:2 title "Sender" w linespoints lw 8 pt 2 ps 3 lc rgb "#0000FF"

unset xlabel
unset ylabel
unset ytics
set y2tics font ", 60" offset -2
set y2range [0:1]
set y2tics auto
set grid y2
set border 1+4+8   
set lmargin at screen 0.55 
set rmargin at screen 0.91
set xtics 1,1,6 font ", 60" offset 0,-2.5
set xtics ("2.47*10^5" 247000,"2.5*10^5" 250000, "2.53*10^5" 253000)
set xrange [247000:253000]
set key at screen 0.5, 0.55 center
set grid
plot 'cdf_output_receiver_train_absolute.txt' u 1:2 every 3 title "Receiver" w linespoints lw 8 pt 1 ps 3 lc rgb "#FF0000" axes x1y2

unset multiplot
