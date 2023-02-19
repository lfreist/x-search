#!/usr/bin/env python3

"""
Copyright 2023, Leon Freist
Author: Leon Freist <freist@informatik.uni-freiburg.de>
"""
import os
import shutil
import sys
import time
from typing import List, Dict
import subprocess
import statistics
import pandas as pd
import json
import requests
import re
import platform
import argparse
import uuid

DATA_DIR = "bench_data"
DATA_FILE = "en.sample.txt"
META_FILE = "en.sample.meta"
DATA_FILE_PATH = ""
META_FILE_PATH = ""
DATA_DOWNLOAD_URL = "https://object.pouta.csc.fi/OPUS-OpenSubtitles/v2016/mono/en.txt.gz"
OUTPUT_DIR = "bench_results"
RESULT_META_DATA = ""
BENCHMARK_BUILD_XS = ""

SILENT = False

OUTPUT_FORMAT = ["json", "csv", "markdown"]


def get_cpu_name():
    with open("/proc/cpuinfo", "r") as f:
        for line in f.readlines():
            if "model name" in line:
                return re.sub(".*model name.*:", "", line, 1).strip()


def DROP_RAM_CACHE():
    subprocess.run("sync")
    with open("/proc/sys/vm/drop_caches", "w") as f:
        f.write("3")
    # give the system some time to recover...
    time.sleep(1)


class XsResult:
    def __init__(self, command, data: Dict):
        self.name = command.name
        self.command = " ".join(command.cmd)
        self.data = self.normalize_inline_bench_data(data)

    @staticmethod
    def normalize_inline_bench_data(data: Dict):
        """
        Flattens the output of the json result of InlineBench into
        {
        "CPU": {"task": {"time [ns]": [int...], "num threads": int}, ...},
        "Wall": {"task": {"time [ns]": [int...], "num threads": int}, ...}
        }
        :param data:
        :return:
        """
        ret = {"CPU": {}, "Wall": {}}
        if data:
            for type_name, measurements in data.items():
                for task, m in measurements.items():
                    if task in ret[type_name].keys():
                        ret[type_name][task]["time [ns]"] += [m["time"]]
                        ret[type_name][task]["num threads"] += 1
                    else:
                        ret[type_name][task]["time [ns]"] = [m["time"]]
                        ret[type_name][task]["num threads"] = 1
        return ret

    def __iadd__(self, other):
        assert self.name == other.name and self.command == other.command
        for type_name, measurements in other.data.items():
            for task, value in measurements.items():
                if task in self.data[type_name].keys() and "time [ns]" in self.data[type_name][task].keys():
                    self.data[type_name][task]["time [ns]"] += [value["time [ns]"]]
                else:
                    self.data[type_name][task]["time [ns]"] = [value["time [ns]"]]
        return self

    def get_data(self):
        return self.data

    def get_cpu(self, task: str = ""):
        if task:
            return self.data["CPU"][task]
        return self.data["CPU"]

    def get_wall(self, task: str = ""):
        if task:
            return self.data["Wall"][task]
        return self.data["Wall"]

    def mean(self, measure_type: str = "wall") -> dict:
        if measure_type.lower() == "wall":
            return {task: statistics.mean(m["time [ns]"]) for task, m in self.data["Wall"].items()}
        elif measure_type.lower() == "cpu":
            return {task: statistics.mean(m["time [ns]"]) for task, m in self.data["CPU"].items()}
        raise IndexError(f"{measure_type} unknown. Use one of ('wall', 'cpu').")

    def stdev(self, measure_type: str) -> dict:
        if measure_type.lower() == "wall":
            return {task: statistics.mean(m["time [ns]"]) if len(m["time [ns]"]) > 2 else float("nan") for task, m in
                    self.data["Wall"].items()}
        elif measure_type.lower() == "cpu":
            return {task: statistics.mean(m["time [ns]"]) if len(m["time [ns]"]) > 2 else float("nan") for task, m in
                    self.data["CPU"].items()}
        raise IndexError(f"{measure_type} unknown. Use one of ('wall', 'cpu').")


class Command:
    def __init__(self, name: str, cmd: List[str] | str):
        self.name = name
        self.cmd = cmd

    def exists(self) -> bool:
        return shutil.which(self.get_binary_name()) is not None or type(self.cmd) == str

    def get_binary_name(self) -> str:
        return self.cmd[0]

    def run(self) -> XsResult:
        out = subprocess.Popen(self.cmd, stderr=subprocess.PIPE, stdout=subprocess.DEVNULL,
                               shell=(type(self.cmd) == str)).communicate()[1]
        out = out.decode()
        data = json.loads(out)
        try:
            return XsResult(self, data)
        except ValueError:
            raise CommandFailedError(f"Error occurred while running {self.name}.")

    def __str__(self):
        if type(self.cmd) == str:
            return self.cmd
        return f"{self.name}: {' '.join(self.cmd)!r}"


class InvalidCommandError(Exception):
    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return self.msg


class CommandFailedError(Exception):
    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return self.msg


class ComparisonBenchmark:
    def __init__(self, name: str, pattern: str, commands: List[Command], file: str,
                 initial_command: List[Command] = None,
                 description: str = "",
                 benchmark_count: int = 3, cache: bool = False):
        self.name = name
        self.pattern = pattern
        self.commands = commands
        self.description = description
        self.file = file
        self.benchmark_count = benchmark_count
        self.cache = cache
        self.initial_command = initial_command
        for cmd in self.commands:
            if not cmd.exists():
                raise InvalidCommandError(f"{cmd} could not be found.")

    def _run_warmup(self):
        if self.cache:
            log("  Running warmup...")
            counter = 0
            for cmd in self.commands:
                log(f"  {counter}/{len(self.commands)}", end='\r', flush=True)
                cmd.run()
                counter += 1
            log("  -> Warmup done.")

    def _run_benchmarks(self):
        bm_result = XsResult(self)
        for n in range(self.benchmark_count):
            for cmd in self.commands:
                log(f"  {n}/{self.benchmark_count}: {cmd.name}", end='\r', flush=True)
                if not self.cache:
                    DROP_RAM_CACHE()
                bm_result += cmd.run()
        return bm_result

    def _run_initial_command(self):
        if self.initial_command:
            for cmd in self.initial_command:
                log(f"  running initial command {cmd.name}...")
                result = cmd.run()
                log(f"  -> done in {result.mean('wall'):.2f} seconds.")

    def run(self):
        self._run_initial_command()
        log(f"  running {self.benchmark_count} benchmarks on {len(self.commands)} commands")
        self._run_warmup()
        return self._run_benchmarks()

    def __str__(self):
        description = ""
        if self.description:
            description = f": {self.description}"
        return f"{self.name}{description}"


class BenchmarkResult:
    def __init__(self, benchmark: ComparisonBenchmark):
        self.benchmark = benchmark
        self.results = {}  # keys: command names, values: Result
        self.setup = {
            "name": benchmark.name,
            "description": benchmark.description,
            "pattern": benchmark.pattern,
            "iterations": benchmark.benchmark_count,
            "cached": benchmark.cache,
            "hardware": {
                "Computer": platform.node(),
                "CPU": get_cpu_name(),
            }
        }

    def __iadd__(self, res: XsResult):
        if res.name in self.results.keys():
            self.results[res.name] += res
        else:
            self.results[res.name] = res
        return self

    def get_dict(self) -> Dict:
        results = sorted(self.results.values())
        return {
            "setup": self.setup,
            "results": {
                res.name: {
                    "command": res.command, "data": res.get_data()}
                for res in results
            }
        }

    def write_json(self, path: str) -> None:
        with open(path, "w") as f:
            f.write(json.dumps(self.get_dict()))

    def get_setup(self):
        return self.setup

    def __str__(self) -> str:
        return f"Results for {self.benchmark}.\n"


def benchmark_chunk_size_xspp(pattern: str, iterations: int, cache: bool) -> ComparisonBenchmark:
    preprocess_commands = []
    commands = []
    for size in [512, 4096, 32768, 262144, 2097152, 16777216, 134217728, 1073741824]:
        meta_file = f"{DATA_FILE_PATH}-{size}.meta"
        preprocess_commands.append(Command(f"xspp -s {size}", ["xspp", DATA_FILE_PATH, "-m", meta_file, "-s", size]))
        commands.append(Command(f"xs -s {size} meta", [BENCHMARK_BUILD_XS, DATA_FILE_PATH, meta_file]))

    return ComparisonBenchmark(
        "Comparison: chunk size (preprocessed)",
        pattern,
        commands,
        DATA_FILE_PATH,
        preprocess_commands,
        benchmark_count=iterations,
        cache=cache
    )


def benchmark_chunk_size(pattern: str, iterations: int, cache: bool) -> ComparisonBenchmark:
    commands = []
    for size in [512, 4096, 32768, 262144, 2097152, 16777216, 134217728, 1073741824]:
        commands.append(Command(f"xs -s {size}", [BENCHMARK_BUILD_XS, DATA_FILE_PATH, "-s", size]))

    return ComparisonBenchmark(
        "Comparison: chunk size (plain)",
        pattern,
        commands,
        DATA_FILE_PATH,
        benchmark_count=iterations,
        cache=cache
    )


def benchmark_chunk_nl_mapping_data_xspp(pattern: str, iterations: int, cache: bool) -> ComparisonBenchmark:
    preprocess_commands = []
    commands = []
    for dist in [1, 500, 1000, 5000, 32000]:
        meta_file = f"{DATA_FILE_PATH}-nl-{dist}.meta"
        preprocess_commands.append(Command(f"xspp -d {dist}", ["xspp", DATA_FILE_PATH, "-m", meta_file, "-d", dist]))
        commands.append(Command(f"xs -s {dist}", [BENCHMARK_BUILD_XS, DATA_FILE_PATH, meta_file]))

    return ComparisonBenchmark(
        "Comparison: new line mapping data distance (preprocessed)",
        pattern,
        commands,
        DATA_FILE_PATH,
        benchmark_count=iterations,
        cache=cache
    )


def benchmark_compressions_xspp(pattern: str, iterations: int, cache: bool) -> ComparisonBenchmark:
    preprocess_commands = []
    commands = []
    compression_args = [
        ["-a", "zstd", "-l", "1"],
        ["-a", "zstd", "-l", "3"],
        ["-a", "zstd", "-l", "7"],
        ["-a", "lz4"],
        ["-a", "lz4", "--hc"],
        ["-a", "lz4", "--hc", "-l", "9"]
    ]
    for arg in compression_args:
        meta_file = f"{DATA_FILE_PATH}{''.join(arg)}.meta"
        preprocess_commands.append(Command(f"xspp {' '.join(arg)}", ["xspp", DATA_FILE_PATH, "-m", meta_file] + arg))
        commands.append(Command(f"xs {' '.join(arg)}", [BENCHMARK_BUILD_XS, DATA_FILE_PATH, meta_file]))

    return ComparisonBenchmark(
        "Comparison: new line mapping data distance (preprocessed)",
        pattern,
        commands,
        DATA_FILE_PATH,
        benchmark_count=iterations,
        cache=cache
    )


def compare_run(commands: List[Command], iterations: int, cache: bool) -> ComparisonBenchmark:
    return ComparisonBenchmark("Custom benchmark", "", commands, "", benchmark_count=iterations, cache=cache)


def log(*args, **kwargs):
    if not SILENT:
        print(80 * " ", end='\r', flush=True)
        print(*args, **kwargs)


def download():
    if not os.path.exists(DATA_DIR):
        os.makedirs(DATA_DIR)
    if not os.path.exists(DATA_FILE_PATH):
        log("Downloading data...")
        data = requests.get(DATA_DOWNLOAD_URL, stream=True)
        num_bytes = 0
        with open(DATA_FILE_PATH + ".gz", "wb") as f:
            for chunk in data.iter_content(chunk_size=16384):
                log(f"{num_bytes / 1000000:.2f} MiB written", end='\r', flush=True)
                f.write(chunk)
                num_bytes += len(chunk)
        log()


def decompress():
    if os.path.exists(DATA_FILE_PATH + ".gz"):
        log("Decompressing data...")
        p = subprocess.Popen(["gunzip", DATA_FILE_PATH + ".gz"])
        p.wait()
        log("Done")


def preprocess_file():
    if not os.path.exists(META_FILE_PATH):
        log("Preprocessing data...")
        subprocess.Popen(["xspp", DATA_FILE_PATH, "-m", META_FILE_PATH])
        log("Done.")


def read_commands_from_file(path: str):
    commands = []
    with open(path, "r") as f:
        for line in f.readlines():
            cmd = line.split(" ")
            commands.append(Command(cmd[0], cmd))
    return commands


def read_result_info_data(path: str):
    if not os.path.exists(path):
        return {}
    with open(path, "r") as f:
        return json.load(f)


def write_result_info_data(meta_data: Dict, path: str):
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
    parser.add_argument("--cache", action="store_true", help="Do not drop RAM caches between benchmark runs")
    parser.add_argument("--iterations", "-i", metavar="INTEGER", type=int, default=3,
                        help="Number of iterations per benchmark")
    parser.add_argument("--pattern", metavar="STRING", default="Sherlock",
                        help="The pattern that is searched during benchmarks")
    parser.add_argument("--commands", metavar="PATH",
                        help="File containing commands separated by '\\n' that will be run")
    parser.add_argument("--output", "-o", metavar="PATH", default="",
                        help="The directory where results are written to (default is printing in terminal)")
    parser.add_argument("--format", metavar="FORMAT", choices=OUTPUT_FORMAT, default="csv",
                        help="The format of the output")
    parser.add_argument("--filter", metavar="FILTER", default="", help="Filter benchmarks by name using regex")
    parser.add_argument("--input-file", metavar="PATH", help="Run benchmarks on the provided file")
    parser.add_argument("--silent", action="store_true", help="Do not output log messages")

    return parser.parse_args()


if __name__ == "__main__":
    pd.set_option("display.max_rows", None)
    pd.set_option("display.max_columns", None)
    pd.set_option("display.width", None)
    benchmarks = {

    }
    args = parse_args()
    SILENT = args.silent
    if args.list_benchmarks:
        print("The following benchmarks are available:")
        for name in benchmarks.keys():
            print(f" - {name}")
        exit(0)
    if args.commands:
        if os.path.exists(args.commands):
            compare_run(read_commands_from_file(args.commands), args.iterations, args.cache)
        else:
            print(f"{args.commands!r} is not a file or does not exist")
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
    preprocess_file()

    if args.output:
        if not os.path.exists(args.output):
            os.makedirs(args.output)
        OUTPUT_DIR = args.output

    RESULT_META_DATA = os.path.join(OUTPUT_DIR, "results.info.json")

    for name, bm_func in benchmarks.items():
        res = None
        if re.search(args.filter, name):
            log(f"Running {name}...")
            if "regex" in name:
                pattern = "She[r ]lock"
            else:
                pattern = args.pattern
            res = bm_func(pattern, args.iterations, args.cache).run()
        else:
            log(f"Skipping {name}...")
        if res:
            if args.output:
                result_info_data = read_result_info_data(RESULT_META_DATA)
                file_name = str(uuid.uuid4())
                while os.path.exists(os.path.join(OUTPUT_DIR, file_name)):
                    file_name = str(uuid.uuid4())
                output_file = os.path.join(OUTPUT_DIR, file_name)
                if args.format == "json":
                    output_file += ".json"
                    res.write_json(output_file)
                elif args.format == "csv":
                    output_file += ".csv"
                    res.write_csv(output_file)
                elif args.format == "markdown":
                    output_file += ".md"
                    res.write_markdown(output_file)
                result_info_data[file_name] = res.get_setup()
                result_info_data[file_name]["format"] = args.format
                write_result_info_data(result_info_data, RESULT_META_DATA)
            else:
                print(f"===== {res.get_setup()['name']} =====")
                print(f" CPU: {res.get_setup()['hardware']['CPU']}")
                print()
                print(res.get_df())
                print()
