set terminal postscript eps enhanced color size 11.5,4.5
set grid 
set lmargin 25
set rmargin 20
set bmargin 11
set tmargin 5
set xtics font ", 60" offset 0,-2.5
set ytics font ", 60" offset -2
set ytics (0.00000001, 0.000001, 0.0001, 0.01, 1)
#set ytics font ", 60" offset -2
set ylabel font "helvetica, 65"
set xlabel font "helvetica, 65"
set xlabel "Noise injection interval in data bits" offset 1,-5
set ylabel "Bit Error Rate" offset -14,-2
set yrange [:1]
set log y 10
set format y "10^{%L}"
#set ytic auto
#set yrange [0:0.3]
#set y2tics 0,0.04,0.16 font ", 60"
#set y2range [0:0.16]
#set y2label font "helvetica, 65"
#set y2label "Bit Error Rate" offset 14, -2
set ytics nomirror

# Adjust legend spacing
#set key spacing 1  # Increase spacing (default is 1)

set key font ",60"
#set key top

set key horizontal tmargin right samplen 1.7 spacing 1.3 at screen 0.9, 1.019

set output "error-lr-noise_ds_sensitivity.eps"

#set label 1 font ",60" "4.6x10^{-4}" at 0.8,0.07 
#set label 2 font ",60" "7.8x10^{-6}" at 1.9,0.05 

plot 'error_lr_noise_ds_sensitivity.txt' u ($2+0.0000000001):xtic(1) t "|DS|: 100 (BW: 14.1 to 14.5 Kbps)" w linespoints lw 7 pt 3 ps 5.5 lc  rgb "#0000FF", \
'' u ($3+0.0000000001):xtic(1) t "|DS|: 800 (BW: 10.4 to 10.5 Kbps)" w linespoints lw 7 pt 5 ps 5.5 lc  rgb "#FF0000" 
