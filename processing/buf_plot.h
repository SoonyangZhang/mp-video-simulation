#! /bin/sh
file1=scale_buf.txt
file2=wrr_buf.txt
file3=bc_buf.txt
file4=edcld_buf.txt
file5=sfl_buf.txt
gnuplot<<!
set xlabel "time" 
set ylabel "recvbuf/bytes"
set xrange [0:200]
set yrange [0:25000]
set term "pdf"
set output "recv_buf.pdf"
plot "${file1}" u 1:2 title "scale" with points ,\
"${file2}" u 1:2 title "wrr" with points ,\
"${file3}" u 1:2 title "mc" with points,\
"${file4}" u 1:2 title "edcld" with points,\
"${file5}" u 1:2 title "sfl" with points
set output
exit
!
