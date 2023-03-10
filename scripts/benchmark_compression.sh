#!/usr/bin/env bash

if [ "$#" -eq 2 ]; then
  BINARY_DIR="$1"
  FILE="$2"
elif [ "$#" -eq 1 ]; then
  BINARY_DIR="$1"
  FILE="tmp/default.out"
else
  echo "Usage:"
  echo "./benchmark_compression </path/to/binaries> [<path/to/file.txt>]"
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

BENCHMARK_DIR_PREPROCESSED=./benchmark/preprocessed/nl_mapping/$(date +"%Y%m%d-%H-%M-%S")
BENCHMARK_DIR_PLAIN=./benchmark/plain/nl_mapping/$(date +"%Y%m%d-%H-%M-%S")
PATTERN="pattern"
DENSITY=32
FILE_SIZE=0.05
COMPRESSIONS=("-a zstd -l 1" "-a zstd -l 2" "-a zstd -l 3" "-a zstd -l 4" "-a zstd -l 5" "-a zstd -l 6" "-a lz4" "-a lz4 --hc -l 1" "-a lz4 --hc -l 5" "-a lz4 --hc -l 12")

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
  LOG "Writing test file..."
  python3 "$BINARY_DIR/scripts/createTestFile.py" -s $FILE_SIZE -o $FILE --keyword $PATTERN --density $DENSITY "$BINARY_DIR/files/words.txt" >/dev/null
fi

FILE_SIZE=$(wc -c < "$FILE")

# run benchmarks
for comp in "${COMPRESSIONS[@]}"; do
  LOG "preprocessing: $comp"
  "$BINARY_DIR/xsproc/XSPreprocessor" $FILE -o $FILE.pp -m $FILE.meta "$comp" -j 4
  DROP_RAM_CACHE
  LOG " grep:"
  "$BINARY_DIR/xsgrep/grep" "$KEYWORD" $FILE.pp $FILE.meta --benchmark-file "$BENCHMARK_DIR_PREPROCESSED/$comp.json" --benchmark-format json >/dev/null;
  truncate -s -1 "$BENCHMARK_DIR_PREPROCESSED/$comp.json"
  size=$(wc -c < "$FILE.pp")
  meta_file_size=$(wc -c < "$FILE.meta")
  echo ",\"compressed size\": $size,\"original size\": $FILE_SIZE,\"meta file size\": $meta_file_size,\"preprocessing\": $comp}" >> "$BENCHMARK_DIR/$comp.json"
  clang-format-14 -i "$BENCHMARK_DIR/$comp.json"
done
