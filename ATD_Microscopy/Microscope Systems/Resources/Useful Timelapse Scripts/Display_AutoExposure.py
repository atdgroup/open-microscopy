# Auto exposure and display, all cubes

import microscope, time

cubes = []

# OnStart is called once when the user presses start to begin the timelapse.
def OnStart():
    global cubes
    
    print "Stage Position, ", microscope.GetStagePosition()
    print microscope.GetTimeLapsePoints()
    
    try:
        # Try to autoexpose within the range of 1 and 60 milli seconds
        # Performing AutoExposure
        print "Performing autoexposure"
        microscope.PerformAutoExposure(1, 60)
    except:
        print "Could not calculate autoexposure. Is there enough light ?"

    try:
        print "Retrieving Cubes"
        cubes = microscope.GetCubes()
    except:
        print "Could not get cubes"    
        
def OnAbort():
    print "OnAbort()"
        
# Call once for each time all the points have been traversed.
def OnCycleStart():
    print "Cycle Changed"
    
# Called each time the stage moves to a new point.
def OnNewPoint(x, y, z, position):
    global cubes
    
    for cube in cubes:
        print "Snapping Image ", (x, y)
        pos = cube['Position']
        print "Moving cube to position: ", pos, "\n"
        microscope.MoveCubeToPosition(pos)
        time.sleep(3)
        microscope.SnapImage()