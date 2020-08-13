# -*- coding: utf-8 -*-
'''
Written by Daniel M. Aukes and CONTRIBUTORS
Email: danaukes<at>asu.edu.
Please see LICENSE for full license.
'''

from setuptools import setup
import sys
import shutil

shutil.rmtree("build", ignore_errors=True)
shutil.rmtree("dist", ignore_errors=True)
shutil.rmtree('git_manage.egg-info', ignore_errors=True)

packages = []
packages.append('pyscilloscope')

package_data = {}
package_data['pyscilloscope'] = []

setup_kwargs = {}
setup_kwargs['name']='pyscilloscope'
setup_kwargs['version']='0.0.1'
setup_kwargs['classifiers']=['Programming Language :: Python','Programming Language :: Python :: 3']   
setup_kwargs['description']='Pyscilloscope is a simple, serial-based data viewing GUI'
setup_kwargs['author']='Dan Aukes'
setup_kwargs['author_email']='danaukes@danaukes.com'
setup_kwargs['url']='https://github.com/danaukes/code_pyscilloscope'
setup_kwargs['license']='MIT'
setup_kwargs['packages']=packages
setup_kwargs['package_dir']={'pyscilloscope' : 'python/pyscilloscope'}
setup_kwargs['package_data'] = package_data
setup_kwargs['install_requires']=[]
  
setup(**setup_kwargs)
