import microscope
import time

try:

    print "Turning Lamp On"
    microscope.TurnLampOn()
    time.sleep(3)
    print "Turning Lamp Off"
    microscope.TurnLampOff()
    print "Turning Lamp Intensity to 0"
    microscope.SetLampIntensity(0.0)
    time.sleep(3)
    print "Turning Lamp On"
    microscope.TurnLampOn()
    print "Turning Lamp Intensity to 100"
    microscope.SetLampIntensity(100.0)
    
except:
    print "Could not control lamp ?"