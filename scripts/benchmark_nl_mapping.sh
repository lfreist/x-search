#!/usr/bin/env bash

# Benchmark description:
#  Goal: Check whether the filesize affects
#   - data throughput (GB/ms) of just reading the data from disk
#   - searching the data with the pattern equally distributed over the lines (bytes/ms)
#
#  Settings:
#   - new line data: 64, 128, 256, 512, 1024, 2048, 4096, 8192
#   - pattern: "pattern"
#   - density: 0
#   - compression: none

if [ "$EUID" -ne 0 ]; then
  echo "================================================================================"
  printf "\x1b[31mERROR: Dropping RAM caches requires root privileges!\x1b[m"
  echo "In order to achieve reproducible benchmarks, we need to drop RAM caches."
  echo "================================================================================"
  exit 1
fi

FILE_DIR=./tmp
EXE_DIR=./build-benchmark
BENCHMARK_DIR=./benchmark/nl_mapping/$(date +"%Y%m%d-%H-%M-%S")
PATTERN="pattern"
DENSITY=32
FILE_SIZE=1
NL_MAPPING=(128 256 512 1024 2048 4096 8192)

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

LOG "Writing test file..."
if [ ! -f "/$FILE_DIR/$FILE_SIZE-gb.txt" ]; then
  python3 scripts/createTestFile.py -s "$FILE_SIZE" -o "$FILE_DIR/$FILE_SIZE-gb.txt" --keyword "$PATTERN" --density "$DENSITY" files/words.txt >/dev/null
fi

LOG "Preprocessing files..."
for nl_mapping in "${NL_MAPPING[@]}"; do
  if [ ! -f "$FILE_DIR/$FILE_SIZE-gb-nl-$nl_mapping.sf.meta" ]; then
    "$EXE_DIR/SFPreprocessorMain" "$FILE_DIR/$FILE_SIZE-gb-density-$DENSITY.txt" -a none -d "$nl_mapping" -m "$FILE_DIR/$FILE_SIZE-gb-nl-$nl_mapping.sf.meta" >/dev/null &
  fi
done

wait
LOG "Done preprocessing files"

# run benchmarks
for nl_mapping in "${NL_MAPPING[@]}"; do
  DROP_RAM_CACHE
  LOG "Running benchmark for mapping distance $nl_mapping"
  "$EXE_DIR/sfgrep" "$PATTERN" "$FILE_DIR/$FILE_SIZE-gb.txt" "$FILE_DIR/$FILE_SIZE-gb-nl-$nl_mapping.sf.meta" -n --benchmark "$BENCHMARK_DIR/$nl_mapping-gb.json" >/dev/null
  truncate -s -1 "$BENCHMARK_DIR/$nl_mapping-gb.json"
  meta_size=$(wc -c < "$FILE_DIR/$FILE_SIZE-gb-nl-$nl_mapping.sf.meta")
  echo ",\"original size\": $((FILE_SIZE * 1000 * 1000 * 1000)),\"nl mapping\": $nl_mapping,\"meta file size\": $meta_size}" >> "$BENCHMARK_DIR/$nl_mapping-gb.json"
  clang-format-14 -i "$BENCHMARK_DIR/$nl_mapping-gb.json"
done