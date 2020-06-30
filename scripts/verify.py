#!/usr/bin/env python3
import os

def dump(b):
    s = ""
    b_size = len(b)

    for i in range(b_size):
        if (i % 16) == 0:
            s += "\n{:0>8x} ".format(i)
        if (i % 8) == 0:
            s += " "

        s += "{:0>2x}".format(b[i])
        s += " "

    return s

has_ref = []
has_ch8 = []
targets = []

for dir_entry in os.scandir("./data"):
    filename, ext = os.path.splitext(dir_entry.path)
    if ext == ".ch8":
        has_ch8.append(filename);
    if ext == ".ref":
        has_ref.append(filename);

targets = sorted([x for x in has_ref if x in has_ch8])

for t in targets:
    print("checking {}".format(t))
    with open(t + ".ch8", "rb") as tch8, open(t + ".ref", "rb") as tref:
        ch8_data = tch8.read();
        ref_data = tref.read();

        if ch8_data == ref_data:
            print("[ PASS ]")
            continue

        print("[ FAIL ]")

        print("ch8", dump(ch8_data), "\n")
        print("ref", dump(ref_data), "\n")

        exit(1)
