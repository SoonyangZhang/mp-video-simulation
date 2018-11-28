#!/usr/bin/env python
import time
import os
import subprocess ,signal
schedule_type=["scale","wrr","edcld","bc","sfl","water"]
prefix_cmd="./waf --run 'scratch/ptr_test --st=%s'"

for i in range(len(schedule_type)):
    cmd=prefix_cmd%schedule_type[i]
    process= subprocess.Popen(cmd,shell = True)
    while 1:
        time.sleep(1)
        ret=subprocess.Popen.poll(process)
        if ret is None:
            continue
        else:
            break
        
        
print "stop"
