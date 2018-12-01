#!/usr/bin/env python
import time
import os
import subprocess ,signal
process=[]
schedule_type=["scale","wrr","edcld","bc","minq","sfl","water"]
test_case=9
for k in range(test_case):
    for i in range(len(schedule_type)):
        data_in="../"+schedule_type[i]+"_"+str(k+1)+"_buf.txt >"
        data_out=" avg_"+schedule_type[i]+"_"+str(k+1)+"_buf.txt"
        cmd="awk -f buf_get.awk  "+data_in+data_out 
        p= subprocess.Popen(cmd,shell = True)
        process.append(p)
    while 1:
        time.sleep(1)
        total=len(process)
        alive=total
        for i in range(total):
            ret=subprocess.Popen.poll(process[i])
            if ret is None:
                continue
            else:
                alive-=1
        if(alive==0):
            break
print "stop"
