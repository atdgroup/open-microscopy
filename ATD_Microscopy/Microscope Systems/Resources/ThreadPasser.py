# -*- coding: utf-8 -*-
"""
Created on Wed Apr 07 09:55:52 2010

@author: gpierce

This formats the thread lock output from the microscope code.
When that is run with the VerboseDebug build it outputs the txt file to C:\\thread.dat
"""
import sys

class ThreadDetails:
    thread_id = 0
    thread_lock_id = 0
    thread_name = ""
    start_lock_time = 0.0
    total_lock_time = 0.0
    number_of_locks = 0
    total_get_release_locks = 0
    
locks = {}

def PrintThreadDetails(details):
    out = "%s (%s) Lock Count %d Average lock Time: %f\n" % (details.thread_name, details.thread_lock_id, details.number_of_locks, float(details.total_lock_time) )
    sys.stdout.write(out)

file = open("C:\\thread.dat")

for line in file:
    
    details = ThreadDetails()
    
    parts = line.split(",")
    name = parts[0]
    details.thread_lock_id = parts[1]
    details.thread_id = parts[2]
    
    if name == "NewLock":
        
        details.thread_name = parts[3][:-1]
        locks[details.thread_lock_id] = details
    elif name == "GetLock":
        details = locks[details.thread_lock_id]
        details.number_of_locks = details.number_of_locks + 1
        details.start_lock_time = float(parts[3])
        locks[details.thread_lock_id] = details
    elif name == "ReleaseLock":
        details = locks[details.thread_lock_id]
        details.number_of_locks = details.number_of_locks - 1
        details.total_get_release_locks = details.total_get_release_locks + 1
        details.total_lock_time = float(parts[3]) - details.start_lock_time
        locks[details.thread_lock_id] = details
 
for lock_key, lock_value in locks.items():
    PrintThreadDetails(lock_value)