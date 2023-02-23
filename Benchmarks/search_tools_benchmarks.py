"""
Copyright 2023, Leon Freist
Author: Leon Freist <freist@informatik.uni-freiburg.de>

Comparison of different command line search tools including:
 - GNU grep (https://www.gnu.org/software/grep/)
 - ripgrep (https://github.com/BurntSushi/ripgrep)
 - xs grep (https://github.com/lfreist/x-search)

We use benchsuit.gnu_time_benchmark for benchmarking the search tools.

Note: Benchmarks only cover searching a pattern on a single file!
"""

import argparse
import json
import re
import subprocess
import sys

import requests

from cmdbench.gnu_time_benchmark import GNUTimeBenchmark, GNUTimeCommand
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


def benchmark_literal_byte_offset(pattern: str, iterations: int, drop_cache: bool) -> GNUTimeBenchmark:
    pre_cmd = []
    if not drop_cache:
        pre_cmd.append(cb.Command("cat", ["cat", DATA_FILE_PATH]))
    meta_file_path = DATA_FILE_PATH + ".meta"
    setup_cmd = [cb.Command("xspp -> metafile", ["xspp", DATA_FILE_PATH, "-m", meta_file_path])]
    cleanup_cmd = [cb.Command("rm metafile", ["rm", meta_file_path])]
    commands = [
        GNUTimeCommand("GNU grep", ["grep", pattern, DATA_FILE_PATH, "-b"], pre_cmd),
        GNUTimeCommand("xs grep", ["xs", pattern, DATA_FILE_PATH, "-b"], pre_cmd),
        GNUTimeCommand("xs grep meta", ["xs", pattern, DATA_FILE_PATH, meta_file_path, "-b"], pre_cmd),
        GNUTimeCommand("xs grep -j 1", ["xs", pattern, DATA_FILE_PATH, "-j", "1", "-b"], pre_cmd),
        GNUTimeCommand("xs grep -j 1 meta", ["xs", pattern, DATA_FILE_PATH, meta_file_path, "-j", "1", "-b"], pre_cmd),
        GNUTimeCommand("ripgrep", ["rg", pattern, DATA_FILE_PATH, "-b"], pre_cmd),
        GNUTimeCommand("ripgrep -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1", "-b"], pre_cmd),
        GNUTimeCommand("cat", ["cat", DATA_FILE_PATH], pre_cmd),
    ]
    return GNUTimeBenchmark(
        name="literal search: byte offsets",
        commands=commands,
        iterations=iterations,
        drop_cache=drop_cache,
        setup_commands=setup_cmd,
        cleanup_commands=cleanup_cmd
    )


def benchmark_literal_line_number(pattern: str, iterations: int, drop_cache: bool) -> GNUTimeBenchmark:
    pre_cmd = []
    if not drop_cache:
        pre_cmd.append(cb.Command("cat", ["cat", DATA_FILE_PATH]))
    meta_file_path = DATA_FILE_PATH + ".meta"
    setup_cmd = [cb.Command("xspp -> metafile", ["xspp", DATA_FILE_PATH, "-m", meta_file_path])]
    cleanup_cmd = [cb.Command("rm metafile", ["rm", meta_file_path])]
    commands = [
        GNUTimeCommand("GNU grep", ["grep", pattern, DATA_FILE_PATH, "-n"], pre_cmd),
        GNUTimeCommand("xs grep", ["xs", pattern, DATA_FILE_PATH, "-n"], pre_cmd),
        GNUTimeCommand("xs grep meta", ["xs", pattern, DATA_FILE_PATH, meta_file_path, "-n"], pre_cmd),
        GNUTimeCommand("xs grep -j 1", ["xs", pattern, DATA_FILE_PATH, "-j", "1", "-n"], pre_cmd),
        GNUTimeCommand("xs grep -j 1 meta", ["xs", pattern, DATA_FILE_PATH, meta_file_path, "-j", "1", "-n"], pre_cmd),
        GNUTimeCommand("ripgrep", ["rg", pattern, DATA_FILE_PATH, "-n"], pre_cmd),
        GNUTimeCommand("ripgrep -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1", "-n"], pre_cmd),
        GNUTimeCommand("cat", ["cat", DATA_FILE_PATH], pre_cmd),
    ]
    return GNUTimeBenchmark(
        name="literal search: line numbers",
        commands=commands,
        iterations=iterations,
        drop_cache=drop_cache,
        setup_commands=setup_cmd,
        cleanup_commands=cleanup_cmd
    )


def benchmark_literal(pattern: str, iterations: int, drop_cache: bool) -> GNUTimeBenchmark:
    """
    Benchmark plain text search
    """
    pre_cmd = []
    if not drop_cache:
        pre_cmd.append(cb.Command("cat", ["cat", DATA_FILE_PATH]))
    meta_file_path = DATA_FILE_PATH + ".meta"
    setup_cmd = [cb.Command("xspp -> metafile", ["xspp", DATA_FILE_PATH, "-m", meta_file_path])]
    cleanup_cmd = [cb.Command("rm metafile", ["rm", meta_file_path])]
    commands = [
        GNUTimeCommand("GNU grep", ["grep", pattern, DATA_FILE_PATH], pre_cmd),
        GNUTimeCommand("xs grep", ["xs", pattern, DATA_FILE_PATH], pre_cmd),
        GNUTimeCommand("xs grep meta", ["xs", pattern, DATA_FILE_PATH, meta_file_path], pre_cmd),
        GNUTimeCommand("xs grep -j 1", ["xs", pattern, DATA_FILE_PATH, "-j", "1"], pre_cmd),
        GNUTimeCommand("xs grep -j 1 meta", ["xs", pattern, DATA_FILE_PATH, meta_file_path, "-j", "1"], pre_cmd),
        GNUTimeCommand("ripgrep", ["rg", pattern, DATA_FILE_PATH], pre_cmd),
        GNUTimeCommand("ripgrep -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1"], pre_cmd),
        GNUTimeCommand("cat", ["cat", DATA_FILE_PATH], pre_cmd),
    ]
    return GNUTimeBenchmark(
        name="literal search: lines",
        commands=commands,
        iterations=iterations,
        drop_cache=drop_cache,
        setup_commands=setup_cmd,
        cleanup_commands=cleanup_cmd
    )


def benchmark_literal_case_insensitive(pattern: str, iterations: int, drop_cache: bool) -> GNUTimeBenchmark:
    """
    Benchmark plain text search
    """
    pre_cmd = []
    if not drop_cache:
        pre_cmd.append(cb.Command("cat", ["cat", DATA_FILE_PATH]))
    meta_file_path = DATA_FILE_PATH + ".meta"
    setup_cmd = [cb.Command("xspp -> metafile", ["xspp", DATA_FILE_PATH, "-m", meta_file_path])]
    cleanup_cmd = [cb.Command("rm metafile", ["rm", meta_file_path])]
    commands = [
        GNUTimeCommand("GNU grep", ["grep", pattern, DATA_FILE_PATH, "-i"], pre_cmd),
        GNUTimeCommand("xs grep", ["xs", pattern, DATA_FILE_PATH, "-i"], pre_cmd),
        GNUTimeCommand("xs grep meta", ["xs", pattern, DATA_FILE_PATH, meta_file_path, "-i"], pre_cmd),
        GNUTimeCommand("xs grep -j 1", ["xs", pattern, DATA_FILE_PATH, "-j", "1", "-i"], pre_cmd),
        GNUTimeCommand("xs grep -j 1 meta", ["xs", pattern, DATA_FILE_PATH, meta_file_path, "-j", "1", "-i"], pre_cmd),
        GNUTimeCommand("ripgrep", ["rg", pattern, DATA_FILE_PATH, "-i"], pre_cmd),
        GNUTimeCommand("ripgrep -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1", "-i"], pre_cmd),
        GNUTimeCommand("cat", ["cat", DATA_FILE_PATH], pre_cmd),
    ]
    return GNUTimeBenchmark(
        name="literal case insensitive search: lines",
        commands=commands,
        iterations=iterations,
        drop_cache=drop_cache,
        setup_commands=setup_cmd,
        cleanup_commands=cleanup_cmd
    )


def benchmark_regex(pattern: str, iterations: int, drop_cache: bool) -> GNUTimeBenchmark:
    pre_cmd = []
    if not drop_cache:
        pre_cmd.append(cb.Command("cat -> cache file", ["cat", DATA_FILE_PATH]))
    meta_file_path = DATA_FILE_PATH + ".meta"
    setup_cmd = [cb.Command("xspp -> metafile", ["xspp", DATA_FILE_PATH, "-m", meta_file_path])]
    cleanup_cmd = [cb.Command("rm metafile", ["rm", meta_file_path])]
    commands = [
        GNUTimeCommand("GNU grep", ["grep", pattern, DATA_FILE_PATH], pre_cmd),
        GNUTimeCommand("xs grep", ["xs", pattern, DATA_FILE_PATH], pre_cmd),
        GNUTimeCommand("xs grep meta", ["xs", pattern, DATA_FILE_PATH, meta_file_path], pre_cmd),
        GNUTimeCommand("xs grep -j 1", ["xs", pattern, DATA_FILE_PATH, "-j", "1"], pre_cmd),
        GNUTimeCommand("xs grep -j 1 meta", ["xs", pattern, DATA_FILE_PATH, meta_file_path, "-j", "1"], pre_cmd),
        GNUTimeCommand("ripgrep", ["rg", pattern, DATA_FILE_PATH], pre_cmd),
        GNUTimeCommand("ripgrep -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1"], pre_cmd),
        GNUTimeCommand("cat", ["cat", DATA_FILE_PATH], pre_cmd),
    ]
    return GNUTimeBenchmark(
        name="regex search: lines",
        commands=commands,
        iterations=iterations,
        drop_cache=drop_cache,
        setup_commands=setup_cmd,
        cleanup_commands=cleanup_cmd
    )


def benchmark_regex_line_number(pattern: str, iterations: int, drop_cache: bool) -> GNUTimeBenchmark:
    pre_cmd = []
    if not drop_cache:
        pre_cmd.append(cb.Command("cat", ["cat", DATA_FILE_PATH]))
    meta_file_path = DATA_FILE_PATH + ".meta"
    setup_cmd = [cb.Command("xspp -> metafile", ["xspp", DATA_FILE_PATH, "-m", meta_file_path])]
    cleanup_cmd = [cb.Command("rm metafile", ["rm", meta_file_path])]
    commands = [
        GNUTimeCommand("GNU grep", ["grep", pattern, DATA_FILE_PATH, "-n"], pre_cmd),
        GNUTimeCommand("xs grep", ["xs", pattern, DATA_FILE_PATH, "-n"], pre_cmd),
        GNUTimeCommand("xs grep meta", ["xs", pattern, DATA_FILE_PATH, meta_file_path, "-n"], pre_cmd),
        GNUTimeCommand("xs grep -j 1", ["xs", pattern, DATA_FILE_PATH, "-j", "1", "-n"], pre_cmd),
        GNUTimeCommand("xs grep -j 1 meta", ["xs", pattern, DATA_FILE_PATH, meta_file_path, "-j", "1", "-n"], pre_cmd),
        GNUTimeCommand("ripgrep", ["rg", pattern, DATA_FILE_PATH, "-n"], pre_cmd),
        GNUTimeCommand("ripgrep -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1", "-n"], pre_cmd),
        GNUTimeCommand("cat", ["cat", DATA_FILE_PATH], pre_cmd),
    ]
    return GNUTimeBenchmark(
        name="literal search: line numbers",
        commands=commands,
        iterations=iterations,
        drop_cache=drop_cache,
        setup_commands=setup_cmd,
        cleanup_commands=cleanup_cmd
    )


def benchmark_regex_byte_offset(pattern: str, iterations: int, drop_cache: bool) -> GNUTimeBenchmark:
    pre_cmd = []
    if not drop_cache:
        pre_cmd.append(cb.Command("cat", ["cat", DATA_FILE_PATH]))
    meta_file_path = DATA_FILE_PATH + ".meta"
    setup_cmd = [cb.Command("xspp -> metafile", ["xspp", DATA_FILE_PATH, "-m", meta_file_path])]
    cleanup_cmd = [cb.Command("rm metafile", ["rm", meta_file_path])]
    commands = [
        GNUTimeCommand("GNU grep", ["grep", pattern, DATA_FILE_PATH, "-b"], pre_cmd),
        GNUTimeCommand("xs grep", ["xs", pattern, DATA_FILE_PATH, "-b"], pre_cmd),
        GNUTimeCommand("xs grep meta", ["xs", pattern, DATA_FILE_PATH, meta_file_path, "-b"], pre_cmd),
        GNUTimeCommand("xs grep -j 1", ["xs", pattern, DATA_FILE_PATH, "-j", "1", "-b"], pre_cmd),
        GNUTimeCommand("xs grep -j 1 meta", ["xs", pattern, DATA_FILE_PATH, meta_file_path, "-j", "1", "-b"], pre_cmd),
        GNUTimeCommand("ripgrep", ["rg", pattern, DATA_FILE_PATH, "-b"], pre_cmd),
        GNUTimeCommand("ripgrep -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1", "-b"], pre_cmd),
        GNUTimeCommand("cat", ["cat", DATA_FILE_PATH], pre_cmd),
    ]
    return GNUTimeBenchmark(
        name="literal search: byte offsets",
        commands=commands,
        iterations=iterations,
        drop_cache=drop_cache,
        setup_commands=setup_cmd,
        cleanup_commands=cleanup_cmd
    )


def benchmark_regex_case_insensitive(pattern: str, iterations: int, drop_cache: bool) -> GNUTimeBenchmark:
    pre_cmd = []
    if not drop_cache:
        pre_cmd.append(cb.Command("cat", ["cat", DATA_FILE_PATH]))
    meta_file_path = DATA_FILE_PATH + ".meta"
    setup_cmd = [cb.Command("xspp -> metafile", ["xspp", DATA_FILE_PATH, "-m", meta_file_path])]
    cleanup_cmd = [cb.Command("rm metafile", ["rm", meta_file_path])]
    commands = [
        GNUTimeCommand("GNU grep", ["grep", pattern, DATA_FILE_PATH, "-i"], pre_cmd),
        GNUTimeCommand("xs grep", ["xs", pattern, DATA_FILE_PATH, "-i"], pre_cmd),
        GNUTimeCommand("xs grep meta", ["xs", pattern, DATA_FILE_PATH, meta_file_path, "-i"], pre_cmd),
        GNUTimeCommand("xs grep -j 1", ["xs", pattern, DATA_FILE_PATH, "-j", "1", "-i"], pre_cmd),
        GNUTimeCommand("xs grep -j 1 meta", ["xs", pattern, DATA_FILE_PATH, meta_file_path, "-j", "1", "-i"], pre_cmd),
        GNUTimeCommand("ripgrep", ["rg", pattern, DATA_FILE_PATH, "-i"], pre_cmd),
        GNUTimeCommand("ripgrep -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1", "-i"], pre_cmd),
        GNUTimeCommand("cat", ["cat", DATA_FILE_PATH], pre_cmd),
    ]
    return GNUTimeBenchmark(
        name="literal search: byte offsets",
        commands=commands,
        iterations=iterations,
        drop_cache=drop_cache,
        setup_commands=setup_cmd,
        cleanup_commands=cleanup_cmd
    )


def benchmark_zstd_input(pattern: str, iterations: int, drop_cache: bool) -> GNUTimeBenchmark:
    pre_cmd = []
    xs_pre_cmd = []
    zstd_file = DATA_FILE_PATH + ".zst"
    xs_zstd_file = DATA_FILE_PATH + ".xszst"
    xs_zstd_meta = DATA_FILE_PATH + ".xszst.meta"
    setup_cmd = [
        cb.Command("zstd", ["zstd", DATA_FILE_PATH, "-o", zstd_file]),
        cb.Command("xspp zstd", ["xspp", DATA_FILE_PATH, "-o", xs_zstd_file, "-m", xs_zstd_meta, "-a" "zstd"])
    ]
    cleanup_cmd = [
        cb.Command("rm zst", ["rm", zstd_file]),
        cb.Command("rm xszst", ["rm", xs_zstd_file]),
        cb.Command("rm xszst.meta", ["rm", xs_zstd_meta])
    ]
    if not drop_cache:
        pre_cmd.append(cb.Command("cat", ["cat", zstd_file]))
        xs_pre_cmd.append(cb.Command("cat", ["cat", xs_zstd_meta]))
        xs_pre_cmd.append(cb.Command("cat", ["cat", xs_zstd_file]))
    commands = [
        GNUTimeCommand("zstdcat | GNU grep", ["grep", pattern, DATA_FILE_PATH], pre_cmd),
        GNUTimeCommand("zstdcat | xs grep", ["xs", pattern, DATA_FILE_PATH], pre_cmd),
        GNUTimeCommand("xs grep meta", ["xs", pattern, xs_zstd_file, xs_zstd_meta], xs_pre_cmd),
        GNUTimeCommand("zstdcat | xs grep -j 1", ["xs", pattern, DATA_FILE_PATH, "-j", "1"], pre_cmd),
        GNUTimeCommand("xs grep -j 1 meta", ["xs", pattern, xs_zstd_file, xs_zstd_meta, "-j", "1"], xs_pre_cmd),
        GNUTimeCommand("zstdcat | ripgrep", ["rg", pattern, DATA_FILE_PATH], pre_cmd),
        GNUTimeCommand("zstdcat | ripgrep -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1"], pre_cmd),
        GNUTimeCommand("zstdcat", ["zstdcat", zstd_file], pre_cmd),
    ]
    return GNUTimeBenchmark(
        name="literal search: ZStandard compressed input",
        commands=commands,
        iterations=iterations,
        drop_cache=drop_cache,
        setup_commands=setup_cmd,
        cleanup_commands=cleanup_cmd
    )


def benchmark_lz4_input(pattern: str, iterations: int, drop_cache: bool) -> GNUTimeBenchmark:
    pre_cmd = []
    xs_pre_cmd = []
    lz4_file = DATA_FILE_PATH + ".lz4"
    xs_lz4_file = DATA_FILE_PATH + ".xslz4"
    xs_lz4_meta = DATA_FILE_PATH + ".xslz4.meta"
    setup_cmd = [
        cb.Command("lz4 HC", ["lz4", DATA_FILE_PATH, lz4_file, "-9"]),
        cb.Command("xspp lz4 HC", ["xspp", DATA_FILE_PATH, "-o", xs_lz4_file, "-m", xs_lz4_meta, "-a" "lz4", "--hc"])
    ]
    cleanup_cmd = [
        cb.Command("rm lz4", ["rm", lz4_file]),
        cb.Command("rm xslz4", ["rm", xs_lz4_file]),
        cb.Command("rm xslz4.meta", ["rm", xs_lz4_meta])
    ]
    if not drop_cache:
        pre_cmd.append(cb.Command("cat", ["cat", lz4_file]))
        xs_pre_cmd.append(cb.Command("cat", ["cat", xs_lz4_meta]))
        xs_pre_cmd.append(cb.Command("cat", ["cat", xs_lz4_file]))
    commands = [
        GNUTimeCommand("zstdcat | GNU grep", ["grep", pattern, DATA_FILE_PATH], pre_cmd),
        GNUTimeCommand("zstdcat | xs grep", ["xs", pattern, DATA_FILE_PATH], pre_cmd),
        GNUTimeCommand("xs grep meta", ["xs", pattern, xs_lz4_file, xs_lz4_meta], xs_pre_cmd),
        GNUTimeCommand("zstdcat | xs grep -j 1", ["xs", pattern, DATA_FILE_PATH, "-j", "1"], pre_cmd),
        GNUTimeCommand("xs grep -j 1 meta", ["xs", pattern, xs_lz4_file, xs_lz4_meta, "-j", "1"], xs_pre_cmd),
        GNUTimeCommand("zstdcat | ripgrep", ["rg", pattern, DATA_FILE_PATH], pre_cmd),
        GNUTimeCommand("zstdcat | ripgrep -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1"], pre_cmd),
        GNUTimeCommand("zstdcat", ["zstdcat", lz4_file], pre_cmd),
    ]
    return GNUTimeBenchmark(
        name="literal search: LZ4 compressed input",
        commands=commands,
        iterations=iterations,
        drop_cache=drop_cache,
        setup_commands=setup_cmd,
        cleanup_commands=cleanup_cmd
    )


def download():
    if not os.path.exists(DATA_DIR):
        os.makedirs(DATA_DIR)
    if not os.path.exists(DATA_FILE_PATH):
        cb.log("Downloading data...")
        data = requests.get(DATA_DOWNLOAD_URL, stream=True)
        num_bytes = 0
        with open(DATA_FILE_PATH + ".gz", "wb") as f:
            for chunk in data.iter_content(chunk_size=16384):
                cb.log(f"{num_bytes / 1000000:.2f} MiB written", end='\r', flush=True)
                f.write(chunk)
                num_bytes += len(chunk)
        cb.log()


def decompress():
    if os.path.exists(DATA_FILE_PATH + ".gz"):
        cb.log("Decompressing data...")
        p = subprocess.Popen(["gunzip", DATA_FILE_PATH + ".gz"])
        p.wait()
        cb.log("Done")


def read_result_info_data(path: str):
    if not os.path.exists(path):
        return {}
    with open(path, "r") as f:
        return json.load(f)


def write_result_info_data(meta_data: dict, path: str):
    with open(path, "w") as f:
        json.dump(meta_data, f)


def parse_args():
    parser = argparse.ArgumentParser(prog="benchsuit",
                                     description="Automated benchmarks of xs/grep")
    parser.add_argument("--dir", metavar="PATH", default=os.path.join(os.getcwd(), "bench_data"),
                        help="The directory data are downloaded to")
    parser.add_argument("--download", action="store_true",
                        help="Only download data without running benchmarks and exit")
    parser.add_argument("--list-benchmarks", action="store_true", help="List available benchmarks by name and exit")
    parser.add_argument("--drop-cache", action="store_true", help="Drop RAM caches before benchmark runs")
    parser.add_argument("--iterations", "-i", metavar="INTEGER", type=int, default=3,
                        help="Number of iterations per benchmark")
    parser.add_argument("--pattern", metavar="STRING", default="Sherlock",
                        help="The pattern that is searched during benchmarks")
    parser.add_argument("--output", "-o", metavar="PATH", default="",
                        help="The directory where results are written to (default is printing in terminal)")
    parser.add_argument("--filter", metavar="FILTER", default="", help="Filter benchmarks by name using regex")
    parser.add_argument("--input-file", metavar="PATH", help="Run benchmarks on the provided file")
    parser.add_argument("--silent", action="store_true", help="Do not output log messages")

    return parser.parse_args()


if __name__ == "__main__":
    benchmarks = {
        "comparison: literal, lines": benchmark_literal,
        "comparison: literal, line numbers": benchmark_literal_line_number,
        "comparison: literal, byte offset": benchmark_literal_byte_offset,
        "comparison: literal, case insensitive": benchmark_literal_case_insensitive,
        "comparison: regex": benchmark_regex,
        "comparison: regex, line numbers": benchmark_regex_line_number,
        "comparison: regex, byte offset": benchmark_regex_byte_offset,
        "comparison: regex, case insensitive": benchmark_regex_case_insensitive,
        "comparison: zstd compressed input file": benchmark_zstd_input,
        "comparison: lz4 compressed input file": benchmark_lz4_input,
    }
    args = parse_args()
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
            cb.log(f"Running {name}...")
            if "regex" in name:
                pattern = "She[r ]lock"
            else:
                pattern = args.pattern
            res = bm_func(pattern, args.iterations, args.drop_cache).run()
        else:
            cb.log(f"Skipping {name}...")
        if res:
            if args.output:
                result_info_data = read_result_info_data(RESULT_META_DATA)
                tmp_id = 0
                file_name = name.replace(" ", "_")
                tmp_file_name = f"{file_name}_{tmp_id}"
                while os.path.exists(os.path.join(OUTPUT_DIR, tmp_file_name)):
                    tmp_id += 1
                    tmp_file_name = f"{file_name}_{tmp_id}"
                file_name = tmp_file_name
                output_file = os.path.join(OUTPUT_DIR, file_name)
                res.write_json(output_file + ".json")
                result_info_data[file_name] = res.get_setup()
                result_info_data[file_name]["plot"] = output_file + ".pdf"
                write_result_info_data(result_info_data, RESULT_META_DATA)
                res.plot(output_file + ".pdf")
            else:
                res.plot()
                print(f"===== {res.get_setup()['name']} =====")
                print()
                print(res.get_result())
