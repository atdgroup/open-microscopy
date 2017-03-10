import sys
from Camera import Camera

#import numpy as np
#import matplotlib.pyplot as plt
#import matplotlib.mlab as mlab

#from pylab import imread, hist, randn

camera = Camera()
#camera.SnapImage()
fib = camera.GetImage()
fib.setRainBowPalette()
#pil = fib.convertToPil()

#fib.save("C:\\testxxxxx.png")
#x = imread("C:\\testxxxxx.png")

camera.DisplayBasicWindow("Test", 50, 50, 400, 400, fib)

del fib

#mu, sigma = 100, 15
#x = mu + sigma * np.random.randn(10000)

#fig = plt.figure()
#ax = fig.add_subplot(111)

# the histogram of the data
#n, bins, patches = ax.hist(x, 50, normed=1, facecolor='green', alpha=0.75)

#ax.set_xlabel('Smarts')
#ax.set_ylabel('Probability')
#ax.set_xlim(40, 160)
#ax.set_ylim(0, 0.03)
#ax.grid(True)

#plt.show()
