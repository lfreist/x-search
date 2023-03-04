"""
Copyright 2023, Leon Freist
Author: Leon Freist <freist@informatik.uni-freiburg.de>

We utilize GNU time to measure wall, sys (cpu) and usr (cpu) times of commands.
"""
import platform
import statistics
import subprocess

from base import (Command, CommandResult, CommandFailedError, InvalidCommandError, BenchmarkResult, Benchmark,
                  get_cpu_name, log)

from typing import List, Tuple
import tempfile
import matplotlib as mpl
import matplotlib.pyplot as plt


class GNUTimeResult(CommandResult):
    def __init__(self, command, wall_s: float, usr_cpu_s: float, sys_cpu_s: float):
        CommandResult.__init__(self, command)
        self.wall_s = [wall_s]
        self.usr_cpu_s = [usr_cpu_s]
        self.sys_cpu_s = [sys_cpu_s]

    def __iadd__(self, other):
        assert self.command == other.command
        self.wall_s += other.wall_s
        self.usr_cpu_s += other.usr_cpu_s
        self.sys_cpu_s += other.sys_cpu_s
        return self

    def __len__(self):
        return len(self.wall_s)

    def get_data(self) -> dict:
        return {
            "wall [s]": self.wall_s,
            "usr [s]": self.usr_cpu_s,
            "sys [s]": self.sys_cpu_s,
            "cpu [s]": [self.sys_cpu_s[i] + self.usr_cpu_s[i] for i in range(len(self.usr_cpu_s))]
        }

    def __getitem__(self, item: str):
        if "wall" in item.lower():
            return self.wall_s
        if "usr" in item.lower():
            return self.usr_cpu_s
        if "sys" in item.lower():
            return self.sys_cpu_s
        if item.lower() == "cpu":
            return [self.sys_cpu_s[i] + self.usr_cpu_s[i] for i in range(len(self.usr_cpu_s))]
        raise KeyError(f"Unknown key {item}. Use 'wall', 'usr', 'sys' or 'cpu' instead.")

    def __gt__(self, other):
        if min(self.wall_s) > min(other.wall_s):
            return True
        elif min(self.wall_s) == min(other.wall_s):
            return min(self["cpu"]) > min(other["cpu"])
        return False

    def __eq__(self, other):
        return statistics.mean(self.wall_s) == statistics.mean(other.wall_s) and statistics.mean(
            self["cpu"]) == statistics.mean(other["cpu"])


class GNUTimeCommand(Command):
    def __init__(self, name: str, cmd: List[str] | str, cwd: str | None = None):
        Command.__init__(self, name, cmd, cwd)

    def _timed_command(self) -> List[str] | str:
        if type(self.cmd) == str:
            return f"/usr/bin/time -f '%e\t%U\t%S' {self.cmd}"
        return ["/usr/bin/time", "-f", "%e\t%U\t%S"] + self.cmd

    def run(self) -> GNUTimeResult | None:
        tmp_file = tempfile.TemporaryFile()
        out = subprocess.Popen(self._timed_command(), stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                               shell=(type(self.cmd) == str), cwd=self.cwd).communicate()[1]
        tmp_file.close()
        out = out.decode()
        wall, usr_cpu, sys_cpu = out.split('\t')
        try:
            return GNUTimeResult(self, float(wall), float(usr_cpu), float(sys_cpu))
        except ValueError:
            return None


class GNUTimeBenchmarkResult(BenchmarkResult):
    def __init__(self, benchmark_name: str):
        BenchmarkResult.__init__(self, benchmark_name)

    def plot(self, path: str = "") -> None:
        mpl.style.use("seaborn-v0_8")
        fig, axs = plt.subplots(1, 2, figsize=(8, 10))
        fig.suptitle(self.benchmark_name, fontsize=16)
        self._add_subplot(axs[0], "Wall Time", self._get_plot_data("wall"))
        self._add_subplot(axs[1], "CPU Time", self._get_plot_data("cpu"))
        axs[0].set_ylabel("Time [s]")
        plt.tight_layout()
        if path:
            plt.savefig(path, format="pdf")
        else:
            plt.show()

    @staticmethod
    def _add_subplot(axs, title: str, data: List[Tuple[str, float, float]]) -> None:
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
                bar.set_color("green")
            if bar.get_height() == max(y):
                bar.set_color("red")
        # replace nan with 0
        y_err = list(map(lambda _x: _x if _x == _x else 0, y_err))
        if sum(y_err) != 0:
            axs.errorbar(x, y, yerr=y_err, fmt='o', color='b')
        axs.set_xticklabels(labels=x, rotation=90)
        axs.set_title(title)

    def _get_plot_data(self, measure_type: str) -> List[Tuple[str, float, float]]:
        ret = []
        for cmd, res in self.results.items():
            data = res[measure_type]
            if len(data) > 1:
                stdev = statistics.stdev(data)
            else:
                stdev = float("nan")
            ret.append((cmd, statistics.mean(res[measure_type]), stdev))
        return ret


class GNUTimeBenchmark(Benchmark):
    def __init__(self, name: str, commands: List[GNUTimeCommand], setup_commands: List[Command] = None,
                 cleanup_commands: List[Command] = None, iterations: int = 3,
                 drop_cache: Command | None = None):
        Benchmark.__init__(self, name, commands, setup_commands, cleanup_commands, iterations, drop_cache)

    def _run_benchmarks(self) -> GNUTimeBenchmarkResult:
        result = GNUTimeBenchmarkResult(self.name)
        for iteration in range(self.iterations):
            for cmd in self.commands:
                if self.drop_cache is None:
                    cmd.run()
                else:
                    self.drop_cache.run()
                log(f"  {iteration}/{self.iterations}: {cmd}", end='\r', flush=True)
                part_res = cmd.run()
                if part_res:
                    result += part_res
        log()
        return result
