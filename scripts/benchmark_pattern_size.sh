#!/usr/bin/env bash

# Benchmark description:
#  Goal: Runtime under different pattern sizes
#
#  Settings:
#   - file sizes are fixed (20GB)
#   - KEYWORDS: "kp", "karp", "krangenp", "kong better worp", "kome longer search pattern hellp"
#   - DENSITY: 2
#
#  Mark: every pattern starts with 'k' and ends with 'p'. The search algorithm used in SF first searches matches of
#        these positions. However this is still not optimal since the different distances affect how often the search
#        algorithm enters the inner loop (if first and last char match).

if [ "$EUID" -ne 0 ]; then
  echo "================================================================================"
  printf "\x1b[31mERROR: Dropping RAM caches requires root privileges!\x1b[m"
  echo "In order to achieve reproducible benchmarks, we need to drop RAM caches."
  echo "================================================================================"
  exit 1
fi

FILE_DIR=./tmp
EXE_DIR=./build-benchmark
BENCHMARK_DIR=./benchmark/pattern_size/$(date +"%Y%m%d-%H-%M-%S")
KEYWORDS=("kp" "karp" "krangenp" "kong better worp" "kome longer search pattern hellp")
FILE_SIZE=1
DENSITY=2

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
for pattern in "${KEYWORDS[@]}"; do
  if [ ! -f "$FILE_DIR/$FILE_SIZE-gb-pattern_size-${#pattern}.txt" ]; then
    python3 scripts/createTestFile.py -s "$FILE_SIZE" -o "$FILE_DIR/$FILE_SIZE-gb-pattern_size-${#pattern}.txt" --keyword "$pattern" --density "$DENSITY" files/words.txt >/dev/null &
  fi
done

wait

LOG "Preprocessing files..."
for pattern in "${KEYWORDS[@]}"; do
  if [ ! -f "$FILE_DIR/$FILE_SIZE-gb-pattern_size-${#pattern}.sf.meta" ]; then
    "$EXE_DIR/SFPreprocessorMain" "$FILE_DIR/$FILE_SIZE-gb-pattern_size-${#pattern}.txt" -a none -d 500 -m "$FILE_DIR/$FILE_SIZE-gb-pattern_size-${#pattern}.sf.meta" >/dev/null &
  fi
done

wait
LOG "Done preprocessing files"

# run benchmarks
for pattern in "${KEYWORDS[@]}"; do
  DROP_RAM_CACHE
  LOG "Running benchmark for keyword '$pattern' (size: ${#pattern})"
  "$EXE_DIR/sfgrep" "$pattern" "$FILE_DIR/$FILE_SIZE-gb-pattern_size-${#pattern}.txt" "$FILE_DIR/$FILE_SIZE-gb-pattern_size-${#pattern}.sf.meta" -n --benchmark "$BENCHMARK_DIR/$FILE_SIZE-gb-pattern_size-${#pattern}.json" >/dev/null
  # count occurrences of k.*p
  fake_pattern="k"
  for _ in $(seq $((${#pattern} - 2))); do
    fake_pattern+="."
  done
  fake_pattern+="p"
  outer_loop_matches=$(grep "$fake_pattern" "$FILE_DIR/$FILE_SIZE-gb-pattern_size-${#pattern}.txt" -c)
  correct_matches=$(grep "$pattern" "$FILE_DIR/$FILE_SIZE-gb-pattern_size-${#pattern}.txt" -c)
  truncate -s -1 "$BENCHMARK_DIR/$FILE_SIZE-gb-pattern_size-${#pattern}.json"
  echo ",\"original size\": $((FILE_SIZE * 1000 * 1000 * 1000)),\"pattern\": \"$pattern\",\"pattern size\": ${#pattern},\"false outer loop matches\": $((outer_loop_matches - correct_matches))}" >> "$BENCHMARK_DIR/$FILE_SIZE-gb-pattern_size-${#pattern}.json"
  clang-format-14 -i "$BENCHMARK_DIR/$FILE_SIZE-gb-pattern_size-${#pattern}.json"
done
