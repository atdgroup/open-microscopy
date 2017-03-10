import microscope

try:
    paths = microscope.GetOpticalPaths()
  
    for path in paths:
        print path, "\n"

    print "Moving to optical path position " , 2
    microscope.MoveOpticalPathToPosition(2)
        
except:
    print "Could not get control optical paths"