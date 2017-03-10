import microscope
import time

try:

    print help(microscope.MicroscopeSetMode)

    print "Setting microscope to mode FLUORESCENCE_MODE"
    microscope.MicroscopeSetMode(FLUORESCENCE_MODE)

    time.sleep(2)
    
    print "Setting microscope to mode BRIGHT_FIELD_MODE"
    microscope.MicroscopeSetMode(BRIGHT_FIELD_MODE)
        
except:
    print "Could not change mode"