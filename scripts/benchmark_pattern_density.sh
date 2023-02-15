#!/usr/bin/env bash

if [ "$#" -eq 2 ]; then
  BINARY_DIR="$1"
  FILE="$2"
elif [ "$#" -eq 1 ]; then
  BINARY_DIR="$1"
  FILE="tmp/default.out"
else
  echo "Usage:"
  echo "./benchmark_pattern_density </path/to/binaries> [<path/to/file.txt>]"
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
FILE_SIZE=1
PATTERN_DENSITY=(1 4 16 256 65536 0)

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

# run benchmarks with preprocessing
for pd in "${PATTERN_DENSITY[@]}"; do
  LOG "Writing file with density $pd"
  python3 "$BINARY_DIR/scripts/createTestFile.py" -s "$FILE_SIZE" -o "$FILE" --keyword "$PATTERN" --density "$pd" "$BINARY_DIR/files/words.txt" --progress
  LOG "preprocessing file:"
  "$BINARY_DIR/xsproc/XSPreprocessor" $FILE -m $FILE.meta -j 4
  DROP_RAM_CACHE
  LOG "running xs/grep with meta file"
  "$BINARY_DIR/xsgrep/grep" $PATTERN $FILE $FILE.meta --benchmark-file "$BENCHMARK_DIR_PREPROCESSED/$pd.json" --benchmark-format json >/dev/null
  truncate -s -1 "$BENCHMARK_DIR_PREPROCESSED/$pd.json"
  meta_size=$(wc -c < "$FILE.meta")
  file_size=$(wc -c < "$FILE")
  echo ",\"original size\": $file_size,\"meta file size\": $meta_size,\"pattern density\": $pd}" >> "$BENCHMARK_DIR_PREPROCESSED/$pd.json"
  clang-format-14 -i "$BENCHMARK_DIR_PREPROCESSED/$pd.json"

  DROP_RAM_CACHE
  LOG "running xs/grep without meta file"
  "$BINARY_DIR/xsgrep/grep" $PATTERN $FILE --benchmark-file "$BENCHMARK_DIR_PLAIN/$pd.json" --benchmark-format json >/dev/null
  truncate -s -1 "$BENCHMARK_DIR_PLAIN/$pd.json"
  meta_size=$(wc -c < "$FILE.meta")
  file_size=$(wc -c < "$FILE")
  echo ",\"original size\": $file_size,\"meta file size\": $meta_size,\"pattern density\": $pd}" >> "$BENCHMARK_DIR_PLAIN/$pd.json"
  clang-format-14 -i "$BENCHMARK_DIR_PLAIN/$pd.json"
done