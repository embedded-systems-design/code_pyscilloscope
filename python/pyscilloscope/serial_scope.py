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


pen0 = pg.mkPen(color=(255,0,0))
pen1 = pg.mkPen(color=(0,255,0))
pen2 = pg.mkPen(color=(0,0,255))
pen3 = pg.mkPen(color=(255,255,0))
pen4 = pg.mkPen(color=(0,255,255))
pen5 = pg.mkPen(color=(255,0,255))
pens = [pen0,pen1,pen2,pen3,pen4,pen5]
symbols = [None,'o','s','t','d','+']

class SerialScope(object):

    def __init__(self,comport,baudrate=9600,window_width = 1000,buffer_width = 1000):
    
        self.window_width = window_width
        self.ptr = -window_width
        self.buffer_width = buffer_width

        self.string_stream = ''
        self.ser = serial.Serial(comport,baudrate)

        self.app = qt.QApplication([])
        self.win = pg.GraphicsWindow(title="Pyscilloscope")
        p = self.win.addPlot(title="Time vs. Voltage")
        
        self.packet = self.read_new_packet()
        self.width = len(self.packet)
        self.curves = []
        self.data_history = numpy.zeros((self.window_width,self.width))
        self.l = p.addLegend()
        for ii in range(self.width-2):
            curve = p.plot(pen=pens[ii%len(pens)],symbol=None)
            self.l.addItem(curve,'V'+str(ii))
            self.curves.append(curve)
        
        self.Xm = numpy.linspace(0,0,window_width)
        self.buffer_width = buffer_width

    def uart_to_lines(self):
        byte_stream = self.ser.read(self.buffer_width)
        string_stream = self.string_stream + byte_stream.decode()
        lines = string_stream.split('\r\n')
        self.string_stream = lines[-1]
        lines = lines[:-1]
        return lines

    def string_to_array(self,line):
        try:
            time,ain,a0,a1,a2 = line.split(',')
            time = float(time)
            ain = float(ain)
            aout = float (a0)
            aout = aout/4095*3.3
            return aout
        except ValueError as e:
            print(e)
    
    def strings_to_array(self,lines):
        aout_list = []
        for line in lines:
            aout = self.string_to_array(line)
            if aout is not None:
                aout_list.append(aout)
        return aout_list

    def lines_to_data(self,lines):
        lines = [line.split(',') for line in lines]
        lines = [line for line in lines if len(line)==self.width]
        values = numpy.array(lines,dtype=numpy.float)
        # x = values[:,0]
        # ain = values[:,1]
        # aout = values[:,2:]
        return values

    def read_new_packet(self):
        lines = self.uart_to_lines()
        if len(lines)>0:
            line = lines[-1]
            line_s = line.split(',')
            values = numpy.array(line_s,dtype=numpy.float)
            return values

    def update(self):
        lines = self.uart_to_lines()
        
        l = len(lines)
        if l>0:
            aout_list = self.strings_to_array(lines)

            self.Xm[:-l] = self.Xm[l:]
            self.Xm[-l:] = aout_list                 # vector containing the instantaneous values      
            self.ptr += l                              # update x position for displaying the curve
            self.curves[0].setData(self.Xm)                     # set the curve with this data
            self.curves[0].setPos(self.ptr,0)                   # set x position in the graph to 0
            qt.QApplication.processEvents()    # you MUST process the plot now

    def update2(self):
        lines = self.uart_to_lines()
        
        l = len(lines)
        if l>0:
            values = self.lines_to_data(lines)
            self.data_history[:-l] = self.data_history[l:]
            self.data_history[-l:]=values
            for ii in range(values.shape[1]-2):
                

                self.curves[ii].setData(self.data_history[:,0],self.data_history[:,ii+2])
            qt.QApplication.processEvents()    # you MUST process the plot now
        
    def run(self):
        while True: 
            self.update2()
        

if __name__=='__main__':
    
    pscope = SerialScope('/dev/ttyACM0',115200,window_width=10000)
    pscope.run()
    
    qt.QApplication.exec_() # you MUST put this at the end
