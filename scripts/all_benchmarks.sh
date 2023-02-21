python3 benchsuit/xs_benchmark.py build-benchmark/xsgrep/xs --iterations 10 --cache -o benchsuit/results/xs/cache/$(hostname)
python3 benchsuit/xs_benchmark.py build-benchmark/xsgrep/xs --iterations 10 -o benchsuit/results/xs/no-cache/$(hostname)

python3 benchsuit/comparison_benchmark.py --iterations 10 --cache -o benchsuit/results/comparison/cache/$(hostname)
python3 benchsuit/comparison_benchmark.py --iterations 10 -o benchsuit/results/comparison/no-cache/$(hostname)