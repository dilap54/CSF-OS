#!/bin/bash
 gcc -Wall lab5.c `pkg-config fuse --cflags --libs` -o lab5