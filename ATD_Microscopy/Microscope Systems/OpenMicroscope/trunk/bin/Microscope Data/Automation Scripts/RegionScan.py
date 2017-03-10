import microscope, MicroscopeModules

data_dir = microscope.GetUserDataDirectory()
output = microscope.ShowSimpleFileSequenceSaveDialog(data_dir)
        
directory = output[0]
filename = output[1]
filename_ext = output[2]

print "Saving to directory ", directory
print "Filename ", filename
print "Filename Extension ", filename_ext

action = MicroscopeModules.RegionScan.SAVE_DISPLAY

rs = MicroscopeModules.RegionScan()

rs.SetRelativeRoi(5000, 5000)
rs.Start(action,directory,filename, filename_ext)