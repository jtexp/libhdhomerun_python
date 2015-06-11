#!/usr/bin/env python

import platform
from distutils.core import setup, Extension

source_files = ['silicondustmodule.c']

module = Extension(
    name = 'silicondust',
    sources = source_files,
    libraries = ['hdhomerun'],
    include_dirs = ['libhdhomerun'],
)

setup(
    name='silicondust',
    version='1.0',
    ext_modules=[module],
)
