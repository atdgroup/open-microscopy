import os, sys

print sys.path


print "CWD: ",os.getcwd()
print "Script dir: ", os.path.dirname( os.path.realpath( __file__ ) )


#import ShutterTest.py

for i in range(10):
	ShutterTest()