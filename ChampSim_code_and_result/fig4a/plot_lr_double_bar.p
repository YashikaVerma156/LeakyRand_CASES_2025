set terminal postscript eps enhanced color size 7,4
set grid 
set lmargin 14
set bmargin 7
set tmargin 9
set rmargin 12
set xtics font ", 50" offset 0,-1.5
set ytics 0,6,18 font ", 50" out
set ylabel font "helvetica, 55"
set xlabel font "helvetica, 55"
set xlabel "Error correction interval" offset 0,-2.7
set ylabel "Bandwidth (Kbps)" offset -5
#set ytic auto
set yrange [0:18]
set y2tics font ", 50"
set y2label font "helvetica, 50"
set log y2 10
set format y2 "10^{%L}" 
set y2range [:1]

set ytics nomirror
set boxwidth 1.3 relative
set style data histograms
set style histogram cluster gap 1
set style fill solid border -1

set xrange [-0.47:6.5]

set output "bwerror-99-999.eps"

set key font ", 44"
set key horizontal tmargin left samplen 1.7 spacing 1.3
#set key at screen 0.05,1

set label 100 font ",40" "|DS|" at -0.35,13 
set arrow 1 from first -0.2,11.69  to -0.15, 8.4 lt 1 lw 2 front size .25, 18
set arrow 2 from first -0.2,11.69  to 0.15, 9.4 lt 1 lw 2 front size .25, 18
#set arrow 2 from first -0.3,10.65  to 0.7, 9.1 lt 1 lw 2 front size .25, 18
set label 1 font ",50" "4" at -0.37,7.5 
set label 2 font ",50" "5" at 0.6,9.7 
set label 3 font ",50" "6" at 1.6,11.75
set label 4 font ",50" "9" at 2.6,12.8
set label 5 font ",50" "12" at 3.45,14.0
set label 6 font ",50" "25" at 4.45,14.55
set label 7 font ",50" "98" at 5.45,11.8

set label 8 font ",50" "3" at 0.00,8.3
set label 9 font ",50" "3" at 1.0,11.2
set label 10 font ",50" "3" at 2.0,13.3
set label 11 font ",50" "4" at 3.0,14.3
set label 12 font ",50" "4" at 4.0,15.3
# Define background rectangle for the label
#set object 1 rect from 4.9,15 to 5.3,15.5 fc rgb "gray80" behind
set label 13 font ",50" "5" at 5.0,15.8
set label 14 font ",50" "9" at 6.0,15.8

set y2tics (0.00000001, 0.000001, 0.0001, 0.01, 1)
#set y2tics add ("0" 10^-10)

plot 'bwerror_99_999.txt' u 2:xtic(1) title "BW(98.88%)" fillstyle pattern 5 lc rgb "#8080FF", '' u 9:xtic(1) title "BW(99.76%)" fillstyle pattern 19 lc rgb "#FFC1BE", \
'' u ($0 - 0.25):($3 + 0.0000000001) t "Err(98.88%)" w linespoints lw 8 pt 7 ps 2.5 lc  rgb "#0000FF" axes x1y2, \
'' u ($0 + 0.25):($10 + 0.0000000001) t "Err(99.76%)" w linespoints lw 8 pt 4 ps 2.5 lc  rgb "#FF0000" axes x1y2

#plot 'bwerror_99_999.txt' u 2:xtic(1) title "BW: 98.88%" fillstyle pattern 5 lc rgb "#8080FF", '' u 9:xtic(1) title "99.76%" fillstyle pattern 19 lc rgb "#FFC1BE", \
#'' u ($0 - 0.25):($3*100000) t "Err: 98.88%" w linespoints lw 8 pt 7 ps 2.5 lc  rgb "#0000FF" axes x1y2, \
#'' u ($0 + 0.25):($10*100000) t "99.76%" w linespoints lw 8 pt 4 ps 2.5 lc  rgb "#FF0000" axes x1y2


#plot 'bwerror_99_999.txt' u 2:xtic(1) title "BW: 98.88%" fillstyle pattern 5 lc rgb "#8080FF", '' u 9:xtic(1) title "99.76%" fillstyle pattern 19 lc rgb "#FFC1BE", \
#'' u ($0 - 0.25):(($3 == 0) ? log10(1) : log10($3)) t "Err: 98.88%" w linespoints lw 8 pt 7 ps 2.5 lc  rgb "#0000FF" axes x1y2, \
#'' u ($0 + 0.25):(($10 == 0) ? log10(1) : log10($10)) t "99.76%" w linespoints lw 8 pt 4 ps 2.5 lc  rgb "#FF0000" axes x1y2
