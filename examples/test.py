#!/usr/bin/env python

import sys
sys.path.insert(1, '../build')

from pynuitrack import Nuitrack
from time import sleep
import numpy as np
import cv2

def b():
    print "hello world"

def depthCallback(data):
    cv2.imshow('Depth', data * 32)
    cv2.waitKey(1)

def colorCallback(data):
    cv2.imshow('Color', data)
    cv2.waitKey(1)





a = Nuitrack()
# a.apply(b)
a.set_depth_callback(depthCallback)
a.set_color_callback(colorCallback)
a.init()
# sleep(5)

frames = 150
while frames:
    a.update()
    frames -= 1
a.release()

# plt.ioff()

# b = a.test()
# print type(b)
# print b
