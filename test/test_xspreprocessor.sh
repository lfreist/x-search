#!/usr/bin/env bash

# setup
echo "Setting up test files"
if [ ! -d ./tmp/ ]; then
  mkdir tmp
fi

if [ "$#" -eq 2 ]; then
  xs_binaries="$1"
  file="$2"
elif [ "$#" -eq 1 ]; then
  xs_binaries="$1"
  file="tmp/dummy.txt"
  if [ ! -f "$file" ]; then
    echo "writing test file..."
    python3 "$xs_binaries/scripts/createTestFile.py" --size 0.2 "$xs_binaries/files/words.txt" --output "$file"
  fi
else
  echo "Usage:"
  echo "./test_xspreprocessor </path/to/xs/binaries> [<original/file>]"
  echo ""
  exit 1
fi

compression_algs=("none" "zstd" "lz4" "lz4 --hc -l 5")
mapping_distances=(1 500 20000)
chunk_sizes=(1 16000 64000)
num_threads=(1 4)
pattern="pattern"
out_file="tmp/out"
meta_file="tmp/out.meta"
grep_args=("" "-n" "-n -o" "-b" "-b -o")

errors=0
success=0

# running grep as reference
echo "running grep for collecting reference results"

grep "$pattern" "$file" >"tmp/grep.tmp" &
grep "$pattern" "$file" -n >"tmp/grep-n.tmp" &
grep "$pattern" "$file" -b >"tmp/grep-b.tmp" &
grep "$pattern" "$file" -n -o >"tmp/grep-n -o.tmp" &
grep "$pattern" "$file" -b -o >"tmp/grep-b -o.tmp" &

wait

for nt in "${num_threads[@]}"; do
  for c_alg in "${compression_algs[@]}"; do
    for md in "${mapping_distances[@]}"; do
      for cs in "${chunk_sizes[@]}"; do
        # preprocessing
        printf "xspp %s:\n" "-a $c_alg -d $md -s $cs -j $nt"
        "./$xs_binaries/xsproc/xspp" "$file" -o "$out_file" -m "$meta_file" -a "$c_alg" -d "$md" -s "$cs" -j "$nt"
        for grep_arg in "${grep_args[@]}"; do
          printf "  grep %s:\t" "$grep_arg"
          # running xs/grep
          "./$xs_binaries/xsgrep/xs" "$pattern" "$out_file" "$meta_file" $grep_arg >tmp/xsgrep.tmp
          if diff tmp/xsgrep.tmp "tmp/grep$grep_arg.tmp" >/dev/null; then
            printf "\x1b[32mPASSED\x1b[m\n"
            success=$((success + 1))
          else
            printf "\x1b[31mFAILED!\x1b[m\n"
            errors=$((errors + 1))
          fi
        done
      done
    done
  done
done

printf "\n%i tests have been performed.\n" "$((errors + success))"

if [ "$((errors))" -ne 0 ]; then
  printf "\x1b[31mTest finished with %i errors.\x1b[m\n" "$errors"
  exit 1
else
  printf "\x1b[32mAll tests passed!\x1b[m\n"
fi
