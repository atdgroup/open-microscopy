import os, sys
import microscope

print microscope.PYTHON_MICROSCOPE_AUTOMATION_ROOT

sys.path.append(microscope.PYTHON_MICROSCOPE_AUTOMATION_ROOT + os.sep + "Tests")

print sys.path

from ShutterTest import ShutterTest


for i in range(10):
	ShutterTest()