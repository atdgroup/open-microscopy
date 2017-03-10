import microscope, MicroscopeModules

# Specify the whether the region scan saves images or just displays them
action = MicroscopeModules.RegionScan.DISPLAY_ONLY



def RegionScanTest(x_size, y_size):

    rs = MicroscopeModules.RegionScan()
    
    try:
        print "Running RegionScanTest"
        rs.SetRelativeRoi(x_size, y_size)
        rs.Start(action, "C:", "", "")
        
    except Exception, e:
        print e
        print "hello"
        print "Could not run regionscan", e
        