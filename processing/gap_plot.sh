#! /bin/sh
method1=edcld
method2=wrr
method3=bc
method4=sfl
for i in $(seq 1 9)
do
file1=${method1}"_"${i}"_gap.txt"
file2=${method2}"_"${i}"_gap.txt"
file3=${method3}"_"${i}"_gap.txt"
file4=${method4}"_"${i}"_gap.txt"
name="gap_"${i}".png"
gnuplot<<!
set xlabel "index" 
set ylabel "gap/ms"
set xrange [0:2000]
set yrange [0:200]
set term "png"
set output "${name}"
plot "${file1}" u 1:2 title "ecld" with points lw 2,\
"${file2}" u 1:2 title "wrr" with points lw 2,\
"${file3}" u 1:2 title "bc" with points lw 2,\
"${file4}" u 1:2 title "sfl" with points lw 2
set output
exit
!
done


