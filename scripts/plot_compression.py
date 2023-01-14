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
                split = file.split("-")
                for i in range(len(split)):
                    if split[i] == "lvl":
                        data["lvl"] = int(split[i + 1])
                        break
                if "lz4" in file_path:
                    data["compression algorithm"] = "lz4"
                elif "zst" in file_path:
                    data["compression algorithm"] = "zstd"
                result.append(data)
    return result


def transform_data(data: List[dict]) -> dict:
    result: dict = {
        "lz4": {},
        "zstd": {}
    }
    for d in data:
        result[d["compression algorithm"]][d["lvl"]] = {
            "compressed size": [float(d["compressed size"]) / 1000000],
            "decompression": [d["Wall"]["decompression"]["0"]["time"]],
            "reading": [d["Wall"]["reading"]["0"]["time"]],
            "decompression and reading": [d["Wall"]["decompression"]["0"]["time"] + d["Wall"]["reading"]["0"]["time"]]
        }
    return result


def merge_data(data: dict, result: dict) -> dict:
    if len(result.keys()) == 0:
        result = data.copy()
        return result
    for k, v in data.items():
        for _k, _v in v.items():
            for __k, __v in _v.items():
                result[k][_k][__k] += __v
    return result


def add_subplot(axs, title: str, data: dict, key: str, y_label: str) -> None:
    items = [(int(k), v[key]) for k, v in data.items()]
    items.sort()
    x = []
    y = []
    y_err = []
    for i in items:
        x.append(str(i[0]))
        if key == "decompression" or key == "reading" or key == "decompression and reading":
            y.append((sum(i[1]) / len(i[1])) / 1000000)
            if len(i[1]) > 1:
                y_err.append((statistics.stdev(i[1])) / 1000000)
        else:
            y.append(sum(i[1]) / len(i[1]))
            if len(i[1]) > 1:
                y_err.append((statistics.stdev(i[1])))
    axs.bar(x, y)
    if y_err and sum(y_err) != 0:
        axs.errorbar(x, y, yerr=y_err, fmt='o', color='r')
    axs.set_ylabel(y_label)
    axs.set_title(title)


def plot(data: dict):
    fig, axs = plt.subplots(4, 2, sharey="row", figsize=(8, 10))
    add_subplot(axs[0, 0], "Reading (zstd)", data["zstd"], "reading", "time [ms]")
    add_subplot(axs[0, 1], "Reading (lz4)", data["lz4"], "reading", "")
    add_subplot(axs[1, 0], "Decompression (zstd)", data["zstd"], "decompression", "time [ms]")
    add_subplot(axs[1, 1], "Decompression (lz4)", data["lz4"], "decompression", "")
    add_subplot(axs[2, 0], "Decompression and Reading (zstd)", data["zstd"], "decompression and reading",
                "time [ms]")
    add_subplot(axs[2, 1], "Decompression and Reading (lz4)", data["lz4"], "decompression and reading", "")
    add_subplot(axs[3, 0], "Compression Ratio (zstd)", data["zstd"], "compressed size", "compressed size [MB]")
    add_subplot(axs[3, 1], "Compression Ratio (lz4)", data["lz4"], "compressed size", "")
    axs[3, 0].set_xlabel("compression level")
    axs[3, 1].set_xlabel("compression level")

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
        plt.savefig(args.output, format="pdf")
    else:
        plt.show()
