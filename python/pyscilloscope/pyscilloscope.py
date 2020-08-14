#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Aug  4 08:19:14 2020

@author: danaukes
"""


# Import libraries
import numpy
import PyQt5.Qt as qt
import PyQt5.QtGui as pg
import PyQt5.QtCore as pc
# import PyQt5.QtApp as qa
import PyQt5.QtWidgets as pw
# from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph as pg
import serial

class PySilloscope(object):

    def __init__(self,comport,baudrate=9600,window_width = 100,buffer_width = 1000,scaling = None):
    
        self.ser = serial.Serial(comport,baudrate)
        self.app = qt.QApplication([])

        self.win = pg.GraphicsWindow(title="Pyscilloscope")
        p = self.win.addPlot(title="Time vs. Voltage")
        self.curve = p.plot()
        
        self.Xm = numpy.linspace(0,0,window_width)
        # self.y = numpy.zeros((3,window_width))
        self.ptr = -window_width
        self.buffer_width = buffer_width

# Realtime data plot. Each time this function is called, the data display is updated

        self.string_stream = ''
    def update(self):
        aout_list = self.readlines()
        l = len(aout_list)
        self.Xm[:-l] = self.Xm[l:]
        # self.y[:-l] = self.y[l:]
        self.Xm[-l:] = aout_list                 # vector containing the instantaneous values      
        # self.y[-l:] = aout_list                 # vector containing the instantaneous values      
        self.ptr += l                              # update x position for displaying the curve
        self.curve.setData(self.Xm)                     # set the curve with this data
        self.curve.setPos(self.ptr,0)                   # set x position in the graph to 0
        qt.QApplication.processEvents()    # you MUST process the plot now
        
    def parse_line(self,line):
        row = line.split(',')
        row = numpy.array(row,dtype=numpy.float)
        return row

    def readlines(self):
        byte_stream = self.ser.read(self.buffer_width)
        self.string_stream += byte_stream.decode()
        lines = self.string_stream.split('\r\n')
        self.string_stream = lines[-1]
        lines = lines[:-1]
        # l = len(lines)
        aout_list = []
        for line in lines:
            # print(line)
            try:
                time,ain,a0,a1,a2 = line.split(',')
                time = float(time)
                ain = float(ain)
                aout = float (a1)
                aout = aout/4095*3.3
                aout_list.append(aout)
            except ValueError as e:
                print(e)
        return aout_list

if __name__=='__main__':
    
    ### MAIN PROGRAM #####    
    # this is a brutal infinite loop calling your realtime data plot
    pscope = PySilloscope('/dev/ttyACM1',115200,window_width=1000)
    while True: pscope.update()
    
    ### END QtApp ####
    qt.QApplication.exec_() # you MUST put this at the end
    ##################