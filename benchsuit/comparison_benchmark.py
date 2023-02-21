#!/usr/bin/env python3

"""
Copyright 2023, Leon Freist
Author: Leon Freist <freist@informatik.uni-freiburg.de>
"""
import os
import shutil
import sys
import time
from typing import List, Dict, Tuple
import subprocess
import statistics
import pandas as pd
import json
import requests
import re
import platform
import argparse
import matplotlib.pyplot as plt
import matplotlib as mpl

DATA_DIR = "bench_data"
DATA_FILE = "en.sample.txt"
META_FILE = "en.sample.meta"
DATA_FILE_PATH = ""
META_FILE_PATH = ""
DATA_DOWNLOAD_URL = "https://object.pouta.csc.fi/OPUS-OpenSubtitles/v2016/mono/en.txt.gz"
OUTPUT_DIR = "bench_results"
RESULT_META_DATA = ""

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


class ComparisonResult:
    def __init__(self, command, wall: float, usr: float, _sys: float):
        self.name = command.name
        self.command = " ".join(command.cmd)
        self.wall_times = [wall]
        self.usr_times = [usr]
        self.sys_times = [_sys]

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
            if len(self.wall_times) < 2:
                return float("nan")
            return statistics.stdev(self.wall_times)
        elif measure_type.lower() == "usr":
            if len(self.usr_times) < 2:
                return float("nan")
            return statistics.stdev(self.usr_times)
        elif measure_type.lower() == "sys":
            if len(self.sys_times) < 2:
                return float("nan")
            return statistics.stdev(self.sys_times)
        elif measure_type.lower() == "cpu":
            if len(self.cpu()) < 2:
                return float("nan")
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
    def __init__(self, name: str, cmd: List[str] | str, is_baseline: bool = False):
        self.name = name
        self.cmd = cmd
        self.is_baseline = is_baseline

    def exists(self) -> bool:
        return shutil.which(self.get_binary_name()) is not None or type(self.cmd) == str

    def get_binary_name(self) -> str:
        return self.cmd[0]

    def get_timed_cmd(self) -> List[str] | str:
        if type(self.cmd) == str:
            return f"/usr/bin/time -f '%e\t%U\t%S' {self.cmd}"
        return ["/usr/bin/time", "-f", "%e\t%U\t%S"] + self.cmd

    def run(self) -> ComparisonResult:
        # We pipe stdout to a file instead of /dev/null because GNU grep stops after first match if output is written to
        #  /dev/null
        tmp_out_path = "/tmp/xs-benchmark.tmp"
        with open(tmp_out_path, "w") as tmp_out_file:
            out = subprocess.Popen(self.get_timed_cmd(), stderr=subprocess.PIPE, stdout=tmp_out_file,
                                   shell=(type(self.cmd) == str)).communicate()[1]
        os.remove("/tmp/xs-benchmark.tmp")
        out = out.decode()
        wall, usr, _sys = str(out).split('\t')
        try:
            return ComparisonResult(self, float(wall), float(usr), float(_sys))
        except ValueError:
            raise CommandFailedError(f"Error occurred while running {self.get_timed_cmd()}.")

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
                 initial_command: List[Command] = None, base_line_cmd: Command = None,
                 description: str = "",
                 benchmark_count: int = 3, cache: bool = False):
        self.name = name
        self.pattern = pattern
        self.commands = commands
        self.base_line_cmd = base_line_cmd
        if self.base_line_cmd:
            self.base_line_cmd.is_baseline = True
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
        bm_result = BenchmarkResult(self)
        for n in range(self.benchmark_count):
            if self.base_line_cmd:
                if not self.cache:
                    DROP_RAM_CACHE()
                bm_result += self.base_line_cmd.run()
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

    def __iadd__(self, res: ComparisonResult):
        if res.name in self.results.keys():
            self.results[res.name] += res
        else:
            self.results[res.name] = res
        return self

    def get_df(self) -> pd.DataFrame:
        data = {
            "name": [], "command": [], "wall speedup": [],
            "min wall [s]": [], "max wall [s]": [], "mean wall [s]": [], "stdev wall [s]": [],
            "min usr [s]": [], "max usr [s]": [], "mean usr [s]": [], "stdev usr [s]": [],
            "min sys [s]": [], "max sys [s]": [], "mean sys [s]": [], "stdev sys [s]": [],
            "min cpu [s]": [], "max cpu [s]": [], "mean cpu [s]": [], "stdev cpu [s]": []
        }
        baseline_wall = 0
        if self.benchmark.base_line_cmd:
            baseline_wall = self.results[self.benchmark.base_line_cmd.name].mean("wall")
        for name, res in self.results.items():
            data["name"].append(name)
            data["command"].append(res.command)
            try:
                data["wall speedup"].append(baseline_wall / res.mean("wall"))
            except ZeroDivisionError:
                data["wall speedup"].append(float("Nan"))
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

    def plot(self, path: str = ""):
        mpl.style.use("seaborn-v0_8")
        fig, axs = plt.subplots(1, 2, figsize=(8, 10))
        self._add_subplot(axs[0], "Wall Time [s]", self._get_plot_data("wall"))
        self._add_subplot(axs[1], "CPU Time [s]", self._get_plot_data("cpu"))
        plt.tight_layout()
        if path:
            plt.savefig(path, format="pdf")
        else:
            plt.show()

    def _add_subplot(self, axs, title: str, data: List[Tuple[str, float, float]]) -> None:
        data.sort(key=lambda v: v[0])  # sort by x label (cmd name) so that we get the same order in all subplots
        x = []
        y = []
        y_err = []
        for i in data:
            x.append(i[0])
            y.append(i[1])
            y_err.append(i[2])
        bar_list = axs.bar(x, y, color="gray")
        for bar in bar_list:
            if bar.get_height() == min(y):
                bar.set_color("red")
            if bar.get_height() == max(y):
                bar.set_color("red")
        # replace nan with 0
        y_err = list(map(lambda _x: _x if _x == _x else 0, y_err))
        if sum(y_err) != 0:
            axs.errorbar(x, y, yerr=y_err, fmt='o', color='r')
        axs.set_xticklabels(labels=x, rotation=90)
        axs.set_title(title)

    def _get_plot_data(self, measure_type: str) -> List[Tuple[str, float, float]]:
        ret = []
        for cmd, res in self.results.items():
            ret.append((cmd, res.mean(measure_type), res.stdev(measure_type)))
        return ret

    def __str__(self) -> str:
        return f"Results for {self.benchmark}.\n"


def compare_literal_byte_offset(pattern: str, iterations: int, cache: bool) -> ComparisonBenchmark:
    commands = [
        Command("GNU grep", ["grep", pattern, DATA_FILE_PATH, "-b"]),
        Command("xs grep", ["xs", pattern, DATA_FILE_PATH, "-b"]),
        Command("xs grep meta", ["xs", pattern, DATA_FILE_PATH, META_FILE_PATH, "-b"]),
        Command("xs grep --no-mmap", ["xs", pattern, DATA_FILE_PATH, "--no-mmap", "-b"]),
        Command("xs grep -j 1", ["xs", pattern, DATA_FILE_PATH, "-j", "1", "-b"]),
        Command("xs grep -j 1 meta", ["xs", pattern, DATA_FILE_PATH, META_FILE_PATH, "-j", "1", "-b"]),
        Command("xs grep --no-mmap -j 1", ["xs", pattern, DATA_FILE_PATH, "--no-mmap", "-j", "1", "-b"]),
        Command("ripgrep", ["rg", pattern, DATA_FILE_PATH, "-b"]),
        Command("ripgrep --no-mmap", ["rg", pattern, DATA_FILE_PATH, "--no-mmap", "-b"]),
        Command("ripgrep -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1", "-b"]),
        Command("ripgrep --no-mmap -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1", "--no-mmap", "-b"]),
    ]
    baseline = Command("cat", ["cat", DATA_FILE_PATH], True)
    return ComparisonBenchmark("literal byte offsets", pattern=pattern, commands=commands, file=DATA_FILE_PATH,
                               initial_command=[], base_line_cmd=baseline,
                               description=f"Case sensitive search of byte offsets",
                               benchmark_count=iterations,
                               cache=cache)


def compare_literal_line_number(pattern: str, iterations: int, cache: bool) -> ComparisonBenchmark:
    commands = [
        Command("GNU grep", ["grep", pattern, DATA_FILE_PATH, "-n"]),
        Command("xs grep", ["xs", pattern, DATA_FILE_PATH, "-n"]),
        Command("xs grep meta", ["xs", pattern, DATA_FILE_PATH, META_FILE_PATH, "-n"]),
        Command("xs grep --no-mmap", ["xs", pattern, DATA_FILE_PATH, "--no-mmap", "-n"]),
        Command("xs grep -j 1", ["xs", pattern, DATA_FILE_PATH, "-j", "1", "-n"]),
        Command("xs grep -j 1 meta", ["xs", pattern, DATA_FILE_PATH, META_FILE_PATH, "-j", "1", "-n"]),
        Command("xs grep --no-mmap -j 1", ["xs", pattern, DATA_FILE_PATH, "--no-mmap", "-j", "1", "-n"]),
        Command("ripgrep", ["rg", pattern, DATA_FILE_PATH, "-b"]),
        Command("ripgrep --no-mmap", ["rg", pattern, DATA_FILE_PATH, "--no-mmap", "-n"]),
        Command("ripgrep -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1", "-n"]),
        Command("ripgrep --no-mmap -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1", "--no-mmap", "-n"]),
    ]
    baseline = Command("cat", ["cat", DATA_FILE_PATH], True)
    return ComparisonBenchmark("literal line numbers", pattern=pattern, commands=commands, file=DATA_FILE_PATH,
                               initial_command=[], base_line_cmd=baseline,
                               description=f"Case sensitive search of line numbers",
                               benchmark_count=iterations,
                               cache=cache)


def compare_literal(pattern: str, iterations: int, cache: bool) -> ComparisonBenchmark:
    """
    Benchmark plain text search
    """
    commands = [
        Command("GNU grep", ["grep", pattern, DATA_FILE_PATH]),
        Command("xs grep", ["xs", pattern, DATA_FILE_PATH]),
        Command("xs grep meta", ["xs", pattern, DATA_FILE_PATH, META_FILE_PATH]),
        Command("xs grep --no-mmap", ["xs", pattern, DATA_FILE_PATH, "--no-mmap"]),
        Command("xs grep -j 1", ["xs", pattern, DATA_FILE_PATH, "-j", "1"]),
        Command("xs grep -j 1 meta", ["xs", pattern, DATA_FILE_PATH, META_FILE_PATH, "-j", "1"]),
        Command("xs grep --no-mmap -j 1", ["xs", pattern, DATA_FILE_PATH, "--no-mmap", "-j", "1"]),
        Command("ripgrep", ["rg", pattern, DATA_FILE_PATH]),
        Command("ripgrep --no-mmap", ["rg", pattern, DATA_FILE_PATH, "--no-mmap"]),
        Command("ripgrep -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1"]),
        Command("ripgrep --no-mmap -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1", "--no-mmap"]),
    ]
    baseline = Command("cat", ["cat", DATA_FILE_PATH], True)
    return ComparisonBenchmark("literal", pattern=pattern, commands=commands, file=DATA_FILE_PATH, initial_command=[],
                               base_line_cmd=baseline,
                               description=f"Case sensitive search of matching lines",
                               benchmark_count=iterations,
                               cache=cache)


def compare_literal_case_insensitive(pattern: str, iterations: int, cache: bool) -> ComparisonBenchmark:
    """
    Benchmark plain text search
    """
    commands = [
        Command("GNU grep", ["grep", pattern, DATA_FILE_PATH, "-i"]),
        Command("xs grep", ["grep", pattern, DATA_FILE_PATH]),
        Command("xs grep meta", ["grep", pattern, DATA_FILE_PATH, META_FILE_PATH]),
        Command("xs grep --no-mmap", ["xs", pattern, DATA_FILE_PATH, "--no-mmap", "-i"]),
        Command("xs grep -j 1", ["xs", pattern, DATA_FILE_PATH, "-j", "1"]),
        Command("xs grep -j 1 meta", ["xs", pattern, DATA_FILE_PATH, META_FILE_PATH, "-j", "1", "-i"]),
        Command("xs grep --no-mmap -j 1", ["xs", pattern, DATA_FILE_PATH, "--no-mmap", "-j", "1", "-i"]),
        Command("ripgrep", ["rg", pattern, DATA_FILE_PATH, "-i"]),
        Command("ripgrep --no-mmap", ["rg", pattern, DATA_FILE_PATH, "--no-mmap", "-i"]),
        Command("ripgrep -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1", "-i"]),
        Command("ripgrep --no-mmap -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1", "--no-mmap", "-i"]),
    ]
    baseline = Command("cat", ["cat", DATA_FILE_PATH], True)
    return ComparisonBenchmark("literal case insensitive", pattern=pattern, commands=commands, file=DATA_FILE_PATH,
                               initial_command=[], base_line_cmd=baseline,
                               description=f"Case insensitive search of matching lines.",
                               benchmark_count=iterations,
                               cache=cache)


def compare_regex(pattern: str, iterations: int, cache: bool) -> ComparisonBenchmark:
    commands = [
        Command("GNU grep", ["grep", pattern, DATA_FILE_PATH]),
        Command("xs grep", ["xs", pattern, DATA_FILE_PATH]),
        Command("xs grep meta", ["xs", pattern, DATA_FILE_PATH, META_FILE_PATH]),
        Command("xs grep --no-mmap", ["xs", pattern, DATA_FILE_PATH, "--no-mmap"]),
        Command("xs grep -j 1", ["xs", pattern, DATA_FILE_PATH, "-j", "1"]),
        Command("xs grep -j 1 meta", ["xs", pattern, DATA_FILE_PATH, META_FILE_PATH, "-j", "1"]),
        Command("xs grep --no-mmap -j 1", ["xs", pattern, DATA_FILE_PATH, "--no-mmap", "-j", "1"]),
        Command("ripgrep", ["rg", pattern, DATA_FILE_PATH]),
        Command("ripgrep --no-mmap", ["rg", pattern, DATA_FILE_PATH, "--no-mmap"]),
        Command("ripgrep -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1"]),
        Command("ripgrep --no-mmap -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1", "--no-mmap"]),
    ]
    baseline = Command("cat", ["cat", DATA_FILE_PATH], True)
    return ComparisonBenchmark("literal", pattern=pattern, commands=commands, file=DATA_FILE_PATH, initial_command=[],
                               base_line_cmd=baseline,
                               description=f"Case sensitive search of matching lines",
                               benchmark_count=iterations,
                               cache=cache)


def compare_regex_line_number(pattern: str, iterations: int, cache: bool) -> ComparisonBenchmark:
    commands = [
        Command("GNU grep", ["grep", pattern, DATA_FILE_PATH, "-n"]),
        Command("xs grep", ["xs", pattern, DATA_FILE_PATH, "-n"]),
        Command("xs grep meta", ["xs", pattern, DATA_FILE_PATH, META_FILE_PATH, "-n"]),
        Command("xs grep --no-mmap", ["xs", pattern, DATA_FILE_PATH, "--no-mmap", "-n"]),
        Command("xs grep -j 1", ["xs", pattern, DATA_FILE_PATH, "-j", "1", "-n"]),
        Command("xs grep -j 1 meta", ["xs", pattern, DATA_FILE_PATH, META_FILE_PATH, "-j", "1", "-n"]),
        Command("xs grep --no-mmap -j 1", ["xs", pattern, DATA_FILE_PATH, "--no-mmap", "-j", "1", "-n"]),
        Command("ripgrep", ["rg", pattern, DATA_FILE_PATH, "-b"]),
        Command("ripgrep --no-mmap", ["rg", pattern, DATA_FILE_PATH, "--no-mmap", "-n"]),
        Command("ripgrep -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1", "-n"]),
        Command("ripgrep --no-mmap -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1", "--no-mmap", "-n"]),
    ]
    baseline = Command("cat", ["cat", DATA_FILE_PATH], True)
    return ComparisonBenchmark("literal line numbers", pattern=pattern, commands=commands, file=DATA_FILE_PATH,
                               initial_command=[], base_line_cmd=baseline,
                               description=f"Case sensitive search of line numbers",
                               benchmark_count=iterations,
                               cache=cache)


def compare_regex_byte_offset(pattern: str, iterations: int, cache: bool) -> ComparisonBenchmark:
    commands = [
        Command("GNU grep", ["grep", pattern, DATA_FILE_PATH, "-b"]),
        Command("xs grep", ["xs", pattern, DATA_FILE_PATH, "-b"]),
        Command("xs grep meta", ["xs", pattern, DATA_FILE_PATH, META_FILE_PATH, "-b"]),
        Command("xs grep --no-mmap", ["xs", pattern, DATA_FILE_PATH, "--no-mmap", "-b"]),
        Command("xs grep -j 1", ["xs", pattern, DATA_FILE_PATH, "-j", "1", "-b"]),
        Command("xs grep -j 1 meta", ["xs", pattern, DATA_FILE_PATH, META_FILE_PATH, "-j", "1", "-b"]),
        Command("xs grep --no-mmap -j 1", ["xs", pattern, DATA_FILE_PATH, "--no-mmap", "-j", "1", "-b"]),
        Command("ripgrep", ["rg", pattern, DATA_FILE_PATH, "-b"]),
        Command("ripgrep --no-mmap", ["rg", pattern, DATA_FILE_PATH, "--no-mmap", "-b"]),
        Command("ripgrep -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1", "-b"]),
        Command("ripgrep --no-mmap -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1", "--no-mmap", "-b"]),
    ]
    baseline = Command("cat", ["cat", DATA_FILE_PATH], True)
    return ComparisonBenchmark("literal byte offsets", pattern=pattern, commands=commands, file=DATA_FILE_PATH,
                               initial_command=[], base_line_cmd=baseline,
                               description=f"Case sensitive search of byte offsets",
                               benchmark_count=iterations,
                               cache=cache)


def compare_regex_case_insensitive(pattern: str, iterations: int, cache: bool) -> ComparisonBenchmark:
    commands = [
        Command("GNU grep", ["grep", pattern, DATA_FILE_PATH, "-i"]),
        Command("xs grep", ["grep", pattern, DATA_FILE_PATH, "-i"]),
        Command("xs grep meta", ["grep", pattern, DATA_FILE_PATH, META_FILE_PATH, "-i"]),
        Command("xs grep --no-mmap", ["xs", pattern, DATA_FILE_PATH, "--no-mmap", "-i"]),
        Command("xs grep -j 1", ["xs", pattern, DATA_FILE_PATH, "-j", "1", "-i"]),
        Command("xs grep -j 1 meta", ["xs", pattern, DATA_FILE_PATH, META_FILE_PATH, "-j", "1", "-i"]),
        Command("xs grep --no-mmap -j 1", ["xs", pattern, DATA_FILE_PATH, "--no-mmap", "-j", "1", "-i"]),
        Command("ripgrep", ["rg", pattern, DATA_FILE_PATH, "-i"]),
        Command("ripgrep --no-mmap", ["rg", pattern, DATA_FILE_PATH, "--no-mmap", "-i"]),
        Command("ripgrep -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1", "-i"]),
        Command("ripgrep --no-mmap -j 1", ["rg", pattern, DATA_FILE_PATH, "-j", "1", "--no-mmap", "-i"]),
    ]
    baseline = Command("cat", ["cat", DATA_FILE_PATH], True)
    return ComparisonBenchmark("literal case insensitive", pattern=pattern, commands=commands, file=DATA_FILE_PATH,
                               initial_command=[], base_line_cmd=baseline,
                               description=f"Case insensitive search of matching lines.",
                               benchmark_count=iterations,
                               cache=cache)


def compare_zstd_input(pattern: str, iterations: int, cache: bool) -> ComparisonBenchmark:
    zstd = Command("zstd", ["zstd", DATA_FILE_PATH, "-o", f"{DATA_FILE_PATH}.zst", "-f", "-q"])
    xs_zstd = Command("xspp zstd",
                      ["xspp", DATA_FILE_PATH, "-o", f"{DATA_FILE_PATH}.xszst", "-m", f"{DATA_FILE_PATH}.xszst.meta",
                       "-a",
                       "zst"])
    zstdcat = Command("zstdcat", ["zstdcat", f"{DATA_FILE_PATH}.zst"], True)

    commands = [
        Command("zstdcat | grep", f"zstdcat {DATA_FILE_PATH}.zst | grep {pattern}"),
        Command("zstdcat | rg", f"zstdcat {DATA_FILE_PATH}.zst | rg {pattern}"),
        Command("zstdcat | xs", f"zstdcat {DATA_FILE_PATH}.zst | xs {pattern}"),
        Command("xspp -a zstd -> xs", ["xs", pattern, f"{DATA_FILE_PATH}.xszst", f"{DATA_FILE_PATH}.xszst.meta"]),
    ]

    return ComparisonBenchmark(
        "ZStandard compressed file input",
        pattern=pattern,
        commands=commands,
        file=DATA_FILE_PATH,
        initial_command=[zstd, xs_zstd],
        base_line_cmd=zstdcat,
        benchmark_count=iterations,
        cache=cache
    )


def compare_lz4_input(pattern: str, iterations: int, cache: bool) -> ComparisonBenchmark:
    lz4 = Command("lz4 HC", ["lz4", DATA_FILE_PATH, f"{DATA_FILE_PATH}.lz4", "-9", "-f", "-q"])
    xs_lz4 = Command("xspp lz4",
                     ["xspp", DATA_FILE_PATH, f"{DATA_FILE_PATH}.xslz4", "-m", f"{DATA_FILE_PATH}.xslz4.meta", "-a",
                      "lz4", "--hc"])
    lz4cat = Command("lz4cat", ["zstdcat", f"{DATA_FILE_PATH}.lz4"], True)

    commands = [
        Command("lz4cat | grep", f"lz4cat {DATA_FILE_PATH}.lz4 | grep {pattern}"),
        Command("lz4cat | rg", f"lz4cat {DATA_FILE_PATH}.lz4 | rg {pattern}"),
        Command("lz4cat | xs", f"lz4cat {DATA_FILE_PATH}.lz4 | xs {pattern}"),
        Command("xspp -a lz4 -> xs", ["xs", pattern, f"{DATA_FILE_PATH}.xslz4", f"{DATA_FILE_PATH}.xslz4.meta"]),
    ]

    return ComparisonBenchmark(
        "ZStandard compressed file input",
        pattern=pattern,
        commands=commands,
        file=DATA_FILE_PATH,
        initial_command=[lz4, xs_lz4],
        base_line_cmd=lz4cat,
        benchmark_count=iterations,
        cache=cache
    )


def compare_run(commands: List[Command], iterations: int, cache: bool) -> ComparisonBenchmark:
    return ComparisonBenchmark("Custom benchmark", pattern="", commands=commands, file="", benchmark_count=iterations,
                               cache=cache)


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
        "comparison: literal, plane": compare_literal,
        "comparison: literal, line numbers": compare_literal_line_number,
        "comparison: literal, byte offset": compare_literal_byte_offset,
        "comparison: literal, case insensitive": compare_literal_case_insensitive,
        "comparison: regex": compare_regex,
        "comparison: regex, line numbers": compare_regex_line_number,
        "comparison: regex, byte offset": compare_regex_byte_offset,
        "comparison: regex, case insensitive": compare_regex_case_insensitive,
        "comparison: zstd compressed input file": compare_zstd_input,
        "comparison: lz4 compressed input file": compare_lz4_input,
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

    if not args.output:
        print(f" CPU: {get_cpu_name()}")

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
                tmp_id = 0
                file_name = name.replace(" ", "_") + str(tmp_id)
                tmp_file_name = file_name + "." + args.format
                while os.path.exists(os.path.join(OUTPUT_DIR, tmp_file_name)):
                    tmp_id += 1
                    file_name[-1] = str(tmp_id)
                    tmp_file_name = file_name + "." + args.format
                output_file = os.path.join(OUTPUT_DIR, file_name)
                if args.format == "json":
                    res.write_json(output_file + ".json")
                elif args.format == "csv":
                    res.write_csv(output_file + ".csv")
                elif args.format == "markdown":
                    res.write_markdown(output_file + ".md")
                result_info_data[file_name] = res.get_setup()
                result_info_data[file_name]["format"] = args.format
                result_info_data[file_name]["plot"] = output_file + ".pdf"
                write_result_info_data(result_info_data, RESULT_META_DATA)
                res.plot(output_file + ".pdf")
            else:
                res.plot()
                print(f"===== {res.get_setup()['name']} =====")
                print()
                print(res.get_df())
                print()