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
    for root, subdirs, files in os.walk(dir):
        for file in files:
            p = subprocess.Popen(['save_loader.exe', os.path.join(root, file)])
            p.communicate()

if __name__ == '__main__':
    main()
