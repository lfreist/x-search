#!/usr/bin/env python3

"""
Copyright 2023, Leon Freist
Author: Leon Freist <freist@informatik.uni-freiburg.de>
"""
import os
import shutil
import time
from typing import List, Dict
import subprocess
import statistics
import pandas as pd
import json
import requests
import psutil
import re
import platform
import argparse

DATA_DIR = "bench_data"
DATA_FILE_NAME = "en.sample.txt"
META_FILE_NAME = "en.sample.meta"
DATA_DOWNLOAD_URL = "https://object.pouta.csc.fi/OPUS-OpenSubtitles/v2016/mono/en.txt.gz"

SILENT = False


def get_cpu_name():
    command = ["cat", "/proc/cpuinfo"]
    all_info = subprocess.Popen(command, stdout=subprocess.PIPE).communicate()[0].decode().strip()
    for line in all_info.split("\n"):
        if "model name" in line:
            return re.sub(".*model name.*:", "", line, 1).strip()


def get_disk(file_path: str) -> str:
    try:
        device_id = os.stat(file_path).st_dev
        # Get the partition that the file is on
        partition = psutil.disk_partitions(all=True)
        return [p for p in partition if os.stat(p.mountpoint).st_dev == device_id][0].device
    except PermissionError:
        return ""


def DROP_RAM_CACHE():
    subprocess.run("sync")
    with open("/proc/sys/vm/drop_caches", "w") as f:
        f.write("1")
    # give the system some time to get up...
    time.sleep(1)


class Result:
    def __init__(self, command, wall: float, usr: float, sys: float):
        self.name = command.name
        self.command = " ".join(command.cmd)
        self.wall_times = [wall]
        self.usr_times = [usr]
        self.sys_times = [sys]

    def __iadd__(self, other):
        assert self.name == other.name and self.command == other.command
        self.wall_times += other.wall_times
        self.usr_times += other.usr_times
        self.sys_times += other.sys_times
        return self

    def wall(self):
        return self.wall_times

    def usr(self):
        return self.usr_times

    def sys(self):
        return self.sys_times

    def cpu(self):
        return [self.usr_times[i] + self.sys_times[i] for i in range(len(self.usr_times))]

    def min(self, measure_type: str) -> float:
        if measure_type.lower() == "wall":
            return min(self.wall_times)
        elif measure_type.lower() == "usr":
            return min(self.usr_times)
        elif measure_type.lower() == "sys":
            return min(self.sys_times)
        elif measure_type.lower() == "cpu":
            return min(self.cpu())
        raise IndexError(f"{measure_type} unknown. Use one of ('wall', 'usr', 'sys').")

    def max(self, measure_type: str) -> float:
        if measure_type.lower() == "wall":
            return max(self.wall_times)
        elif measure_type.lower() == "usr":
            return max(self.usr_times)
        elif measure_type.lower() == "sys":
            return max(self.sys_times)
        elif measure_type.lower() == "cpu":
            return max(self.cpu())
        raise IndexError(f"{measure_type} unknown. Use one of ('wall', 'usr', 'sys').")

    def mean(self, measure_type: str) -> float:
        if measure_type.lower() == "wall":
            return statistics.mean(self.wall_times)
        elif measure_type.lower() == "usr":
            return statistics.mean(self.usr_times)
        elif measure_type.lower() == "sys":
            return statistics.mean(self.sys_times)
        elif measure_type.lower() == "cpu":
            return statistics.mean(self.cpu())
        raise IndexError(f"{measure_type} unknown. Use one of ('wall', 'usr', 'sys').")

    def stdev(self, measure_type: str) -> float:
        if measure_type.lower() == "wall":
            return statistics.stdev(self.wall_times)
        elif measure_type.lower() == "usr":
            return statistics.stdev(self.usr_times)
        elif measure_type.lower() == "sys":
            return statistics.stdev(self.sys_times)
        elif measure_type.lower() == "cpu":
            return statistics.stdev(self.cpu())
        raise IndexError(f"{measure_type} unknown. Use one of ('wall', 'usr', 'sys').")

    def get_statistics(self) -> Dict:
        return {
            "wall": {
                "min": self.min("wall"),
                "max": self.max("wall"),
                "mean": self.mean("wall"),
                "stdev": self.stdev("wall")
            },
            "usr": {
                "min": self.min("usr"),
                "max": self.max("usr"),
                "mean": self.mean("usr"),
                "stdev": self.stdev("usr")
            },
            "sys": {
                "min": self.min("sys"),
                "max": self.max("sys"),
                "mean": self.mean("sys"),
                "stdev": self.stdev("sys")
            },
            "cpu": {
                "min": self.min("cpu"),
                "max": self.max("cpu"),
                "mean": self.mean("cpu"),
                "stdev": self.stdev("cpu")
            }
        }

    def __gt__(self, other):
        if self.min("all") > other.min("wall"):
            return True
        elif self.min("wall") == other.min("wall"):
            return self.min("cpu") > other.min("cpu")
        return False

    def __str__(self):
        return f"{self.mean('wall')}\t{self.mean('usr')}\t{self.mean('sys')}\t{self.mean('cpu')}"


class Command:
    def __init__(self, name: str, cmd: List[str]):
        self.name = name
        self.cmd = cmd

    def exists(self) -> bool:
        return shutil.which(self.get_binary_name()) is not None

    def get_binary_name(self) -> str:
        return self.cmd[0]

    def get_timed_cmd(self, timed: str) -> List[str]:
        return [timed] + self.cmd

    def run(self, path_to_timed: str = "./benchsuit/timed") -> Result:
        assert shutil.which(path_to_timed) is not None
        out = subprocess.Popen(self.get_timed_cmd(path_to_timed), stdout=subprocess.PIPE).communicate()[0]
        out = out.decode()
        wall, usr, sys = str(out).split('\t')
        try:
            return Result(self, float(wall), float(usr), float(sys))
        except ValueError:
            raise CommandFailedError(f"Error occurred while running {self.get_timed_cmd(path_to_timed)}.")

    def __str__(self):
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


class Benchmark:
    def __init__(self, name: str, pattern: str, commands: List[Command], file: str, description: str = "",
                 benchmark_count: int = 10, drop_ram_cache: bool = True):
        self.name = name
        self.pattern = pattern
        self.commands = commands
        self.description = description
        self.file = file
        self.benchmark_count = benchmark_count
        self.drop_ram_cache = drop_ram_cache
        for cmd in self.commands:
            if not cmd.exists():
                raise InvalidCommandError(f"{cmd} could not be found.")

    def _run_warmup(self):
        if not self.drop_ram_cache:
            log("Running warmup...")
            counter = 0
            for cmd in self.commands:
                log(f"{counter}/{len(self.commands)}", end='\r', flush=True)
                cmd.run()
                counter += 1
            log("Warmup done.")

    def _run_benchmarks(self):
        bm_result = BenchmarkResult(self)
        log("Running Benchmarks...")
        for n in range(self.benchmark_count):
            for cmd in self.commands:
                log(f"{n}/{self.benchmark_count}: {cmd.name}", end='\r', flush=True)
                if self.drop_ram_cache:
                    DROP_RAM_CACHE()
                res = cmd.run()
                bm_result += res
        log("done")
        return bm_result

    def run(self):
        log(f"running {self.benchmark_count} benchmarks on {len(self.commands)} commands")
        self._run_warmup()
        return self._run_benchmarks()

    def __str__(self):
        description = ""
        if self.description:
            description = f": {self.description}"
        return f"{self.name}{description}"


class BenchmarkResult:
    def __init__(self, benchmark: Benchmark):
        self.benchmark = benchmark
        self.results = {}  # keys: command names, values: Result
        self.setup = {
            "name": benchmark.name,
            "description": benchmark.description,
            "pattern": benchmark.pattern,
            "iterations": benchmark.benchmark_count,
            "from disk": benchmark.drop_ram_cache,
            "hardware": {
                "Computer": platform.node(),
                "CPU": get_cpu_name(),
                "disk": get_disk(os.path.abspath(self.benchmark.file))
            }
        }

    def __iadd__(self, res: Result):
        if res.name in self.results.keys():
            self.results[res.name] += res
        else:
            self.results[res.name] = res
        return self

    def get_df(self) -> pd.DataFrame:
        data = {
            "name": [], "command": [],
            "min wall [s]": [], "max wall [s]": [], "mean wall [s]": [], "stdev wall [s]": [],
            "min usr [s]": [], "max usr [s]": [], "mean usr [s]": [], "stdev usr [s]": [],
            "min sys [s]": [], "max sys [s]": [], "mean sys [s]": [], "stdev sys [s]": [],
            "min cpu [s]": [], "max cpu [s]": [], "mean cpu [s]": [], "stdev cpu [s]": []
        }
        for name, res in self.results.items():
            data["name"].append(name)
            data["command"].append(res.command)
            data["min wall [s]"].append(res.min("wall"))
            data["max wall [s]"].append(res.max("wall"))
            data["mean wall [s]"].append(res.mean("wall"))
            data["stdev wall [s]"].append(res.stdev("wall"))
            data["min usr [s]"].append(res.min("usr"))
            data["max usr [s]"].append(res.max("usr"))
            data["mean usr [s]"].append(res.mean("usr"))
            data["stdev usr [s]"].append(res.stdev("usr"))
            data["min sys [s]"].append(res.min("sys"))
            data["max sys [s]"].append(res.max("sys"))
            data["mean sys [s]"].append(res.mean("sys"))
            data["stdev sys [s]"].append(res.stdev("sys"))
            data["min cpu [s]"].append(res.min("cpu"))
            data["max cpu [s]"].append(res.max("cpu"))
            data["mean cpu [s]"].append(res.mean("cpu"))
            data["stdev cpu [s]"].append(res.stdev("cpu"))
        return pd.DataFrame(data)

    def get_dict(self) -> Dict:
        results = sorted(self.results.values())
        return {
            "setup": self.setup,
            "results": {
                res.name: {
                    "command": res.command, "data": res.get_statistics}
                for res in results
            }
        }

    def write_json(self, path: str) -> None:
        with open(path, "w") as f:
            f.write(json.dumps(self.get_dict()))

    def write_csv(self, path: str, sep: str = ',', sort_by: str = "mean wall [s]") -> None:
        df = self.get_df()
        try:
            df = df.sort_values(by=sort_by)
        except KeyError:
            pass
        df.to_csv(path, index=False, sep=sep)

    def write_markdown(self, path: str):
        self.get_df().to_markdown(path, index=False)

    def get_setup(self):
        return self.setup

    def __str__(self) -> str:
        return f"Results for {self.benchmark}.\n"


def benchmark_subtitles_en_literal_byte(pattern: str, iterations: int = 5,
                                        drop_ram_cache: bool = True) -> Benchmark:
    data_path = os.path.join(DATA_DIR, DATA_FILE_NAME)
    meta_data_path = os.path.join(DATA_DIR, META_FILE_NAME)
    commands = [
        Command("GNU grep", ["grep", pattern, data_path, "-b"]),
        Command("xs grep", ["xsgrep", pattern, data_path, "-b"]),
        Command("xs grep meta", ["xsgrep", pattern, data_path, meta_data_path, "-b"]),
        Command("xs grep --no-mmap", ["xsgrep", pattern, data_path, "--no-mmap", "-b"]),
        Command("xs grep -j", ["xsgrep", pattern, data_path, "-j", "-b"]),
        Command("xs grep -j meta", ["xsgrep", pattern, data_path, meta_data_path, "-j", "-b"]),
        Command("xs grep --no-mmap -j", ["xsgrep", pattern, data_path, "--no-mmap", "-j", "-b"]),
        Command("ripgrep", ["rg", pattern, data_path, "-b"]),
        Command("ripgrep --no-mmap", ["rg", pattern, data_path, "--no-mmap", "-b"]),
        Command("ripgrep -j 1", ["rg", pattern, data_path, "-j", "1", "-b"]),
        Command("ripgrep --no-mmap -j 1", ["rg", pattern, data_path, "-j", "1", "--no-mmap", "-b"]),
    ]
    return Benchmark("literal", pattern, commands, data_path,
                     description=f"Case sensitive search of byte offsets",
                     benchmark_count=iterations,
                     drop_ram_cache=drop_ram_cache)


def benchmark_subtitles_en_literal_line_number(pattern: str, iterations: int = 5,
                                               drop_ram_cache: bool = True) -> Benchmark:
    data_path = os.path.join(DATA_DIR, DATA_FILE_NAME)
    meta_data_path = os.path.join(DATA_DIR, META_FILE_NAME)
    commands = [
        Command("GNU grep", ["grep", pattern, data_path, "-n"]),
        Command("xs grep", ["xsgrep", pattern, data_path, "-n"]),
        Command("xs grep meta", ["xsgrep", pattern, data_path, meta_data_path, "-n"]),
        Command("xs grep --no-mmap", ["xsgrep", pattern, data_path, "--no-mmap", "-n"]),
        Command("xs grep -j", ["xsgrep", pattern, data_path, "-j", "-n"]),
        Command("xs grep -j meta", ["xsgrep", pattern, data_path, meta_data_path, "-j", "-n"]),
        Command("xs grep --no-mmap -j", ["xsgrep", pattern, data_path, "--no-mmap", "-j", "-n"]),
        Command("ripgrep", ["rg", pattern, data_path, "-b"]),
        Command("ripgrep --no-mmap", ["rg", pattern, data_path, "--no-mmap", "-n"]),
        Command("ripgrep -j 1", ["rg", pattern, data_path, "-j", "1", "-n"]),
        Command("ripgrep --no-mmap -j 1", ["rg", pattern, data_path, "-j", "1", "--no-mmap", "-n"]),
    ]
    return Benchmark("literal", pattern, commands, data_path,
                     description=f"Case sensitive search of line numbers",
                     benchmark_count=iterations,
                     drop_ram_cache=drop_ram_cache)


def benchmark_subtitles_en_literal(pattern: str, iterations: int = 5, drop_ram_cache: bool = True) -> Benchmark:
    """
    Benchmark plain text search
    """
    data_path = os.path.join(DATA_DIR, DATA_FILE_NAME)
    meta_data_path = os.path.join(DATA_DIR, META_FILE_NAME)
    commands = [
        Command("GNU grep", ["grep", pattern, data_path]),
        Command("xs grep", ["xsgrep", pattern, data_path]),
        Command("xs grep meta", ["xsgrep", pattern, data_path, meta_data_path]),
        Command("xs grep --no-mmap", ["xsgrep", pattern, data_path, "--no-mmap"]),
        Command("xs grep -j", ["xsgrep", pattern, data_path, "-j"]),
        Command("xs grep -j meta", ["xsgrep", pattern, data_path, meta_data_path, "-j"]),
        Command("xs grep --no-mmap -j", ["xsgrep", pattern, data_path, "--no-mmap", "-j"]),
        Command("ripgrep", ["rg", pattern, data_path]),
        Command("ripgrep --no-mmap", ["rg", pattern, data_path, "--no-mmap"]),
        Command("ripgrep -j 1", ["rg", pattern, data_path, "-j", "1"]),
        Command("ripgrep --no-mmap -j 1", ["rg", pattern, data_path, "-j", "1", "--no-mmap"]),
    ]
    return Benchmark("literal", pattern, commands, data_path,
                     description=f"Case sensitive search of matching lines",
                     benchmark_count=iterations,
                     drop_ram_cache=drop_ram_cache)


def benchmark_subtitles_en_literal_casi(pattern: str, iterations: int = 5, drop_ram_cache: bool = True) -> Benchmark:
    """
    Benchmark plain text search
    """
    data_path = os.path.join(DATA_DIR, DATA_FILE_NAME)
    meta_data_path = os.path.join(DATA_DIR, META_FILE_NAME)
    commands = [
        Command("GNU grep", ["grep", pattern, data_path, "-i"]),
        Command("xs grep", ["grep", pattern, data_path]),
        Command("xs grep meta", ["grep", pattern, data_path, meta_data_path]),
        Command("xs grep --no-mmap", ["xsgrep", pattern, data_path, "--no-mmap", "-i"]),
        Command("xs grep -j", ["xsgrep", pattern, data_path, "-j"]),
        Command("xs grep -j meta", ["xsgrep", pattern, data_path, meta_data_path, "-j", "-i"]),
        Command("xs grep --no-mmap -j", ["xsgrep", pattern, data_path, "--no-mmap", "-j", "-i"]),
        Command("ripgrep", ["rg", pattern, data_path, "-i"]),
        Command("ripgrep --no-mmap", ["rg", pattern, data_path, "--no-mmap", "-i"]),
        Command("ripgrep -j 1", ["rg", pattern, data_path, "-j", "1", "-i"]),
        Command("ripgrep --no-mmap -j 1", ["rg", pattern, data_path, "-j", "1", "--no-mmap", "-i"]),
    ]
    return Benchmark("literal", pattern, commands, data_path,
                     description=f"Case insensitive search of matching lines.",
                     benchmark_count=iterations,
                     drop_ram_cache=drop_ram_cache)


def log(*args, **kwargs):
    if not SILENT:
        print(80 * " ", end='\r', flush=True)
        print(*args, **kwargs)


def set_up_data():
    if not os.path.exists(DATA_DIR):
        os.makedirs(DATA_DIR)
    data_path = os.path.join(DATA_DIR, DATA_FILE_NAME)
    data_meta_path = os.path.join(DATA_DIR, META_FILE_NAME)
    if not os.path.exists(data_path):
        log("Downloading data...")
        data = requests.get(DATA_DOWNLOAD_URL, stream=True)
        num_bytes = 0
        with open(data_path, "wb") as f:
            for chunk in data.iter_content(chunk_size=16384):
                log(f"{num_bytes / 1000000:.2f} MiB written", end='\r', flush=True)
                f.write(chunk)
                num_bytes += len(chunk)
        log()
    if not os.path.exists(data_meta_path):
        log("Preprocessing data...")
        subprocess.Popen(["xspp", data_path, "-m", data_meta_path])
        log("Done.")


if __name__ == "__main__":
    set_up_data()
    bm = benchmark_subtitles_en_literal("Sherlock", 3, False)
    result = bm.run()
    result.write_csv("benchmark.csv")
