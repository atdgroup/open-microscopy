from microscope import PerformAutoExposure 

try:
    # Try to autoexpose within the range of 1 and 60 milli seconds
    PerformAutoExposure(1, 60)
except:
    print "Could not calculate autoexposure. Is there enough light ?"