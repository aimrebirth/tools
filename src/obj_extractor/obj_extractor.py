#!/usr/bin/python3
# -*- coding: utf-8 -*-

import argparse
import os
import subprocess

def main():    
    parser = argparse.ArgumentParser(description='Batch models converter')
    parser.add_argument('--dir', dest='dir', help='path to directory with maps')
    parser.add_argument('--db', dest='db', help='path to db')
    parser.add_argument('--prefix', dest='prefix', help='prefix')
    pargs = parser.parse_args()

    if not pargs.prefix:
        pargs.prefix = ''

    if pargs.dir:
        run(pargs.dir, pargs.db, pargs.prefix)

def run(dir, db, prefix):
    for file in sorted(os.listdir(dir)):
        if os.path.isdir(file) or os.path.splitext(file)[1].lower() != ".mmo":
            continue
        print('loading: ' + file)
        p = subprocess.Popen(['obj_extractor.exe', db, dir + '/' + file, prefix])
        p.communicate()

if __name__ == '__main__':
    main()
