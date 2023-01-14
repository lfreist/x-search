#!/usr/bin/env bash

# Benchmark description:
#  Goal: Check whether the filesize affects
#   - data throughput (GB/ms) of just reading the data from disk
#   - searching the data with the pattern equally distributed over the lines (bytes/ms)
#
#  Settings:
#   - file sizes of 1, 5, 10, 15, 20, 25, 30 GB
#   - pattern: "pattern"
#   - density: 32

if [ "$EUID" -ne 0 ]; then
  echo "================================================================================"
  printf "\x1b[31mERROR: Dropping RAM caches requires root privileges!\x1b[m"
  echo "In order to achieve reproducible benchmarks, we need to drop RAM caches."
  echo "================================================================================"
  exit 1
fi

FILE_DIR=./tmp
EXE_DIR=./build-benchmark
BENCHMARK_DIR=./benchmark/file_size/$(date +"%Y%m%d-%H-%M-%S")
PATTERN="pattern"
DENSITY=32
FILE_SIZE=(1 2 4)

if [ ! -d "$BENCHMARK_DIR" ]; then
  mkdir -p "$BENCHMARK_DIR"
fi

LOG() {
  echo "[$(date +'%T')]:  $1"
}

DROP_RAM_CACHE() {
  sync
  echo 1 >/proc/sys/vm/drop_caches
  sleep 1
  LOG "RAM caches dropped"
}

LOG "Writing test files..."
for f_size in "${FILE_SIZE[@]}"; do
  if [ ! -f "$FILE_DIR/$f_size-gb.txt" ]; then
    python3 scripts/createTestFile.py -s "$f_size" -o "$FILE_DIR/$f_size-gb.txt" --keyword "$PATTERN" --density "$DENSITY" files/words.txt >/dev/null &
  fi
done

wait

LOG "Preprocessing files..."
for f_size in "${FILE_SIZE[@]}"; do
  if [ ! -f "$FILE_DIR/$f_size-gb.sf.meta" ]; then
    "$EXE_DIR/SFPreprocessorMain" "$FILE_DIR/$f_size-gb.txt" -a none -d 500 -m "$FILE_DIR/$f_size-gb.sf.meta" >/dev/null &
  fi
done

wait
LOG "Done preprocessing files"

# run benchmarks
for f_size in "${FILE_SIZE[@]}"; do
  DROP_RAM_CACHE
  LOG "Running benchmark for file size $f_size"
  "$EXE_DIR/sfgrep" "$PATTERN" "$FILE_DIR/$f_size-gb.txt" "$FILE_DIR/$f_size-gb.sf.meta" -n --benchmark "$BENCHMARK_DIR/$f_size-gb.json" >/dev/null
  truncate -s -1 "$BENCHMARK_DIR/$f_size-gb.json"
  echo ",\"original size\": $((f_size * 1000 * 1000 * 1000))}" >> "$BENCHMARK_DIR/$f_size-gb.json"
  clang-format-14 -i "$BENCHMARK_DIR/$f_size-gb.json"
done
