"""
Copyright 2023, Leon Freist
Author: Leon Freist <freist@informatik.uni-freiburg.de>

We use the output of InlineBench (https://github.com/lfreist/InlineBench) as results
"""
import json
import statistics
import subprocess

from base import (Command, CommandResult, CommandFailedError, InvalidCommandError, BenchmarkResult, Benchmark, log,
                  get_cpu_name)

from typing import List, Tuple
import matplotlib as mpl
import matplotlib.pyplot as plt
import math


class InlineBenchResult(CommandResult):
    def __init__(self, command, data: dict):
        CommandResult.__init__(self, command)
        self.data = self._normalize_data(data)

    def _normalize_data(self, data: dict) -> dict:
        """
        Flattens the output of the json result of InlineBench into
        {
        "CPU": {"task": {"time [ns]": [int], "num threads": int}, ...},
        "Wall": {"task": {"time [ns]": [int], "num threads": int}, ...}
        }
        :param data:
        :return:
        """
        ret = {"CPU": {}, "Wall": {}}
        if data:
            for type_name, measurements in data.items():
                for task, threads in measurements.items():
                    if task not in ret[type_name].keys():
                        ret[type_name][task] = {}
                    for thread_id, m in threads.items():
                        if "time [ns]" in ret[type_name][task].keys():
                            ret[type_name][task]["time [ns]"][0] += m["time"]
                            ret[type_name][task]["num threads"] += 1
                        else:
                            ret[type_name][task]["time [ns]"] = [m["time"]]
                            ret[type_name][task]["num threads"] = 1
        for type_name, measurements in ret.items():
            for task, value in measurements.items():
                value["time [ns]"] = [int(value["time [ns]"][0] / value["num threads"])]
        return ret

    def __iadd__(self, other):
        """
        {
        "CPU": {"task": {"time [ns]": [int...], "num threads": int}, ...},
        "Wall": {"task": {"time [ns]": [int...], "num threads": int}, ...}
        }
        :param other:
        :return:
        """
        assert self.command == other.command
        for type_name, measurements in other.data.items():
            for task, value in measurements.items():
                if task in self.data[type_name].keys() and "time [ns]" in self.data[type_name][task].keys():
                    self.data[type_name][task]["time [ns]"] += value["time [ns]"]
                else:
                    self.data[type_name][task]["time [ns]"] = [value["time [ns]"]]
        return self

    def __len__(self):
        return self.data["Wall"]

    def get_data(self):
        return self.data

    def __getitem__(self, item: str):
        return self.data["Wall"][item]

    def __gt__(self, other):
        return False

    def __eq__(self, other):
        return True


class InlineBenchCommand(Command):
    def __init__(self, name: str, cmd: List[str] | str, cwd: str | None = None):
        Command.__init__(self, name, cmd, cwd)

    def run(self) -> InlineBenchResult | None:
        out = subprocess.Popen(self.cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE,
                               shell=(type(self.cmd) == str), cwd=self.cwd).communicate()[1]
        out = out.decode()
        try:
            data = json.loads(out)
            return InlineBenchResult(self, data)
        except ValueError:
            print(out)
            return None


class InlineBenchBenchmarkResult(BenchmarkResult):
    def __init__(self, benchmark_name: str):
        BenchmarkResult.__init__(self, benchmark_name)

    def plot(self, path: str = "") -> None:
        if not self.results:
            log("No results were collected...")
            return
        mpl.style.use("seaborn-v0_8")
        num_tasks = len((self.results[list(self.results.keys())[0]].data["Wall"].keys()))
        columns = num_tasks
        rows = 1
        fig_size = (4, 4)
        if columns == 3:
            fig_size = (6, 4)
        if columns > 8:
            fig_size = (10, 10)
            rows = 3
        elif columns > 3:
            rows = 2
        columns = math.ceil(columns / rows)
        fig, axs = plt.subplots(rows, columns, figsize=fig_size, sharex="all")
        fig.suptitle(self.benchmark_name, fontsize=16)
        y = rows - 1
        x = 0
        for task in self.results[list(self.results.keys())[0]].data["Wall"].keys():
            if num_tasks == 1:
                _axs = axs
            elif rows == 1:
                _axs = axs[x]
            else:
                _axs = axs[y, x]
            self._add_subplot(_axs, task, self._get_plot_data(task))
            x += 1
            if x >= columns:
                x = 0
                y -= 1
        plt.tight_layout()
        if path:
            plt.savefig(path, format="pdf")
        else:
            plt.show()

    def _add_subplot(self, axs, title: str, data: List[Tuple[str, float, float]]) -> None:
        # data.sort(key=lambda v: v[0])  # sort by x label (cmd name) so that we get the same order in all subplots
        x = []
        y = []
        y_err = []
        for i in data:
            x.append(i[0])
            y.append(i[1] / 1000000)
            y_err.append(i[2] / 1000000)
        bar_list = axs.bar(x, y, color="gray")
        for bar in bar_list:
            if bar.get_height() == min(y):
                bar.set_color("green")
            if bar.get_height() == max(y):
                bar.set_color("red")
        # replace nan with 0
        y_err = list(map(lambda x: x if x == x else 0, y_err))
        if sum(y_err) != 0:
            axs.errorbar(x, y, yerr=y_err, fmt='o', color='b')
        axs.set_xticklabels(labels=x, rotation=90)
        axs.set_title(title)
        axs.set_ylabel("time [ms]")

    def _get_plot_data(self, task: str) -> List[Tuple[str, float, float]]:
        ret = []
        for cmd, res in self.results.items():
            stdev = float("nan")
            if len(res[task]["time [ns]"]) > 1:
                stdev = statistics.stdev(res[task]["time [ns]"])
            ret.append((cmd, statistics.mean(res[task]["time [ns]"]), stdev))  # in milliseconds
        return ret


class InlineBenchBenchmark(Benchmark):
    def __init__(self, name: str, commands: List[InlineBenchCommand], setup_commands: List[Command] = None,
                 cleanup_commands: List[Command] = None, iterations: int = 3,
                 drop_cache: Command | None = None):
        Benchmark.__init__(self, name, commands, setup_commands, cleanup_commands, iterations, drop_cache)

    def _run_benchmarks(self) -> InlineBenchBenchmarkResult:
        result = InlineBenchBenchmarkResult(self.name)
        for iteration in range(self.iterations):
            for cmd in self.commands:
                if self.drop_cache is None:
                    cmd.run()
                else:
                    self.drop_cache.run()
                log(f"  {iteration}/{self.iterations}: {cmd.name}", end='\r', flush=True)
                part_res = cmd.run()
                if part_res is not None:
                    result += part_res
        log()
        return result
