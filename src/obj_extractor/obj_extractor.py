#!/usr/bin/python3
# -*- coding: utf-8 -*-

import argparse
import os
import subprocess

def main():    
    parser = argparse.ArgumentParser(description='Batch models converter')
    parser.add_argument('--dir', dest='dir', help='path to directory with maps')
    parser.add_argument('--db', dest='db', help='path to db')
    pargs = parser.parse_args()

    if pargs.dir:
        run(pargs.dir, pargs.db)

def run(dir, db):
    for file in sorted(os.listdir(dir)):
        if os.path.isdir(file) or os.path.splitext(file)[1] != ".mmo":
            continue
        print('loading: ' + file)
        p = subprocess.Popen(['obj_extractor.exe', db, dir + '/' + file])
        p.communicate()

if __name__ == '__main__':
    main()
