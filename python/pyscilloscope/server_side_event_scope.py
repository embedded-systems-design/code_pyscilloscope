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
# import serial

from sseclient import SSEClient 

pen0 = pg.mkPen(color=(255,0,0))
pen1 = pg.mkPen(color=(0,255,0))
pen2 = pg.mkPen(color=(0,0,255))
pen3 = pg.mkPen(color=(255,255,0))
pen4 = pg.mkPen(color=(0,255,255))
pen5 = pg.mkPen(color=(255,0,255))
pens = [pen0,pen1,pen2,pen3,pen4,pen5]
symbols = [None,'o','s','t','d','+']

class ServerSideEventScope(object):

    def __init__(self,url,window_width=300,data_width = 500,connect_history = False):
    
        self.app = qt.QApplication([])

        self.url = url
        self.window_width = window_width
        self.data_width = data_width
 
        self.win = pg.GraphicsWindow(title="Pyscilloscope")

        self.messages = SSEClient(self.url)

        # self.packet = self.read_new_packet()

        self.plot_init = False
        
        self.connect_history = connect_history
        
    def init_plot(self,lines):
        if not self.plot_init:
            self.width = len(lines[-1])
            p = self.win.addPlot(title="Time vs. Voltage")
            self.curves = []
            self.l = p.addLegend()
            self.data_history = numpy.zeros((self.window_width,self.width))
            
            for ii in range(self.width-1):
                curve = p.plot(pen=pens[ii%len(pens)],symbol=None)
                self.l.addItem(curve,'V'+str(ii))
            self.curves.append(curve)
            self.plot_init = True


    def strip_special(self,s):
        substring = 'data":"'
        ii = s.find(substring)
        if ii>0:
            s = s[(ii+len(substring)):]
    
        substring = '","'
        ii = s.find(substring)
        if ii>0:
            s = s[:ii]
        return s

    def read_new_packet(self):
        for msg in self.messages: 
            pass
        data_s = self.strip_special(msg.data)
        lines = data_s.split(';')
        if len(lines)>0:
            line = lines[-1]
            line_s = line.split(',')
            values = numpy.array(line_s,dtype=numpy.float)
            return values

    def lines_to_data(self,lines):
        # lines = [line.split(',') for line in lines]
        lines = [line for line in lines if len(line)==self.width]
        values = numpy.array(lines,dtype=numpy.float)
        return values

    def update(self,msg):
        self.msg = msg
        data_s = self.strip_special(self.msg.data)
        lines = data_s.split(';')
        lines = [line.split(',') for line in lines if line !='']
        if len(lines)>0:
            self.init_plot(lines)
            self.values = self.lines_to_data(lines)
            # self.values = [int(item) for item in data_s.split(',') if item != '']
            # self.values_a = numpy.array(self.values)
            l = len(self.values)
            if l>0:
                values = self.lines_to_data(lines)
                self.data_history[:-l] = self.data_history[l:]
                self.data_history[-l:]=values
                for ii in range(self.width-1):
                    if self.connect_history:
                        self.curves[ii].setData(self.data_history[:,0],self.data_history[:,ii+1])
                    else:
                        self.curves[ii].setData(values[:,0],values[:,ii+1])
    
                qt.QApplication.processEvents()    # you MUST process the plot now
            
    def run(self):
        for msg in self.messages: 
            self.update(msg)

if __name__=='__main__':
    
    pscope = ServerSideEventScope('https://api.particle.io/v1/devices/events?access_token=d335bc89d666834185edb810cd21a9ded2627613',connect_history=True,window_width=1000)
    pscope.run()
    
    qt.QApplication.exec_() # you MUST put this at the end
