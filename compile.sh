#!/usr/bin/env bash

gcc src/*.c src/*/*.c src/*/*/*.c -static -Wall -O3 $(pkg-config --cflags --libs cursed-tea-ui) -o forum
