import sys

import FreeImagePy
import FreeImageAlgorithmsPy as FIA

F = FIA.FIAImage()  
F.load("C:\\test.jpg")

print F.getHistogram(0, 255, 255)

F.save("C:\\test_result.bmp")