#!/usr/bin/python3
# -*- coding: utf-8 -*-

import argparse
import os
import subprocess

def main():    
    parser = argparse.ArgumentParser(description='Batch models converter')
    parser.add_argument('--dir', dest='dir', help='path to directory with models')
    pargs = parser.parse_args()

    if pargs.dir:
        run(pargs.dir)

def run(dir):
    for file in os.listdir(dir):
        if os.path.isdir(file):
            continue
        p = subprocess.Popen(['mod_converter.exe', file])
        p.communicate()

if __name__ == '__main__':
    main()
