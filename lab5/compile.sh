#!/bin/bash
#Вначале нужно установить libfuse-dev
# sudo apt install libfuse-dev
gcc lab5.c -o lab5 -lfuse -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26