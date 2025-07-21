# Set terminal and output
set terminal postscript eps enhanced color size 9in, 5in
set output 'region_identification_99_percent.eps'

set grid
set lmargin 11
set rmargin 17
set bmargin 7
set tmargin 4
set xtics font ", 29" offset 0,-0.6
set ytics font ", 29"
set ylabel font "helvetica, 37"
set xlabel font "helvetica, 37"

# Set titles and labels
#set title "Sample Bar Chart"
set xlabel "X" offset 0,-2
set ylabel "LLC blocks occupied(%)" offset -2


# Define custom x-tics
#set xtics ("1" 1, "2" 2, "3" 3)

# Define custom x-tics with increased gap
set xtics 6
set ytic auto
set yrange [60:100]
# Adjust the range to control the distance of the first bar from the y-axis
#set xrange [-0.5:2.5]

# Set style for bars
#set style data histogram
#set style histogram clustered gap 1
#set style fill solid border -1

# Define a variable for the font size
label_font = ",32"  # Change this value to adjust label font size

# Read values from files using system and awk
fc = system("awk '$1==1 {print $2}' temp.txt")
pc = system("awk '$1==1 {print $3}' temp.txt")

avg = system("awk '$1==1 {print $2}' region_99.txt")
min = system("awk '$1==1 {print $3}' region_99.txt")
max = system("awk '$1==1 {print $4}' region_99.txt")

# For x==1
set label 1 sprintf("FC: %s", fc) at 0.5,73.5 font label_font
set label 2 sprintf("PC: %s", pc) at 0.5,70.5 font label_font
set label 3 sprintf("min %s", min) at 0.3,67.5 font label_font
set label 4 sprintf("avg %s", avg) at 0.3,64.5 font label_font
set label 5 sprintf("max %s", max) at 0.3,61.5 font label_font

# Read values from files using system and awk
fc = system("awk '$1==2 {print $2}' temp.txt")
pc = system("awk '$1==2 {print $3}' temp.txt")

avg = system("awk '$1==2 {print $2}' region_99.txt")
min = system("awk '$1==2 {print $3}' region_99.txt")
max = system("awk '$1==2 {print $4}' region_99.txt")

#for x==2
set label 6 sprintf("FC: %s", fc) at 0.3,93.5 font label_font
set label 7 sprintf("PC: %s", pc) at 0.3,90.5 font label_font
set label 8 sprintf("min %s", min) at 0.35,87.5 font label_font
set label 9 sprintf("avg %s", avg) at 0.35,84.5 font label_font
set label 10 sprintf("max %s", max) at 0.35,81.5 font label_font

# Read values from files using system and awk
fc = system("awk '$1==3 {print $2}' temp.txt")
pc = system("awk '$1==3 {print $3}' temp.txt")

avg = system("awk '$1==3 {print $2}' region_99.txt")
min = system("awk '$1==3 {print $3}' region_99.txt")
max = system("awk '$1==3 {print $4}' region_99.txt")

#for x==3
set label 13 sprintf("min %s", min) at 1.82,93 font label_font
set label 14 sprintf("avg %s", avg) at 1.82,90 font label_font
set label 15 sprintf("max %s", max) at 1.82,87 font label_font
set label 11 sprintf("FC: %s", fc) at 1.4,84 font label_font
set label 12 sprintf("PC: %s", pc) at 1.4,81 font label_font

# Read values from files using system and awk
fc = system("awk '$1==4 {print $2}' temp.txt")
pc = system("awk '$1==4 {print $3}' temp.txt")

avg = system("awk '$1==4 {print $2}' region_99.txt")
min = system("awk '$1==4 {print $3}' region_99.txt")
max = system("awk '$1==4 {print $4}' region_99.txt")

#for x==4
set label 16 sprintf("min %s", min) at 2.8,96 font label_font
set label 17 sprintf("avg %s", avg) at 2.8,93 font label_font
set label 18 sprintf("max %s", max) at 2.8,90 font label_font
set label 19 sprintf("FC: %s", fc) at 2.7,87 font label_font
set label 20 sprintf("PC: %s", pc) at 2.7,84 font label_font

# Read values from files using system and awk
fc = system("awk '$1==5 {print $2}' temp.txt")
pc = system("awk '$1==5 {print $3}' temp.txt")

avg = system("awk '$1==5 {print $2}' region_99.txt")
min = system("awk '$1==5 {print $3}' region_99.txt")
max = system("awk '$1==5 {print $4}' region_99.txt")

#for x==5
set label 21 sprintf("min %s", min) at 4.1,102 font label_font
set label 22 sprintf("avg %s", avg) at 4.1,99 font label_font
set label 23 sprintf("max %s", max) at 4.1,96 font label_font
set label 24 sprintf("FC: %s", fc) at 3.7,93 font label_font
set label 25 sprintf("PC: %s", pc) at 3.7,90 font label_font

#legend
#set key font ",27"
#set key right bottom

# Set the box width
set boxwidth 0.9

#Remove the legend.
unset key

# Plot the data
plot 'region_99.txt' u 2:xtic(1) w linespoints lw 5 pt 5 ps 2.5 lc rgb "#0000FF"

