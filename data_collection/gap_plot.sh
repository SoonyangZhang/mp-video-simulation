#! /bin/sh
file1=scale_gap.txt
file2=wrr_gap.txt
file3=bc_gap.txt
gnuplot<<!
set xlabel "index" 
set ylabel "gap/ms"
set xrange [0:1000]
set yrange [0:200]
set term "png"
set output "gap.png"
plot "${file1}" u 1:2 title "scale" with points lw 2,\
"${file2}" u 1:2 title "wrr" with points lw 2,\
"${file3}" u 1:2 title "bc" with points lw 2
set output
exit
!
