#! /bin/sh
method1=edcld
method2=wrr
method3=bc
method4=sfl
i=1
file1=${method1}"_"${i}"_rate.txt"
file2=${method2}"_"${i}"_rate.txt"
file3=${method3}"_"${i}"_rate.txt"
file4=${method4}"_"${i}"_rate.txt"
name="rate_"${i}".png"
gnuplot<<!
set xlabel "time/s" 
set ylabel "rate/kbps"
set xrange [0:200]
set yrange [0:4000]
set term "png"
set output "${name}"
plot "${file1}" u 1:2 title "ecld" with lines lw 2,\
"${file2}" u 1:2 title "wrr" with lines lw 2,\
"${file3}" u 1:2 title "bc" with lines lw 2,\
"${file4}" u 1:2 title "sfl" with lines lw 2
set output
exit
!


i=2
file1=${method1}"_"${i}"_rate.txt"
file2=${method2}"_"${i}"_rate.txt"
file3=${method3}"_"${i}"_rate.txt"
file4=${method4}"_"${i}"_rate.txt"
name="rate_"${i}".png"
gnuplot<<!
set xlabel "time/s" 
set ylabel "rate/kbps"
set xrange [0:200]
set yrange [0:5000]
set term "png"
set output "${name}"
plot "${file1}" u 1:2 title "ecld" with lines lw 2,\
"${file2}" u 1:2 title "wrr" with lines lw 2,\
"${file3}" u 1:2 title "bc" with lines lw 2,\
"${file4}" u 1:2 title "sfl" with lines lw 2
set output
exit
!

i=3
file1=${method1}"_"${i}"_rate.txt"
file2=${method2}"_"${i}"_rate.txt"
file3=${method3}"_"${i}"_rate.txt"
file4=${method4}"_"${i}"_rate.txt"
name="rate_"${i}".png"
gnuplot<<!
set xlabel "time/s" 
set ylabel "rate/kbps"
set xrange [0:200]
set yrange [0:5000]
set term "png"
set output "${name}"
plot "${file1}" u 1:2 title "ecld" with lines lw 2,\
"${file2}" u 1:2 title "wrr" with lines lw 2,\
"${file3}" u 1:2 title "bc" with lines lw 2,\
"${file4}" u 1:2 title "sfl" with lines lw 2
set output
exit
!

i=4
file1=${method1}"_"${i}"_rate.txt"
file2=${method2}"_"${i}"_rate.txt"
file3=${method3}"_"${i}"_rate.txt"
file4=${method4}"_"${i}"_rate.txt"
name="rate_"${i}".png"
gnuplot<<!
set xlabel "time/s" 
set ylabel "rate/kbps"
set xrange [0:200]
set yrange [0:5000]
set term "png"
set output "${name}"
plot "${file1}" u 1:2 title "ecld" with lines lw 2,\
"${file2}" u 1:2 title "wrr" with lines lw 2,\
"${file3}" u 1:2 title "bc" with lines lw 2,\
"${file4}" u 1:2 title "sfl" with lines lw 2
set output
exit
!

i=5
file1=${method1}"_"${i}"_rate.txt"
file2=${method2}"_"${i}"_rate.txt"
file3=${method3}"_"${i}"_rate.txt"
file4=${method4}"_"${i}"_rate.txt"
name="rate_"${i}".png"
gnuplot<<!
set xlabel "time/s" 
set ylabel "rate/kbps"
set xrange [0:200]
set yrange [0:3000]
set term "png"
set output "${name}"
plot "${file1}" u 1:2 title "ecld" with lines lw 2,\
"${file2}" u 1:2 title "wrr" with lines lw 2,\
"${file3}" u 1:2 title "bc" with lines lw 2,\
"${file4}" u 1:2 title "sfl" with lines lw 2
set output
exit
!

i=6
file1=${method1}"_"${i}"_rate.txt"
file2=${method2}"_"${i}"_rate.txt"
file3=${method3}"_"${i}"_rate.txt"
file4=${method4}"_"${i}"_rate.txt"
name="rate_"${i}".png"
gnuplot<<!
set xlabel "time/s" 
set ylabel "rate/kbps"
set xrange [0:200]
set yrange [0:7000]
set term "png"
set output "${name}"
plot "${file1}" u 1:2 title "ecld" with lines lw 2,\
"${file2}" u 1:2 title "wrr" with lines lw 2,\
"${file3}" u 1:2 title "bc" with lines lw 2,\
"${file4}" u 1:2 title "sfl" with lines lw 2
set output
exit
!

i=7
file1=${method1}"_"${i}"_rate.txt"
file2=${method2}"_"${i}"_rate.txt"
file3=${method3}"_"${i}"_rate.txt"
file4=${method4}"_"${i}"_rate.txt"
name="rate_"${i}".png"
gnuplot<<!
set xlabel "time/s" 
set ylabel "rate/kbps"
set xrange [0:200]
set yrange [0:6000]
set term "png"
set output "${name}"
plot "${file1}" u 1:2 title "ecld" with lines lw 2,\
"${file2}" u 1:2 title "wrr" with lines lw 2,\
"${file3}" u 1:2 title "bc" with lines lw 2,\
"${file4}" u 1:2 title "sfl" with lines lw 2
set output
exit
!

i=8
file1=${method1}"_"${i}"_rate.txt"
file2=${method2}"_"${i}"_rate.txt"
file3=${method3}"_"${i}"_rate.txt"
file4=${method4}"_"${i}"_rate.txt"
name="rate_"${i}".png"
gnuplot<<!
set xlabel "time/s" 
set ylabel "rate/kbps"
set xrange [0:200]
set yrange [0:7000]
set term "png"
set output "${name}"
plot "${file1}" u 1:2 title "ecld" with lines lw 2,\
"${file2}" u 1:2 title "wrr" with lines lw 2,\
"${file3}" u 1:2 title "bc" with lines lw 2,\
"${file4}" u 1:2 title "sfl" with lines lw 2
set output
exit
!

i=9
file1=${method1}"_"${i}"_rate.txt"
file2=${method2}"_"${i}"_rate.txt"
file3=${method3}"_"${i}"_rate.txt"
file4=${method4}"_"${i}"_rate.txt"
name="rate_"${i}".png"
gnuplot<<!
set xlabel "time/s" 
set ylabel "rate/kbps"
set xrange [0:200]
set yrange [0:6000]
set term "png"
set output "${name}"
plot "${file1}" u 1:2 title "ecld" with lines lw 2,\
"${file2}" u 1:2 title "wrr" with lines lw 2,\
"${file3}" u 1:2 title "bc" with lines lw 2,\
"${file4}" u 1:2 title "sfl" with lines lw 2
set output
exit
!
