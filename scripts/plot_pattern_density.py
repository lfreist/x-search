"""
Copyright 2022, Leon Freist
Author: Leon Freist <freist.leon@informatik.uni-freiburg.de>

This file is part of StringFinder <https://github.com/lfreist/stringFinder>
"""

import argparse
import json
import os
import statistics

import matplotlib.pyplot as plt

from typing import List


def read_data(path: str) -> List[dict]:
    result: List[dict] = []
    for file in os.listdir(path):
        file_path = os.path.join(path, file)
        if os.path.isfile(file_path) and file.endswith(".json"):
            with open(file_path, "r") as f:
                data = json.load(f)
                result.append(data)
    return result


def transform_data(data: List[dict]) -> dict:
    result: dict = {
        "total time": {},
        "reading": {},
        "searching byte offsets": {},
        "mapping byte offsets to line indices": {},
        "mapping byte offset to line": {},
        "formatting and printing": {}
    }
    for d in data:
        for k, v in result.items():
            try:
                v[d["density"]] = [d["Wall"][k]["0"]["time"] / 1000000000]
            except KeyError:
                v[d["density"]] = [0]
    return result


def merge_data(data: dict, result: dict) -> dict:
    if len(result.keys()) == 0:
        result = data.copy()
        return result
    for k, v in data.items():
        for _k, _v in v.items():
            result[k][_k] += _v
    return result


def add_subplot(axs, title: str, data: dict) -> None:
    items = list(data.items())
    items.sort(key=lambda v: v[0])
    x = []
    y = []
    y_err = []
    for i in items:
        x.append(str(i[0]))
        y.append((sum(i[1]) / len(i[1])))
        if len(i[1]) > 1:
            y_err.append(statistics.stdev(i[1]))
    axs.bar(x, y)
    if y_err:
        axs.errorbar(x, y, yerr=y_err, fmt='-', color='r')
    axs.set_xticklabels(labels=x, rotation=90)
    axs.set_xlabel("pattern density (lines/pattern)")
    axs.set_ylabel("time [s]")
    axs.set_title(title)


def plot(data: dict):
    n_cols: int = len(data)
    n_rows: int = 1
    if len(data) < 4:
        pass
    elif len(data) < 9:
        n_cols = (len(data) // 2) + (len(data) % 2)
        n_rows = 2
    elif len(data) < 13:
        n_cols = (len(data) // 3) + 1 if (len(data) % 2) else 0
        n_rows = 3
    else:
        n_cols = (len(data) // 4) + 1 if (len(data) % 2) else 0
        n_rows = 4
    fig, axs = plt.subplots(n_rows, n_cols, figsize=(8, 10))
    col = 0
    row = 0
    for k, v in data.items():
        add_subplot(axs[row, col], k, v)
        if col < n_cols - 1:
            col += 1
        else:
            col = 0
            row += 1
    plt.tight_layout()


def parse_command_line_arguments():
    parser = argparse.ArgumentParser(prog="plot.py",
                                     description="Plot results created by running 'benchmark_pattern_<*>.sh'.")
    parser.add_argument("data_dir", metavar="DATA_DIR", type=str, help="Path to directory containing benchmarking data")
    parser.add_argument("--output", metavar="OUT_FILE", type=str, default="", help="Save figure as...")
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_command_line_arguments()
    PATH = args.data_dir
    ALL_DATA = {}
    for obj in os.listdir(PATH):
        full_path = os.path.join(PATH, obj)
        if os.path.isdir(full_path):
            DATA = read_data(full_path)
            ALL_DATA = merge_data(transform_data(DATA), ALL_DATA)
        else:
            if obj.endswith(".json"):
                DATA = read_data(PATH)
                ALL_DATA = merge_data(transform_data(DATA), ALL_DATA)
                break
    plot(ALL_DATA)
    if args.output:
        plt.savefig(args.output)
    else:
        plt.show()
