#! /bin/sh
file1=scale.txt
file2=wrr.txt
file3=bc.txt
file4=edcld.txt
file5=sfl.txt
file6=water.txt
gnuplot<<!
set xlabel "index" 
set ylabel "delay/ms"
set xrange [0:7]
set yrange [0:600]
set term "png"
set output "delay.png"
plot "${file1}" u 1:2 title "scale" with lines lw 2,\
"${file2}" u 1:2 title "wrr" with lines lw 2,\
"${file3}" u 1:2 title "bc" with lines lw 2,\
"${file4}" u 1:2 title "edcld" with lines lw 2,\
"${file5}" u 1:2 title "sfl" with lines lw 2
set output
exit
!
#"${file5}" u 1:2 title "sfl" with points,\
#"${file6}" u 1:2 title "water" with points

#"${file3}" u 1:2 title "bc" with points,\
#"${file4}" u 1:2 title "edcld" with points,\
