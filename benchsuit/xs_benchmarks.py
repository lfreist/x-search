"""
Copyright 2023, Leon Freist
Author: Leon Freist <freist@informatik.uni-freiburg.de>

Comparison of different usages of xs grep including:
 - ...

We use InlineBench for benchmarking the search tools.

"""

import argparse
import json
import multiprocessing
import re
import subprocess
import sys

import requests

from cmdbench.InlineBench_benchmark import InlineBenchCommand, InlineBenchBenchmark
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
BENCHMARK_BUILD_XS = ""


def benchmark_chunk_size(pattern: str, iterations: int, drop_cache: bool) -> InlineBenchBenchmark:
    pre_cmd = []
    if not drop_cache:
        pre_cmd.append(cb.Command("cat", ["cat", DATA_FILE_PATH]))
    commands = []
    units = ["B", "KiB", "MiB", "GiB"]
    for size_bytes in [1024, 4096, 32768, 262144, 2097152, 16777216, 134217728, 1073741824]:
        unit = units[0]
        size = size_bytes
        for unit_i in range(len(units) - 1):
            if len(str(size)) > 3:
                size = int(size / 1024)
                unit = units[unit_i + 1]
        commands.append(
            InlineBenchCommand(f"{size} {unit}", [BENCHMARK_BUILD_XS, pattern, DATA_FILE_PATH, "-s", str(size_bytes), "-j", "1"]))

    return InlineBenchBenchmark(
        name="chunks sizes (plain)",
        commands=commands,
        setup_commands=pre_cmd,
        iterations=iterations,
        drop_cache=drop_cache
    )


def benchmark_chunk_size_xspp(pattern: str, iterations: int, drop_cache: bool) -> InlineBenchBenchmark:
    commands = []
    setup_commands = []
    cleanup_commands = []
    units = ["B", "KiB", "MiB", "GiB"]
    for size_bytes in [1024, 4096, 32768, 262144, 2097152, 16777216, 134217728, 1073741824]:
        meta_file = f"{DATA_FILE_PATH}-s-{size_bytes}.meta"
        setup_commands.append(cb.Command("xspp", ["xspp", DATA_FILE_PATH, "-m", meta_file, "-s", str(size_bytes)]))
        cleanup_commands.append(cb.Command("rm", ["rm", meta_file]))
        size = size_bytes
        unit = units[0]
        for unit_i in range(len(units) - 1):
            if len(str(size)) > 3:
                size = int(size / 1024)
                unit = units[unit_i + 1]

        commands.append(
            InlineBenchCommand(f"{size} {unit}", [BENCHMARK_BUILD_XS, pattern, DATA_FILE_PATH, meta_file, "-j", "1"]))

    if not drop_cache:
        setup_commands.append(cb.Command("cat", ["cat", DATA_FILE_PATH]))

    return InlineBenchBenchmark(
        name="chunks sizes (plain)",
        commands=commands,
        setup_commands=setup_commands,
        cleanup_commands=cleanup_commands,
        iterations=iterations,
        drop_cache=drop_cache
    )


def benchmark_num_threads(pattern: str, iterations: int, drop_cache: bool) -> InlineBenchBenchmark:
    pre_cmd = []
    if not drop_cache:
        pre_cmd.append(cb.Command("cat", ["cat", DATA_FILE_PATH]))
    commands = [InlineBenchCommand(f"xs", [BENCHMARK_BUILD_XS, pattern, DATA_FILE_PATH])]
    for nt in [1] + list(range(2, multiprocessing.cpu_count() + 1, 2)):  # [1, 2, 4, 6, ...]
        commands.append(InlineBenchCommand(f"{nt}", [BENCHMARK_BUILD_XS, pattern, DATA_FILE_PATH, "-j", str(nt)]))

    return InlineBenchBenchmark(
        name="number of threads (plain)",
        commands=commands,
        setup_commands=pre_cmd,
        iterations=iterations,
        drop_cache=drop_cache
    )


def benchmark_options(pattern: str, iterations: int, drop_cache: bool) -> InlineBenchBenchmark:
    pre_cmd = []
    if not drop_cache:
        pre_cmd.append(cb.Command("cat", ["cat", DATA_FILE_PATH]))
    commands = [
        InlineBenchCommand(f"xs", [BENCHMARK_BUILD_XS, pattern, DATA_FILE_PATH]),
        InlineBenchCommand(f"xs -i", [BENCHMARK_BUILD_XS, pattern, DATA_FILE_PATH, "-i"]),
        InlineBenchCommand(f"xs -n", [BENCHMARK_BUILD_XS, pattern, DATA_FILE_PATH, "-n"]),
        InlineBenchCommand(f"xs -n -i", [BENCHMARK_BUILD_XS, pattern, DATA_FILE_PATH, "-i", "-n"]),
    ]

    return InlineBenchBenchmark(
        name="xs options (-n, -i)",
        commands=commands,
        iterations=iterations,
        setup_commands=pre_cmd,
        drop_cache=drop_cache
    )


def benchmark_chunk_nl_mapping_data_xspp(pattern: str, iterations: int, drop_cache: bool) -> InlineBenchBenchmark:
    preprocess_commands = []
    commands = []
    cleanup_commands = []
    for dist in ["1", "500", "1000", "5000", "32000"]:
        meta_file = f"{DATA_FILE_PATH}-nl-{dist}.meta"
        preprocess_commands.append(cb.Command(f"xspp -d {dist}", ["xspp", DATA_FILE_PATH, "-m", meta_file, "-d", dist]))
        cleanup_commands.append(cb.Command(f"rm {meta_file}", ["rm", meta_file]))
        commands.append(
            InlineBenchCommand(f"{dist} Bytes", [BENCHMARK_BUILD_XS, pattern, DATA_FILE_PATH, meta_file, "-j", "1"]))

    if not drop_cache:
        preprocess_commands.append(cb.Command("cat", ["cat", DATA_FILE_PATH]))

    return InlineBenchBenchmark(
        name="xs options (-n, -i)",
        commands=commands,
        setup_commands=preprocess_commands,
        cleanup_commands=cleanup_commands,
        iterations=iterations,
        drop_cache=drop_cache
    )


def benchmark_compressions_xspp(pattern: str, iterations: int, drop_cache: bool) -> InlineBenchBenchmark:
    preprocess_commands = []
    commands = []
    cleanup_commands = []
    compression_args = [
        ["-a", "none"],
        ["-a", "zstd", "-l", "1"],
        ["-a", "zstd", "-l", "3"],
        ["-a", "zstd", "-l", "7"],
        ["-a", "lz4"],
        ["-a", "lz4", "--hc"],
        ["-a", "lz4", "--hc", "-l", "9"]
    ]
    for arg in compression_args:
        meta_file = f"{DATA_FILE_PATH}{''.join(arg)}.meta"
        compressed_file = f"{DATA_FILE_PATH}{''.join(arg)}"
        pre_cmd = [] if drop_cache else [cb.Command("cat", ["cat", compressed_file])]
        preprocess_commands.append(
            cb.Command(f"xspp {' '.join(arg)}", ["xspp", DATA_FILE_PATH, "-o", compressed_file, "-m", meta_file] + arg))
        commands.append(
            InlineBenchCommand(f"xs {' '.join(arg)}",
                               [BENCHMARK_BUILD_XS, pattern, compressed_file, meta_file, "-j", "1"],
                               pre_cmd))
        cleanup_commands.append(cb.Command(f"rm {compressed_file}", ["rm", compressed_file]))
        cleanup_commands.append(cb.Command(f"rm {meta_file}", ["rm", meta_file]))

    return InlineBenchBenchmark(
        name="xs options (-n, -i)",
        commands=commands,
        setup_commands=preprocess_commands,
        cleanup_commands=cleanup_commands,
        iterations=iterations,
        drop_cache=drop_cache
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
    parser = argparse.ArgumentParser(prog="xs_benchmark",
                                     description="Automated benchmarks of xs grep")
    parser.add_argument("xs_benchmark_build", metavar="XS_PATH",
                        help="Path to xs executable built with -DBENCHMARK flag.")
    parser.add_argument("--dir", metavar="PATH", default=os.path.join(os.getcwd(), "bench_data"),
                        help="The directory data are downloaded to")
    parser.add_argument("--download", action="store_true",
                        help="Only download data without running benchmarks and exit")
    parser.add_argument("--list-benchmarks", action="store_true", help="List available benchmarks by name and exit")
    parser.add_argument("--drop-cache", action="store_true", help="Drop RAM caches before benchmark run")
    parser.add_argument("--iterations", "-i", metavar="INTEGER", type=int, default=3,
                        help="Number of iterations per benchmark")
    parser.add_argument("--pattern", metavar="STRING", default="Sherlock",
                        help="The pattern that is searched by xs")
    parser.add_argument("--output", "-o", metavar="PATH", default="",
                        help="The directory where results are written to (default is printing in terminal)")
    parser.add_argument("--filter", metavar="FILTER", default="", help="Filter benchmarks by name using regex")
    parser.add_argument("--silent", action="store_true", help="Do not output log messages")
    parser.add_argument("--plot", action="store_true", help="Plot results. If output is set, plots are stored as pdf")
    parser.add_argument("--input-file", metavar="PATH", help="Run benchmarks on the provided file")

    return parser.parse_args()


if __name__ == "__main__":
    benchmarks = {
        "xs grep: chunk size (plain)": benchmark_chunk_size,
        "xs grep: chunk size (preprocessed)": benchmark_chunk_size_xspp,
        "xs grep: new line mapping data distance (preprocessed)": benchmark_chunk_nl_mapping_data_xspp,
        "xs grep: compressions (preprocessed)": benchmark_compressions_xspp,
        "xs grep: number of worker threads (plain)": benchmark_num_threads,
        "xs grep: using available options (-n, -i)": benchmark_options
    }
    args = parse_args()
    BENCHMARK_BUILD_XS = args.xs_benchmark_build
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
            res = bm_func(args.pattern, args.iterations, args.drop_cache).run()
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
                result_info_data[file_name + ".json"]["pattern"] = args.pattern
                result_info_data[file_name + ".json"]["file"] = DATA_FILE_PATH
                write_result_info_data(result_info_data, RESULT_META_DATA)
                res.write_json(output_file + ".json")
                res.plot(output_file + ".pdf")
            else:
                res.plot()
