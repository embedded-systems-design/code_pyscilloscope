#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Aug 12 20:13:53 2020

@author: danaukes
"""


from pyscilloscope.server_side_event_scope import ServerSideEventScope
import PyQt5.Qt as qt

if __name__=='__main__':
    
    pscope = ServerSideEventScope('https://api.particle.io/v1/devices/events?access_token=d335bc89d666834185edb810cd21a9ded2627613')
    pscope.run()
    qt.QApplication.exec_()