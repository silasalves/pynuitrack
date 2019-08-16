#!/usr/bin/env python

# import classes

# t = classes.World()
# t.set("bom dia!")
# print (t.greet())

# t.many(['Good Morning', 'Buon giorno', 'Kali mera'])
# print (t.greet())

import sys
sys.path.insert(1, '../build')

from pynuitrack import Nuitrack
from time import sleep

def b():
    print "hello world"

a = Nuitrack()
# a.apply(b)
a.init()
a.release()


