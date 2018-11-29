#!/usr/bin/env python
import time
import os
import subprocess ,signal
schedule_type=["scale","wrr","edcld","bc","minq","sfl","water"]
prefix_cmd="./waf --run 'scratch/ptr_test --st=%s --cs=%s'"
test_case=9
for case in range(test_case):
    if(case==0):
        continue
    inst=str(case+1);
    for i in range(len(schedule_type)):
        cmd=prefix_cmd%(schedule_type[i],inst)
        process= subprocess.Popen(cmd,shell = True)
        while 1:
            time.sleep(1)
            ret=subprocess.Popen.poll(process)
            if ret is None:
                continue
            else:
                break
        
'''
cmd=prefix_cmd%schedule_type[2]
process= subprocess.Popen(cmd,shell = True)
while 1:
    time.sleep(1)
    ret=subprocess.Popen.poll(process)
    if ret is None:
        continue
    else:
        break 
'''  
print "stop"
