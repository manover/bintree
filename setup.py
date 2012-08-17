#!/usr/bin/env python2.7
from distutils.core import setup, Extension

setup(name="avl", version="1.0",
      ext_modules=[Extension("avl", ["avl.c"])])
