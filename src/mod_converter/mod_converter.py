#!/usr/bin/python3
# -*- coding: utf-8 -*-

import argparse
import os
import subprocess

banned_ext = [
    '.obj',
    '.mtl',
    '.txt',
    '.tm',
    '.TM',
    '.tga',
]

def main():    
    parser = argparse.ArgumentParser(description='Batch models converter')
    parser.add_argument('--dir', dest='dir', help='path to directory with models')
    pargs = parser.parse_args()

    if pargs.dir:
        run(pargs.dir)

def run(dir):
    for file in sorted(os.listdir(dir)):
        if os.path.isdir(file) or os.path.splitext(file)[1] in banned_ext:
            continue
        p = subprocess.Popen(['mod_converter.exe', '-m', dir + '/' + file])
        p.communicate()

if __name__ == '__main__':
    main()
