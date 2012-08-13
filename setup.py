#!/usr/bin/env python
from distutils.core import setup, Extension

setup(name="avl", version="1.0",
      ext_modules=[Extension("avl", ["avl.c"])])
