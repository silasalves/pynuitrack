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
    # print "d"
    cv2.imshow('Depth', data * 32)
    cv2.waitKey(1)

def colorCallback(data):
    # print "c"
    cv2.imshow('Color', data)
    cv2.waitKey(1)

def skelCallback(data):
    # print "s"
    print data

def handsCallback(data):
    # print "s"
    print data


a = Nuitrack()
# print a.test()

a.set_depth_callback(depthCallback)
a.set_color_callback(colorCallback)
a.set_skeleton_callback(skelCallback)
a.set_hands_callback(handsCallback)

a.init()

# a.update()

frames = 300
while frames:
    # print "-"
    a.update()
    # print "u"
    frames -= 1

a.release()