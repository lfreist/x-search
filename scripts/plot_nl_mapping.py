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
        "searching byte offsets": {},
        "mapping byte offsets to line indices": {},
        "meta file size": {},
        "meta p data": {}
    }
    for d in data:
        result["searching byte offsets"][d["nl mapping"]] = [d["Wall"]["searching byte offsets"]["0"]["time"] / 1000000]
        result["mapping byte offsets to line indices"][d["nl mapping"]] = \
            [d["Wall"]["mapping byte offsets to line indices"]["0"]["time"] / 1000000]
        result["meta file size"][d["nl mapping"]] = [d["meta file size"] / 1000000]
        result["meta p data"][d["nl mapping"]] = [d["meta file size"] / d["original size"]]
    return result


def merge_data(data: dict, result: dict) -> dict:
    if len(result.keys()) == 0:
        result = data.copy()
        return result
    for k, v in data.items():
        for _k, _v in v.items():
            result[k][_k] += _v
    return result


def add_subplot(axs, title: str, data: dict, y_label: str) -> None:
    items = list(data.items())
    items.sort(key=lambda v: v[0])
    x = []
    y = []
    y_err = []
    for i in items:
        x.append(str(i[0]))
        y.append(statistics.mean(i[1]))
        if len(i[1]) > 1:
            y_err.append(statistics.stdev(i[1]))
    axs.bar(x, y)
    if len(y_err) and sum(y_err) != 0:
        axs.errorbar(x, y, yerr=y_err, fmt='o', color='r')
    axs.set_ylabel(y_label)
    axs.set_xlabel("Newline Mapping Data Byte Distance")
    axs.set_title(title)


def plot(data: dict):
    fig, axs = plt.subplots(2, 2, figsize=(10, 5))
    add_subplot(axs[0, 0], "searching byte offsets", data["searching byte offsets"], "time [ms]")
    add_subplot(axs[0, 1], "line mapping", data["mapping byte offsets to line indices"], "time [ms]")
    add_subplot(axs[1, 0], "Metafile size", data["meta file size"], "MB")
    add_subplot(axs[1, 1], "Metafile size / Data size", data["meta p data"], "Metafile Size / Data Size")
    plt.tight_layout()


def parse_command_line_arguments():
    parser = argparse.ArgumentParser(prog="plot_file_size.py",
                                     description="Plot results created by running 'benchmark_file_size.sh'.")
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
