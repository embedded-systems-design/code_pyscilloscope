#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Aug 12 20:13:53 2020

@author: danaukes
"""


from pyscilloscope.serial_scope import SerialScope
import PyQt5.Qt as qt

if __name__=='__main__':
    
    pscope = SerialScope('COM1',115200,window_width=1000)
    pscope.run()
    qt.QApplication.exec_()