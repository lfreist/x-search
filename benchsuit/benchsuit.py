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

DATA_DIR = "bench_data"
DATA_FILE_NAME = "en.sample.txt"
META_FILE_NAME = "en.sample.meta"
DATA_DOWNLOAD_URL = "https://object.pouta.csc.fi/OPUS-OpenSubtitles/v2016/mono/en.txt.gz"

SILENT = False


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
        return f"{self.mean('wall')}\t{self.mean('usr')}\t{self.mean('sys')}\t{self.mean('cpu')}\t"


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
        print(self.get_timed_cmd(path_to_timed))
        process = subprocess.Popen(self.get_timed_cmd(path_to_timed), stdout=subprocess.PIPE)
        out, err = process.communicate()
        out = str(out)
        wall, usr, sys = str(out).split('\t')
        return Result(self, wall, usr, sys)

    def __str__(self):
        return f"{self.name}: {' '.join(self.cmd)!r}"


class InvalidCommand(Exception):
    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return self.msg


class Benchmark:
    def __init__(self, name: str, pattern: str, commands: List[Command], description: str = "", warmup_cycles: int = 1,
                 benchmark_count: int = 10, drop_ram_cache: bool = True):
        self.name = name
        self.pattern = pattern
        self.commands = commands
        self.description = description
        self.warmup_cycles = warmup_cycles
        self.benchmark_count = benchmark_count
        self.drop_ram_cache = drop_ram_cache
        for cmd in self.commands:
            if not cmd.exists():
                raise InvalidCommand(f"{cmd} could not be found.")

    def _run_warmup(self):
        print("Running warmup...")
        for _ in range(self.warmup_cycles):
            for cmd in self.commands:
                if self.drop_ram_cache:
                    # Drop systems RAM cache precedent
                    DROP_RAM_CACHE()
                cmd.run()

    def _run_benchmarks(self):
        result = BenchmarkResult(self)
        for n in range(self.benchmark_count):
            for cmd in self.commands:
                log(f"{n}/{self.benchmark_count}: {cmd.name}", end='\r', flush=True)
                if self.drop_ram_cache:
                    DROP_RAM_CACHE()
                timings = cmd.run()
                result += cmd.run()
        return result

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

    def __iadd__(self, res: Result):
        if res.name in self.results:
            self.results[res.name] = res
        self.results[res.name] += res
        return self

    def get_df(self) -> pd.DataFrame:
        data = {
            "name": [], "description": [], "command": [],
            "min wall [s]": [], "max wall [s]": [], "mean wall [s]": [], "stdev wall [s]": [],
            "min usr [s]": [], "max usr [s]": [], "mean usr [s]": [], "stdev usr [s]": [],
            "min sys [s]": [], "max sys [s]": [], "mean sys [s]": [], "stdev sys [s]": [],
            "min cpu [s]": [], "max cpu [s]": [], "mean cpu [s]": [], "stdev cpu [s]": []
        }
        for name, result in self.results.items():
            data["name"].append(name)
            data["description"].append(result.description)
            data["command"].append(result.command)
            data["min wall [s]"].append(result.min("wall"))
            data["max wall [s]"].append(result.max("wall"))
            data["mean wall [s]"].append(result.mean("wall"))
            data["stdev wall [s]"].append(result.stdev("wall"))
            data["min usr [s]"].append(result.min("usr"))
            data["max usr [s]"].append(result.max("usr"))
            data["mean usr [s]"].append(result.mean("usr"))
            data["stdev usr [s]"].append(result.stdev("usr"))
            data["min sys [s]"].append(result.min("sys"))
            data["max sys [s]"].append(result.max("sys"))
            data["mean sys [s]"].append(result.mean("sys"))
            data["stdev sys [s]"].append(result.stdev("sys"))
            data["min cpu [s]"].append(result.min("cpu"))
            data["max cpu [s]"].append(result.max("cpu"))
            data["mean cpu [s]"].append(result.mean("cpu"))
            data["stdev cpu [s]"].append(result.stdev("cpu"))
        return pd.DataFrame(data)

    def get_dict(self) -> Dict:
        results = sorted(self.results.values())
        return {
            "name": self.benchmark.name,
            "description": self.benchmark.description,
            "pattern": self.benchmark.pattern,
            "results": {
                res.name: {
                    "command": res.command, "data": res.get_statistics}
                for res in results
            }
        }

    def write_json(self, path: str) -> None:
        with open(path, "w") as f:
            f.write(json.dumps(self.get_dict()))

    def write_csv(self, path: str, sep: str = ',', sort_by: str = "") -> None:
        df = self.get_df()
        try:
            df = df.sort_values(by=sort_by)
        except KeyError:
            pass
        df.to_csv(path, index=False, sep=sep)

    def write_markdown(self, path: str):
        self.get_df().to_markdown(path, index=False)

    def __str__(self) -> str:
        return f"Results for {self.benchmark}.\n"


def benchmark_subtitles_en_literal_disk(pattern: str) -> Benchmark:
    """
    Benchmark plain text search
    """
    data_path = os.path.join(DATA_DIR, DATA_FILE_NAME)
    meta_path = os.path.join(DATA_DIR, META_FILE_NAME)
    # patterns = ["short", "A little bit", "Sherlock Holmes", "jeez Rick"]
    commands = [
        Command("GNU grep", ["/usr/bin/grep", pattern, data_path]),
        Command("GNU grep -b", ["/usr/bin/grep", pattern, data_path, "-b"]),
        Command("GNU grep -n", ["/usr/bin/grep", pattern, data_path, "-n"]),
        Command("xs grep", ["build/xsgrep/grep", pattern, data_path]),
        Command("xs grep -b", ["build/xsgrep/grep", pattern, data_path, "-b"]),
        Command("xs grep -n", ["build/xsgrep/grep", pattern, data_path, "-n"]),
        Command("xs grep --no-map", ["build/xsgrep/grep", pattern, data_path, "--no-map"]),
        Command("xs grep -j", ["build/xsgrep/grep", pattern, data_path, "-j", "-1"]),
        Command("xs grep -b -j", ["build/xsgrep/grep", pattern, data_path, "-b", "-j", "-1"]),
        Command("xs grep -n -j", ["build/xsgrep/grep", pattern, data_path, "-n", "-j", "-1"]),
        Command("xs grep --no-map -j", ["build/xsgrep/grep", pattern, data_path, "--no-map", "-j", "-1"]),
    ]
    return Benchmark("literal", pattern, commands,
                     description=f"Comparison benchmark using pattern {pattern!r} reading data from disk",
                     drop_ram_cache=True)


def log(*args, **kwargs):
    if not SILENT:
        print(*args, **kwargs)


def set_up_data():
    if not os.path.exists(DATA_DIR):
        os.makedirs(DATA_DIR)
    data_path = os.path.join(DATA_DIR, DATA_FILE_NAME)
    if not os.path.exists(data_path):
        log("Downloading data...")
        data = requests.get(DATA_DOWNLOAD_URL, stream=True)
        num_bytes = 0
        with open(data_path, "wb") as f:
            for chunk in data.iter_content(chunk_size=16384):
                log(f"{num_bytes/1000000:.2f} MiB written", end='\r', flush=True)
                f.write(chunk)
                num_bytes += len(chunk)
        log()


if __name__ == "__main__":
    set_up_data()
    bm = benchmark_subtitles_en_literal_disk("Sherlock Holmes")
    result = bm.run()
    print(result.get_df())
