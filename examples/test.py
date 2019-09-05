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
    print data[2]
    if data.skeleton_num:
        print data.skeletons[0].head.real

def handsCallback(data):
    # print "s"
    print data

def userCallback(data):
    cv2.imshow('User', data*2**8 + 2**8)
    cv2.waitKey(1)

def gestureCallback(data):
    print data

a = Nuitrack()
# print a.test()

a.set_depth_callback(depthCallback)
a.set_color_callback(colorCallback)
a.set_skeleton_callback(skelCallback)
# a.set_face_callback(skelCallback)
# a.set_hands_callback(handsCallback)
# a.set_user_callback(userCallback)
# a.set_gesture_callback(gestureCallback)
# a.set_issue_callback(gestureCallback)

a.init()

# a.update()

frames = 100
while frames:
    # print "-"
    a.update()
    # print "u"
    frames -= 1

a.release()

print "the end"

cv2.destroyAllWindows()

print "done"