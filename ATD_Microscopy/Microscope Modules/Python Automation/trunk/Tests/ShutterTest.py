import microscope

def ShutterTest():
	try:
		microscope.CloseShutter()
	   
		print "Setting Shutter Open time to 10 seconds"
		# This line blocks at this point so checking that the shutter is open after wards will always return false (0)
		microscope.SetShutterOpenTime(10000)

		if microscope.IsShutterOpen():
			print "Shutter is open"
		else:
			print "Shutter is closed"
			
	except:
		print "Could not control shutter"