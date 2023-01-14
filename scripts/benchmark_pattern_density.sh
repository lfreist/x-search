#!/usr/bin/env bash

# Benchmark description:
#  Goal: Runtime under different pattern DENSITIES (line wise)
#
#  Settings:
#   - file sizes are fixed (20GB)
#   - pattern: "pattern"
#   - DENSITIES: 0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 33554432

if [ "$EUID" -ne 0 ]; then
  echo "================================================================================"
  printf "\x1b[31mERROR: Dropping RAM caches requires root privileges!\x1b[m"
  echo "In order to achieve reproducible benchmarks, we need to drop RAM caches."
  echo "================================================================================"
  exit 1
fi

FILE_DIR=./tmp
EXE_DIR=./build-benchmark
BENCHMARK_DIR=./benchmark/pattern_density/$(date +"%Y%m%d-%H-%M-%S")
KEYWORD="pattern"
DENSITIES=(0 1 2 4 8 16 32 64 128 256 512 1024 33554432)
FILE_SIZE=1

LOG() {
  echo "[$(date +'%T')]:  $1"
}

DROP_RAM_CACHE() {
  sync
  echo 1 >/proc/sys/vm/drop_caches
  sleep 1
  LOG "RAM caches dropped"
}

if [ ! -d "$BENCHMARK_DIR" ]; then
  mkdir -p "$BENCHMARK_DIR"
fi

LOG "Writing test files..."
for density in "${DENSITIES[@]}"; do
  if [ ! -f "$FILE_DIR/$FILE_SIZE-gb-density-$density.txt" ]; then
    python3 scripts/createTestFile.py -s "$FILE_SIZE" -o "$FILE_DIR/$FILE_SIZE-gb-density-$density.txt" --keyword "$KEYWORD" --density "$density" files/words.txt >/dev/null &
  fi
done

wait

LOG "Preprocessing files..."
for density in "${DENSITIES[@]}"; do
  if [ ! -f "$FILE_DIR/$FILE_SIZE-gb-density-$density.sf.meta" ]; then
    "$EXE_DIR/SFPreprocessorMain" "$FILE_DIR/$FILE_SIZE-gb-density-$density.txt" -a none -d 500 -m "$FILE_DIR/$FILE_SIZE-gb-density-$density.sf.meta" >/dev/null &
  fi
done

wait
LOG "Done preprocessing files"

# run benchmarks
for density in "${DENSITIES[@]}"; do
  DROP_RAM_CACHE
  LOG "Running benchmark for density $density"
  "$EXE_DIR/sfgrep" "$KEYWORD" "$FILE_DIR/$FILE_SIZE-gb-density-$density.txt" "$FILE_DIR/$FILE_SIZE-gb-density-$density.sf.meta" -n --benchmark "$BENCHMARK_DIR/$FILE_SIZE-gb-density-$density.json" >/dev/null
  truncate -s -1 "$BENCHMARK_DIR/$FILE_SIZE-gb-density-$density.json"
  echo ",\"original size\": $((FILE_SIZE * 1000 * 1000 * 1000)),\"density\": $density}" >> "$BENCHMARK_DIR/$FILE_SIZE-gb-density-$density.json"
  clang-format-14 -i "$BENCHMARK_DIR/$FILE_SIZE-gb-density-$density.json"
done
