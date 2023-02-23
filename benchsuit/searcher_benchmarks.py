"""
Copyright 2023, Leon Freist
Author: Leon Freist <freist@informatik.uni-freiburg.de>

Comparison of different search utilities:
 - re2
 - std::string::find
 - simd::strstr
 - std::strstr
 - std::regex

We use InlineBench for benchmarking the search tools.

"""

import argparse
import json
import multiprocessing
import re
import subprocess
import sys

import requests

from InlineBench_benchmark import InlineBenchCommand, InlineBenchBenchmark
import cmdbench as cb

import os

DATA_DIR = "data"
DATA_FILE = "en.sample.txt"
META_FILE = "en.sample.meta"
DATA_FILE_PATH = ""
META_FILE_PATH = ""
DATA_DOWNLOAD_URL = "https://object.pouta.csc.fi/OPUS-OpenSubtitles/v2016/mono/en.txt.gz"
OUTPUT_DIR = "bench_results"
RESULT_META_DATA = ""
BUILD_PATH = ""


def benchmark_regex(pattern: str, iterations: int) -> InlineBenchBenchmark:
    pre_cmd = [cb.Command("cat", ["cat", DATA_FILE_PATH])]
    commands = [
        InlineBenchCommand("re2 regex", [os.path.join(BUILD_PATH, "re2_search"), pattern, DATA_FILE_PATH]),
        InlineBenchCommand("std::regex", [os.path.join(BUILD_PATH, "std_regex_search"), pattern, DATA_FILE_PATH]),
        InlineBenchCommand("re2 regex -i", [os.path.join(BUILD_PATH, "re2_search"), pattern, DATA_FILE_PATH, "-i"]),
        InlineBenchCommand("std::regex -i", [os.path.join(BUILD_PATH, "std_regex_search"), pattern, DATA_FILE_PATH, "-i"]),
    ]

    return InlineBenchBenchmark(
        name="regex search",
        commands=commands,
        setup_commands=pre_cmd,
        iterations=iterations,
        drop_cache=False
    )


def benchmark_literal(pattern: str, iterations: int) -> InlineBenchBenchmark:
    pre_cmd = [cb.Command("cat", ["cat", DATA_FILE_PATH])]
    commands = [
        InlineBenchCommand("re2 regex", [os.path.join(BUILD_PATH, "re2_search"), pattern, DATA_FILE_PATH, "-l"]),
        InlineBenchCommand("std::strstr", [os.path.join(BUILD_PATH, "std_strstr_search"), pattern, DATA_FILE_PATH]),
        InlineBenchCommand("std::string::find", [os.path.join(BUILD_PATH, "std_string_find_search"), pattern, DATA_FILE_PATH]),
        InlineBenchCommand("simd::strstr", [os.path.join(BUILD_PATH, "simd_strstr"), pattern, DATA_FILE_PATH]),
    ]

    return InlineBenchBenchmark(
        name="literal",
        commands=commands,
        setup_commands=pre_cmd,
        iterations=iterations,
        drop_cache=False
    )


def benchmark_literal_case_insensitive(pattern: str, iterations: int) -> InlineBenchBenchmark:
    pre_cmd = [cb.Command("cat", ["cat", DATA_FILE_PATH])]
    commands = [
        InlineBenchCommand("re2 regex", [os.path.join(BUILD_PATH, "re2_search"), pattern, DATA_FILE_PATH, "-l", "-i"]),
        InlineBenchCommand("std::strstr", [os.path.join(BUILD_PATH, "std_strstr_search"), pattern, DATA_FILE_PATH, "-i"]),
        InlineBenchCommand("std::string::find", [os.path.join(BUILD_PATH, "std_string_find_search"), pattern, DATA_FILE_PATH, "-i"]),
        InlineBenchCommand("simd::strstr", [os.path.join(BUILD_PATH, "simd_strstr"), pattern, DATA_FILE_PATH, "-i"]),
    ]

    return InlineBenchBenchmark(
        name="literal case insensitive",
        commands=commands,
        setup_commands=pre_cmd,
        iterations=iterations,
        drop_cache=False
    )


def download():
    if not os.path.exists(DATA_DIR):
        os.makedirs(DATA_DIR)
    if not os.path.exists(DATA_FILE_PATH):
        cb.log("⏳ Downloading data")
        data = requests.get(DATA_DOWNLOAD_URL, stream=True)
        num_bytes = 0
        with open(DATA_FILE_PATH + ".gz", "wb") as f:
            for chunk in data.iter_content(chunk_size=16384):
                cb.log(f"{num_bytes / 1000000:.2f} MiB written", end='\r', flush=True)
                f.write(chunk)
                num_bytes += len(chunk)
        cb.log("✅ download done")


def decompress():
    if os.path.exists(DATA_FILE_PATH + ".gz"):
        cb.log("⏳ decompressing data")
        p = subprocess.Popen(["gunzip", DATA_FILE_PATH + ".gz"])
        p.wait()
        cb.log("✅ decompression done")


def read_result_info_data(path: str) -> dict:
    if not os.path.exists(path):
        return {}
    with open(path, "r") as f:
        return json.load(f)


def write_result_info_data(meta_data: dict, path: str):
    with open(path, "w") as f:
        json.dump(meta_data, f)


def parse_args():
    parser = argparse.ArgumentParser(prog="searcher benchmarks",
                                     description="Automated benchmarks of different text search utilities")
    parser.add_argument("binary_path", metavar="PATH",
                        help="Path to directory holding the executables.")
    parser.add_argument("--dir", metavar="PATH", default=os.path.join(os.getcwd(), "bench_data"),
                        help="The directory data are downloaded to")
    parser.add_argument("--download", action="store_true",
                        help="Only download data without running benchmarks and exit")
    parser.add_argument("--list-benchmarks", action="store_true", help="List available benchmarks by name and exit")
    parser.add_argument("--iterations", "-i", metavar="INTEGER", type=int, default=3,
                        help="Number of iterations per benchmark")
    parser.add_argument("--pattern", metavar="STRING", default="Sherlock",
                        help="The pattern that is searched by xs")
    parser.add_argument("--output", "-o", metavar="PATH", default="",
                        help="The directory where results are written to (default is printing in terminal)")
    parser.add_argument("--filter", metavar="FILTER", default="", help="Filter benchmarks by name using regex")
    parser.add_argument("--silent", action="store_true", help="Do not output log messages")
    parser.add_argument("--input-file", metavar="PATH", help="Run benchmarks on the provided file")

    return parser.parse_args()


if __name__ == "__main__":
    benchmarks = {
        "regex": benchmark_regex,
        "literal": benchmark_literal,
        "literal case insensitive": benchmark_literal_case_insensitive,
    }
    args = parse_args()
    BUILD_PATH = args.binary_path
    cb.SILENT = args.silent
    if args.list_benchmarks:
        print("The following benchmarks are available:")
        for name in benchmarks.keys():
            print(f" - {name}")
        exit(0)
    if args.dir:
        if os.path.exists(args.dir) and os.path.isdir(args.dir):
            DATA_DIR = args.dir
        else:
            print(f"{args.dir!r} is not a directory or does not exist")
            exit(1)
    DATA_FILE_PATH = os.path.join(DATA_DIR, DATA_FILE)
    META_FILE_PATH = os.path.join(DATA_DIR, META_FILE)
    if args.input_file:
        if os.path.exists(args.input_file) and os.path.isfile(args.input_file):
            DATA_FILE_NAME = args.input_file
            META_FILE_PATH = DATA_FILE_NAME + ".meta"
        else:
            print(f"{args.input_file!r} is not a file or does not exist")
            exit(1)
    else:
        download()
        decompress()
        if args.download:
            sys.exit(0)

    if args.output:
        if not os.path.exists(args.output):
            os.makedirs(args.output)
        OUTPUT_DIR = args.output

    RESULT_META_DATA = os.path.join(OUTPUT_DIR, "results.info.json")

    for name, bm_func in benchmarks.items():
        res = None
        if re.search(args.filter, name):
            cb.log(f"🏃 running {name}...")
            if "regex" in name and args.pattern == "Sherlock":
                pattern = "She[r ]lock"
            else:
                pattern = args.pattern
            res = bm_func(pattern, args.iterations).run()
        else:
            cb.log(f"↦ skipping {name}...")
        if res:
            if args.output:
                result_info_data = read_result_info_data(RESULT_META_DATA)
                tmp_id = 0
                file_name = name.replace(" ", "_")
                tmp_file_name = f"{file_name}_{tmp_id}.json"
                while os.path.exists(os.path.join(OUTPUT_DIR, tmp_file_name)):
                    tmp_id += 1
                    tmp_file_name = f"{file_name}_{tmp_id}.json"
                file_name = f"{file_name}_{tmp_id}"
                output_file = os.path.join(OUTPUT_DIR, file_name)
                result_info_data[file_name + ".json"] = res.get_setup()
                result_info_data[file_name + ".json"]["plot"] = file_name + ".pdf"
                result_info_data[file_name + ".json"]["pattern"] = pattern
                result_info_data[file_name + ".json"]["file"] = DATA_FILE_PATH
                write_result_info_data(result_info_data, RESULT_META_DATA)
                res.write_json(output_file + ".json")
                res.plot(output_file + ".pdf")
            else:
                res.plot()
