#python3 benchsuit/xs_benchmark.py build-benchmark/xsgrep/xs --iterations 10 --cache -o benchsuit/results/xs/cache/$(hostname)
#python3 benchsuit/xs_benchmark.py build-benchmark/xsgrep/xs --iterations 10 -o benchsuit/results/xs/no-cache/$(hostname)

#python3 benchsuit/comparison_benchmark.py --iterations 10 --cache -o benchsuit/results/search_tools/cache/$(hostname)
#python3 benchsuit/comparison_benchmark.py --iterations 10 -o benchsuit/results/search_tools/no-cache/$(hostname)

python3 benchsuit/searcher_benchmarks.py ./cmake-build-benchmark/task_benchmarks/searchers --iterations 10 -o benchsuit/results/tasks/searchers/$(hostname)

python3 benchsuit/reader_benchmarks.py ./cmake-build-benchmark/task_benchmarks/readers --iterations 10 -o benchsuit/results/tasks/readers/$(hostname)