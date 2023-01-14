"""
Copyright 2022, Leon Freist
Author: Leon Freist <freist.leon@informatik.uni-freiburg.de>

This file is part of StringFinder <https://github.com/lfreist/stringFinder>
"""

import os
import argparse
import random
from random import randint

PROGRESS = False
WORDS = []
FILE_SIZE = 0
OUTPUT_FILE = ""


def parse_command_line_arguments():
    parser = argparse.ArgumentParser(prog="createTestFile.py",
                                     description="Create a test file of a given size for StringFinder.")
    parser.add_argument("words_file", metavar="WORDS_FILE", type=str, help="File which holds possible words")
    parser.add_argument("--size", "-s", metavar="GiB", type=float, default="5", help="Size of the test file in GiB")
    parser.add_argument("--output", "-o", metavar="OUTPUT_FILE", type=str, default="sf-test.txt",
                        help="Path to output file")
    parser.add_argument("--progress", "-p", default=0, action="count", help="Display progress")
    parser.add_argument("--keyword", default="", type=str,
                        help="special keyword that is placed in file according to --density")
    parser.add_argument("--density", default=-1, type=int,
                        help="determines how often a keyword appears: -1: random, 0: never, x: every x-th line")
    return parser.parse_args()


def display_progress(done: int, of: int):
    if done % 100 != 0 or not PROGRESS:
        return
    progress = done / of
    bars = "|" * int(progress * 80)
    spaces = " " * (80 - len(bars))
    print(f"[{bars}{spaces}] - {round(progress * 100, 2)}%", end="\r")


def create_test_file():
    bytes_wrote = 0
    line_counter = 0
    with open(OUTPUT_FILE, "w") as f:
        while bytes_wrote < FILE_SIZE:
            display_progress(bytes_wrote, FILE_SIZE)
            min_line_length = randint(5, 50) * 10
            line = ""
            while min_line_length > 0:
                word = WORDS[randint(0, len(WORDS) - 1)]
                if KEYWORD and KEYWORD in word:
                    continue
                line += ' ' + word
                min_line_length -= len(word)
            line_counter += 1
            if line_counter == DENSITY:
                line_list = line.split(" ")
                insert_position = random.randint(0, len(line_list))
                line = " ".join(line_list[:insert_position] + [KEYWORD] + line_list[insert_position:])
                line_counter = 0
            if (bytes_wrote + len(line)) > FILE_SIZE:
                line = line[:int(FILE_SIZE)-bytes_wrote-1]
            line += '\n'
            bytes_wrote += f.write(line)
    display_progress(100, 100)
    print()


if __name__ == "__main__":
    args = parse_command_line_arguments()
    PROGRESS = bool(args.progress)
    FILE_SIZE = args.size * 1000000000
    OUTPUT_FILE = args.output
    KEYWORD = args.keyword
    DENSITY = args.density
    if DENSITY >= 0 and not KEYWORD:
        print("You must provide a keyword if density is set.")
        DENSITY = -1
    with open(args.words_file, "r") as f:
        WORDS = f.readlines()
        WORDS = list(map(lambda word: word.strip(), WORDS))
    create_test_file()
