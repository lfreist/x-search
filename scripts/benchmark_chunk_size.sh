#!/usr/bin/env bash

if [ "$#" -eq 2 ]; then
  BINARY_DIR="$1"
  FILE="$2"
elif [ "$#" -eq 1 ]; then
  BINARY_DIR="$1"
  FILE="tmp/default.out"
else
  echo "Usage:"
  echo "./benchmark_chunk_size </path/to/binaries> [<path/to/file.txt>]"
  echo ""
  exit 1
fi

if [ "$EUID" -ne 0 ]; then
  echo "================================================================================"
  printf "\x1b[31mERROR: Dropping RAM caches requires root privileges!\x1b[m\n"
  echo "In order to achieve reproducible benchmarks, we need to drop RAM caches."
  echo "================================================================================"
  exit 1
fi

BENCHMARK_DIR_PREPROCESSED=./benchmark/preprocessed/chunk_size/$(date +"%Y%m%d-%H-%M-%S")
BENCHMARK_DIR_PLAIN=./benchmark/plain/chunk_size/$(date +"%Y%m%d-%H-%M-%S")

PATTERN="pattern"
DENSITY=32
FILE_SIZE=0.05
CHUNK_SIZE=(1 500 1000 4000 16000 32000 64000)

LOG() {
  echo "[$(date +'%T')]:  $1"
}

DROP_RAM_CACHE() {
  sync
  echo 1 >/proc/sys/vm/drop_caches
  sleep 1
  LOG "RAM caches dropped"
}

if [ ! -d "$BENCHMARK_DIR_PREPROCESSED" ]; then
  mkdir -p "$BENCHMARK_DIR_PREPROCESSED"
fi
if [ ! -d "$BENCHMARK_DIR_PLAIN" ]; then
  mkdir -p "$BENCHMARK_DIR_PLAIN"
fi

if [ ! -f "$FILE" ]; then
  LOG "Writing file..."
  python3 "$BINARY_DIR/scripts/createTestFile.py" -s "$FILE_SIZE" -o "$FILE" --keyword "$PATTERN" --density "$DENSITY" "$BINARY_DIR/files/words.txt" --progress
fi

# run benchmarks with preprocessing
for cs in "${CHUNK_SIZE[@]}"; do
  LOG "preprocessing file: -s $cs"
  "$BINARY_DIR/xsproc/XSPreprocessor" $FILE -m $FILE.meta -s "$cs" -j 4
  DROP_RAM_CACHE
  LOG "running xs/grep"
  "$BINARY_DIR/xsgrep/grep" $PATTERN $FILE $FILE.meta --benchmark-file "$BENCHMARK_DIR_PREPROCESSED/$cs.json" --benchmark-format json >/dev/null
  truncate -s -1 "$BENCHMARK_DIR_PREPROCESSED/$cs.json"
  meta_size=$(wc -c < "$FILE.meta")
  file_size=$(wc -c < "$FILE")
  echo ",\"original size\": $file_size,\"chunk size\": $cs,\"meta file size\": $meta_size}" >> "$BENCHMARK_DIR_PREPROCESSED/$cs.json"
  clang-format-14 -i "$BENCHMARK_DIR_PREPROCESSED/$cs.json"
done

# run benchmarks without preprocessing
for cs in "${CHUNK_SIZE[@]}"; do
  DROP_RAM_CACHE
  LOG "running xs/grep -s $cs"
  "$BINARY_DIR/xsgrep/grep" $PATTERN $FILE -s "$cs" --benchmark-file "$BENCHMARK_DIR_PLAIN/$cs.json" --benchmark-format json >/dev/null
  truncate -s -1 "$BENCHMARK_DIR_PLAIN/$cs.json"
  file_size=$(wc -c < "$FILE")
  echo ",\"original size\": $file_size\"chunk size\": $cs,\"meta file size\": 0}" >> "$BENCHMARK_DIR_PLAIN/$cs.json"
  clang-format-14 -i "$BENCHMARK_DIR_PLAIN/$cs.json"
done