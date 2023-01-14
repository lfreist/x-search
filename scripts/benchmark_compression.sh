#!/usr/bin/env bash

# Benchmark description:
#  Goal: Runtime of different compression algorithms (zstd, lz4) and different compression levels
#
#  Settings:
#   - file size fixed
#   - pattern: "pattern"
#   - density: 0 -> we only measure decompression times here
#   - compression algorithms: none, zstd, lz4
#   - compression levels: 1, 2, 3, 4, 5, 6, 7, 8, 9

if [ "$EUID" -ne 0 ]; then
  echo "================================================================================"
  printf "\x1b[31mERROR: Dropping RAM caches requires root privileges!\x1b[m"
  echo "In order to achieve reproducible benchmarks, we need to drop RAM caches."
  echo "================================================================================"
  exit 1
fi

FILE_DIR=./tmp
EXE_DIR=./build-benchmark
BENCHMARK_DIR=./benchmark/compression/$(date +"%Y%m%d-%H-%M-%S")
KEYWORD="pattern"
DENSITY=0
LEVELS=(1 2 3 4 5 6 7 8 9)
FILE_SIZE=1

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

if [ ! -d "$BENCHMARK_DIR" ]; then
  mkdir -p "$BENCHMARK_DIR"
fi

if [ ! -d $FILE_DIR ]; then
  LOG "creating temporary file directory 'tmp'"
  mkdir $FILE_DIR
fi

LOG "Writing test files..."
if [ ! -f "$FILE_DIR/$FILE_SIZE-gb.txt" ]; then
  python3 scripts/createTestFile.py -s "$FILE_SIZE" -o "$FILE_DIR/$FILE_SIZE-gb.txt" --keyword "$KEYWORD" --density "$DENSITY" files/words.txt >/dev/null &
fi

wait

LOG "Preprocessing files using LZ4..."
for lvl in "${LEVELS[@]}"; do
  if [ ! -f "$FILE_DIR/$FILE_SIZE-gb-lvl-$lvl.sf.lz4" ]; then
    "$EXE_DIR/SFPreprocessorMain" "$FILE_DIR/$FILE_SIZE-gb.txt" -o "$FILE_DIR/$FILE_SIZE-gb-lvl-$lvl.sflz4" -a lz4 -l "$lvl" -d 500 -m "$FILE_DIR/$FILE_SIZE-gb-lvl-$lvl.sflz4.meta" > /dev/null &
  fi
done

wait

LOG "Preprocessing files using ZSTD..."
for lvl in "${LEVELS[@]}"; do
  if [ ! -f "$FILE_DIR/$FILE_SIZE-gb-lvl-$lvl.sf.zst" ]; then
    "$EXE_DIR/SFPreprocessorMain" "$FILE_DIR/$FILE_SIZE-gb.txt" -o "$FILE_DIR/$FILE_SIZE-gb-lvl-$lvl.sfzst" -a zstd -l "$lvl" -d 500 -m "$FILE_DIR/$FILE_SIZE-gb-lvl-$lvl.sfzst.meta" > /dev/null &
  fi
done

wait
LOG "Done preprocessing files."

# run benchmarks
for lvl in "${LEVELS[@]}"; do
  DROP_RAM_CACHE
  LOG "Running benchmark for LZ4 level $lvl"
  if "$EXE_DIR/sfgrep" "$KEYWORD" "$FILE_DIR/$FILE_SIZE-gb-lvl-$lvl.sflz4" "$FILE_DIR/$FILE_SIZE-gb-lvl-$lvl.sflz4.meta" -n --benchmark "$BENCHMARK_DIR/$FILE_SIZE-gb-lvl-$lvl-alg-lz4.json" >/dev/null; then
    truncate -s -1 "$BENCHMARK_DIR/$FILE_SIZE-gb-lvl-$lvl-alg-lz4.json"
    size=$(wc -c < "$FILE_DIR/$FILE_SIZE-gb-lvl-$lvl.sf.lz4")
    echo ",\"compressed size\": $size,\"original size\": $((FILE_SIZE * 1000 * 1000 * 1000))}" >> "$BENCHMARK_DIR/$FILE_SIZE-gb-lvl-$lvl-alg-lz4.json"
    clang-format-14 -i "$BENCHMARK_DIR/$FILE_SIZE-gb-lvl-$lvl-alg-lz4.json"
  else
    LOG "Error running: ./sfgrep $KEYWORD $FILE_SIZE-gb-lvl-$lvl.sflz4 ..."
  fi
done

for lvl in "${LEVELS[@]}"; do
  DROP_RAM_CACHE
  LOG "Running benchmark for ZSTD level $lvl"
  if "$EXE_DIR/sfgrep" "$KEYWORD" "$FILE_DIR/$FILE_SIZE-gb-lvl-$lvl.sfzst" "$FILE_DIR/$FILE_SIZE-gb-lvl-$lvl.sfzst.meta" -n --benchmark "$BENCHMARK_DIR/$FILE_SIZE-gb-lvl-$lvl-alg-zstd.json" >/dev/null; then
    truncate -s -1 "$BENCHMARK_DIR/$FILE_SIZE-gb-lvl-$lvl-alg-zstd.json"
    size=$(wc -c < "$FILE_DIR/$FILE_SIZE-gb-lvl-$lvl.sf.zst")
    echo ",\"compressed size\": $size,\"original size\": $((FILE_SIZE * 1000 * 1000 * 1000))}" >> "$BENCHMARK_DIR/$FILE_SIZE-gb-lvl-$lvl-alg-zstd.json"
    clang-format-14 -i "$BENCHMARK_DIR/$FILE_SIZE-gb-lvl-$lvl-alg-zstd.json"
  else
    LOG "Error running: ./sfgrep $KEYWORD $FILE_SIZE-gb-lvl-$lvl.sfzst ..."
  fi
done
