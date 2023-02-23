"""
Copyright 2023, Leon Freist
Author: Leon Freist <freist@informatik.uni-freiburg.de>

We utilize GNU time to measure wall, sys (cpu) and usr (cpu) times of commands.
"""
import platform
import statistics
import subprocess

import cmdbench
from typing import List, Tuple
import tempfile
import matplotlib as mpl
import matplotlib.pyplot as plt


class GNUTimeResult(cmdbench.CommandResult):
    def __init__(self, command, wall_s: float, usr_cpu_s: float, sys_cpu_s: float):
        cmdbench.CommandResult.__init__(self, command)
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


class GNUTimeCommand(cmdbench.Command):
    def __init__(self, name: str, cmd: List[str] | str, pre_commands: List[cmdbench.Command] = None):
        cmdbench.Command.__init__(self, name, cmd)
        self.pre_commands = pre_commands if pre_commands is not None else []

    def _timed_command(self) -> List[str] | str:
        if type(self.cmd) == str:
            return f"/usr/bin/time -f '%e\t%U\t%S' {self.cmd}"
        return ["/usr/bin/time", "-f", "%e\t%U\t%S"] + self.cmd

    def run(self, drop_cache: bool = False) -> GNUTimeResult:
        for cmd in self.pre_commands:
            cmd.run()
        if drop_cache:
            cmdbench.drop_ram_cache()
        tmp_file = tempfile.TemporaryFile()
        out = subprocess.Popen(self._timed_command(), stdout=tmp_file, stderr=subprocess.PIPE,
                               shell=(type(self.cmd) == str)).communicate()[1]
        tmp_file.close()
        out = out.decode()
        wall, usr_cpu, sys_cpu = out.split('\t')
        try:
            return GNUTimeResult(self, float(wall), float(usr_cpu), float(sys_cpu))
        except ValueError:
            raise cmdbench.CommandFailedError(self)


class GNUTimeBenchmarkResult(cmdbench.BenchmarkResult):
    def __init__(self, benchmark_name: str):
        cmdbench.BenchmarkResult.__init__(self, benchmark_name)

    def get_result(self) -> dict:
        """
        Provides information about the Benchmark including its name, the hardware (client name and cpu) and the results
        collected on this benchmark.
        Might be overridden to provide more information.
        :return:
        """
        return {name: r.get_data() for name, r in self.results.items()}

    def plot(self, path: str = "") -> None:
        mpl.style.use("seaborn-v0_8")
        fig, axs = plt.subplots(1, 2, figsize=(8, 10))
        self._add_subplot(axs[0], "Wall Time [s]", self._get_plot_data("wall"))
        self._add_subplot(axs[1], "CPU Time [s]", self._get_plot_data("cpu"))
        plt.tight_layout()
        if path:
            plt.savefig(path, format="pdf")
        else:
            plt.show()

    @staticmethod
    def _add_subplot(axs, title: str, data: List[Tuple[str, float, float]]) -> None:
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
                bar.set_color("green")
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
            data = res[measure_type]
            if len(data) > 1:
                stdev = statistics.stdev(data)
            else:
                stdev = float("nan")
            ret.append((cmd, statistics.mean(res[measure_type]), stdev))
        return ret


class GNUTimeBenchmark(cmdbench.Benchmark):
    def __init__(self, name: str, commands: List[GNUTimeCommand], setup_commands: List[cmdbench.Command] = None,
                 cleanup_commands: List[cmdbench.Command] = None, iterations: int = 3, drop_cache: bool = False):
        cmdbench.Benchmark.__init__(self, name, commands, setup_commands, cleanup_commands, iterations, drop_cache)

    def _run_benchmarks(self) -> GNUTimeBenchmarkResult:
        result = GNUTimeBenchmarkResult(self.name)
        for iteration in range(self.iterations):
            for cmd in self.commands:
                cmdbench.log(f"  {iteration}/{self.iterations}: {cmd.name}", end='\r', flush=True)
                result += cmd.run(self.drop_cache)
        cmdbench.log()
        return result
