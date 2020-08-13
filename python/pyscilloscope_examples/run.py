#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Aug 12 20:13:53 2020

@author: danaukes
"""


from pyscilloscope.pyscilloscope import PySilloscope
import PyQt5.Qt as qt

if __name__=='__main__':
    
    pscope = PySilloscope('/dev/ttyACM0',115200,window_width=1000)
    while True: pscope.update()
    qt.QApplication.exec_()