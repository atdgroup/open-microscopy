Standalone project for the Stage Scan module (using any microscope with a Corvus XY stage controller). 
Performs XY meander pattern, (using Venus macr), while outputting trigger pulse at line start and end.
This project contains all the modules of a manual miscrosope, hence:
This project requires:-

from the C Libraries repository:
com_port_control
FreeImage.h
freeimage_imaq
FreeImageAlgorithms_...
gci_menu_utils
GCI_utils
gci_utils_progress_feedback
string_utils
GL_CVIRegistry
image_utils
password
profile
status
signals
xml_utils

from the Hardware Control repository:
stage		(Corvus)
focus drive	(dummy)
analyzer	(generic only)
microscope 	(manual - not 90i files)
objectives	(dummy)
fluor Cubes	(dummy)
Optical Path	(generic only)
Condenser	(dummy)
Lamp		(dummy)
Optical Shutter	(dummy)
Aperture Stop	(generic only)
Field Stop	(generic only)
Epi-field Stop	(generic only)
Optical Zoom	(generic only)
Camera

All from IcsViewer repository:

All from Imaging repository:

All from RegionOfInterest repository:

All from Mosaic repository:

All from StageScan repository:

All from RegionScan repository:

All from Timelapse repository:

All from GeneralPurposeMicroscope repository:

All from Zstacks repository:

All from Alignment repository:

from GCI_Libraries:
dcamapi.lib, .dll
FreeImage.lib, .dll
FreeImageAlgorithms.lib, .dll
FreeImageIcs.lib, .dll
ics.lib, .dll
ImageViewer.dll
IMAQ_CVI.dll
StringUtils.lib
zlib1.dll

from cvi sdk:
ole32.lib 
Shell32.lib
Vfw32.lib