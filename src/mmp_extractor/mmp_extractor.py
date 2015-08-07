#!/usr/bin/python3
# -*- coding: utf-8 -*-

import argparse
import os
import subprocess

def main():    
    parser = argparse.ArgumentParser(description='Batch MMP extractor')
    parser.add_argument('--dir', dest='dir', help='path to directory with maps')
    parser.add_argument('--tex', dest='tex', help='path to textures ids')
    pargs = parser.parse_args()

    if pargs.dir:
        run(pargs.dir, pargs.tex)

def run(dir, tex):
    for file in sorted(os.listdir(dir)):
        if os.path.isdir(file) or os.path.splitext(file)[1].lower() != ".mmp":
            continue
        print('processing: ' + file)
        p = subprocess.Popen(['mmp_extractor.exe', dir + '/' + file, tex])
        p.communicate()

if __name__ == '__main__':
    main()
