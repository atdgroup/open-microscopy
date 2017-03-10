import microscope
 
class RegionScan:

  SAVE = 1
  SAVE_DISPLAY = 2
  DISPLAY_ONLY = 3

  def __init__(self):
    self._impl = microscope.RegionScanImpl()
    
  def Start(self, action, dir, prefix, ext):
    microscope.RegionScanStart(self._impl, action, dir, prefix, ext)

  def Stop(self):
    microscope.RegionScanStop(self._impl)

  def Hide(self):
    microscope.RegionScanHide(self._impl)

  def SetRoi(self, left, top, width, height):
    microscope.RegionScanSetRoi(self._impl, left, top, width, height)

  def SetRelativeRoi(self, width, height):
    """Set the region of interest in microns relative from the current stage position"""
    x, y, z = microscope.GetStagePosition()
    half_width = width / 2
    half_height = height / 2
    self.SetRoi(x - half_width, y - half_height, width, height)

  def SetRelativeRoiFromTimelapse(self, r):
    """Set the region of interest in microns relative from the current stage position from a width, heigth tuple"""
    x, y, z = microscope.GetStagePosition()
    half_width = r[0] / 2
    half_height = r[1] / 2
    self.SetRoi(x - half_width, y - half_height, r[0], r[1])

  def SetFocalPlaneOptions(self, options):
    """Set the focal plane options, usually from a timelapse region"""
    microscope.RegionScanSetFocalPlaneOptions(self._impl, options[0], options[1], options[2], options[3])

  def SetFocalPlaneOffset(self, offset):
    """Set a focal plane offset to the existing focal plane, usually from a timelapse region"""
    microscope.RegionScanSetFocalPlaneOffset(self._impl, offset)

  def SetFocalPlaneOffsetFromXYZ(self, x, y, z):
    """Sets a focal plane offset for a region scan from a new x,y,z corrdinate, usually from a timelapse region"""
    microscope.RegionScanSetFocalPlaneOffsetFromXYZ(self._impl, x, y, z)

  def SetSoftwareAutofocus(self, val):
    """Turn on or off the software autofocus on every point in the scan"""
    microscope.RegionScanSetPerformSwAutofocus(self._impl, val)
    
    
class CellFinder:

  def __init__(self):
    self._impl = microscope.CellFindingImpl()
    
    
    
class Spc:

  SPC_LIMIT_TYPE_SECONDS=1,
  SPC_LIMIT_TYPE_FRAMES=2,
  SPC_LIMIT_TYPE_MAX_COUNT=3,
  SPC_LIMIT_TYPE_MEAN_COUNT=4,

  def __init__(self):
    pass
    #self._impl = microscope.SpcImpl()
    
  def Start(self, limit_type = 2, repeat = 0, repeat_time = 10.0, should_display = 1, display_time = 2.0, accumulate = 0, autosave = 0, filepath = None):
    microscope.SpcStartAdvanced(limit_type, repeat, repeat_time, should_display, display_time, accumulate, autosave, filepath)
    
  def Stop(self):
    microscope.SpcStop()
    
  def ClearBoardMemory(self):
    microscope.SpcClearBoardMemory()
    
  def AcquireAndSaveToFileUsingUIValues(self, save_3d, filepath):
    microscope.SpcAcquireAndSaveUsingInterfaceValues(save_3d, filepath)
    
    