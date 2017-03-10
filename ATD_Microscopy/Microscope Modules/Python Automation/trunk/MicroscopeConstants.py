
class MicroscopeIlluminationMode:
    UNDEFINED = -1
    FLUORESCENCE = 0 
    BRIGHT_FIELD = 1
    LASER_SCANNING = 2
    PHASE_CONTRAST = 3
    FLUOR_NO_SHUTTER = 4 

class Scanner:
    RES_1024  = 2	
    RES_512   = 3	
    RES_256   = 4	
    RES_128   = 5	
    RES_64    = 6	
    RES_32    = 7	
    
    XYSCAN    = 0
    LINESCAN  = 1
    
    ZOOM_X1   = 255
    ZOOM_X2   = 127
    ZOOM_X5   = 51
    ZOOM_X10  = 25
    ZOOM_X20  = 13
    ZOOM_PARK = 0