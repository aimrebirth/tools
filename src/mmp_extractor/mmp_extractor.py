#!/usr/bin/python3
# -*- coding: utf-8 -*-

import argparse
import os
import subprocess

def main():    
    parser = argparse.ArgumentParser(description='Batch MMP extractor')
    parser.add_argument('--dir', dest='dir', help='path to directory with maps')
    pargs = parser.parse_args()

    if pargs.dir:
        run(pargs.dir)

def run(dir):
    for file in sorted(os.listdir(dir)):
        if os.path.isdir(file) or os.path.splitext(file)[1].lower() != ".mmp":
            continue
        print('processing: ' + file)
        p = subprocess.Popen(['mmp_extractor.exe', dir + '/' + file])
        p.communicate()

if __name__ == '__main__':
    main()
