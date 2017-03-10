from microscope import GetCubes 

try:
    cubes = GetCubes()
  
    for cube in cubes:
        print cube, "\n"

except:
    print "Could not get cubes"