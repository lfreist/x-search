#!/usr/bin/env/python3
"""
Copyright 2023, Leon Freist
Author: Leon Freist <freist@informatik.uni-freiburg.de>

Written in the scope of Leon Freists Bachelor Thesis "x-search - A C++ Library for fast external String Search"
available at "https://github.com/lfreist/x-search"

benchsuit is a collection of partially abstract classes that can be used to compare and benchmark executables on *nix
systems.
It includes
 - CommandResult: the benchmark result of a specific command
 - Command: a class representing a command-line command with a user chosen name identifier
 - BenchmarkResult: a collection of CommandResults representing the full benchmark result
 - Benchmark: a class representing a Benchmark. Benchmark.run() starts the benchmark and returns a BenchmarkResult
"""

import json
import platform
import shutil
from abc import ABC, abstractmethod
from typing import List
import subprocess
import os
import time
import re

# ===== Helper functions ===============================================================================================

SILENT: bool = False


def log(*args, **kwargs):
    """
    Forward arguments to print() if SILENT is False.
    :param args:
    :param kwargs:
    :return:
    """
    if not SILENT:
        print(80 * " ", end='\r', flush=True)
        print(*args, **kwargs)


def get_cpu_name() -> str:
    """
    Read the CPUs name out of /proc/cpuinfo on unix systems and return it
    :return:
    """
    with open("/proc/cpuinfo", "r") as f:
        for line in f.readlines():
            if "model name" in line:
                return re.sub(".*model name.*:", "", line, 1).strip()


def has_root_privileges() -> bool:
    """
    Return boolean indicating whether the user running this script has root privileges
    :return:
    """
    return os.geteuid() == 0 or os.system("sudo -n true 2>/dev/null") == 0


def drop_ram_cache() -> None:
    """
    Drop systems RAM cache (only unix systems) or raise PermissionError if access is denied
    :return:
    """
    if has_root_privileges():
        subprocess.run("sync")
        with open("/proc/sys/vm/drop_caches", "w") as f:
            f.write("3")
        log("RAM caches dropped!")
        # give the system some time to recover...
        time.sleep(1)
    else:
        raise PermissionError("Cannot drop RAM caches as non root")


# ===== Base Classes ===================================================================================================
class CommandResult(ABC):
    """
    Abstract base class of the result of the runs of a specific commands.
    CommandResults of the same command can be merged using __iadd__ (+= operator)
    """

    def __init__(self, command):
        self.command = command

    @abstractmethod
    def __iadd__(self, other):
        pass

    @abstractmethod
    def __len__(self):
        pass

    @abstractmethod
    def __getitem__(self, item):
        pass

    @abstractmethod
    def __gt__(self, other):
        pass

    @abstractmethod
    def __eq__(self, other):
        pass

    @abstractmethod
    def get_data(self) -> dict:
        pass


class Command:
    """
    Base class of a command. Represents a single command together with a user chosen name describing the command
    """

    def __init__(self, name: str, cmd: List[str] | str):
        self.name = name
        self.cmd = cmd

    def __str__(self):
        if type(self.cmd) == str:
            return f"{self.name}: {self.cmd}"
        return f"{self.name}: {' '.join(self.cmd)}"

    def exists(self) -> bool:
        return shutil.which(self._get_binary_name()) is not None

    def _get_binary_name(self) -> str:
        if type(self.cmd) == str:
            return self.cmd.split(' ')[0]
        return self.cmd[0]

    def run(self, drop_cache: bool = False) -> CommandResult | None:
        if drop_cache:
            drop_ram_cache()
        proc = subprocess.Popen(self.cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                                shell=(type(self.cmd) == str))
        proc.wait()
        return


class BenchmarkResult(ABC):
    def __init__(self, benchmark_name: str):
        self.benchmark_name = benchmark_name
        self.results: dict = {}

    def __iadd__(self, other: CommandResult):
        """
        Calls other.__iadd__ if the provided CommandResult already is hold in self.results. If not other is added to
        self.results.
        :param other:
        :return:
        """
        if other.command.name in self.results.keys():
            self.results[other.command.name] += other
        else:
            self.results[other.command.name] = other
        return self

    def __str__(self):
        return f"Result of benchmark {self.benchmark_name!r}."

    def get_setup(self) -> dict:
        return {
            "name": self.benchmark_name,
            "hardware": {
                "client": platform.node(),
                "cpu": get_cpu_name()
            }
        }

    def get_result(self) -> dict:
        """
        Provides information about the Benchmark including its name, the hardware (client name and cpu) and the results
        collected on this benchmark.
        Might be overridden to provide more information.
        :return:
        """
        return self.results

    def write_json(self, path: str) -> None:
        """
        Write the benchmarks result to 'path' in json format
        :param path:
        :return:
        """
        data = {
            "setup": self.get_setup(),
            "results": self.get_result()
        }
        with open(path, "w") as out:
            out.write(json.dumps(data))

    @abstractmethod
    def plot(self, path: str = "") -> None:
        """
        Implementations of plot() should plot the collected results and safe the plot to 'path' if it is not empty.
        :param path:
        :return:
        """
        pass


class Benchmark(ABC):
    def __init__(self, name: str, commands: List[Command], setup_commands: List[Command] = None,
                 cleanup_commands: List[Command] = None, iterations: int = 3, drop_cache: bool = False):
        self.name = name
        self.commands = commands
        self.setup_commands = setup_commands if setup_commands is not None else []
        self.cleanup_commands = cleanup_commands if cleanup_commands is not None else []
        self.iterations = iterations
        self.drop_cache = drop_cache

    def __str__(self):
        return self.name

    def _setup(self):
        log("  ⏳ Setting things up...")
        for cmd in self.setup_commands:
            cmd.run()
        log("  ✅ Setup done")

    def _cleanup(self):
        log("  ⏳ Cleaning...")
        for cmd in self.cleanup_commands:
            cmd.run()
        log("  ✅ Cleanup done")

    @abstractmethod
    def _run_benchmarks(self) -> BenchmarkResult:
        pass

    def run(self) -> BenchmarkResult:
        self._setup()
        result = self._run_benchmarks()
        self._cleanup()
        return result


# ===== Error definitions ==============================================================================================

class CommandFailedError(Exception):
    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return self.msg


class InvalidCommandError(Exception):
    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return self.msg
