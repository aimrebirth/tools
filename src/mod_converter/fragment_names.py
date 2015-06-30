#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys

print(sorted(set(open(sys.argv[1]).readlines())))