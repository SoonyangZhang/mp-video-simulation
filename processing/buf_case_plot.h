#! /bin/sh
file1=scale_buf.txt
file2=wrr_buf.txt
file3=bc_buf.txt
file4=edcld_buf.txt
file5=sfl_buf.txt
gnuplot<<!
set xlabel "time" 
set ylabel "recvbuf/bytes"
set xrange [0:10]
set yrange [0:25000]
set term "png"
set output "recv_buf.png"
plot "${file2}" u 1:2 title "wrr" with lines ,\
"${file3}" u 1:2 title "bc" with lines,\
"${file4}" u 1:2 title "edcld" with lines,\
"${file5}" u 1:2 title "sfl" with lines
set output
exit
!
#"${file1}" u 1:2 title "scale" with lines ,\
