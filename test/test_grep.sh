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
  file="tmp/default.out"
  if [ ! -f "$file" ]; then
    echo "writing test file..."
    if ! python3 "$xs_binaries/scripts/createTestFile.py" --size 0.1 "$xs_binaries/files/words.txt" --output "$file"; then
      echo "ERROR: failed writing file"
      exit 1
    fi
  fi
else
  echo "Usage:"
  echo "./test_grep </path/to/xs/binaries> [<original/file>]"
  echo ""
  exit 1
fi

file_name=$(basename "${file}")

# preprocessing file:
echo " Processing $file..."
echo "  no compression: $file_name.xs.meta"
"$xs_binaries/xsproc/xspp" "$file" -a none -m "tmp/$file_name.xs.meta" >/dev/null &
echo "  lz4 compression: $file_name.xslz4.meta | $file_name.xslz4"
"$xs_binaries/xsproc/xspp" "$file" -a lz4 -m "tmp/$file_name.xslz4.meta" -o "tmp/$file_name.xslz4" >/dev/null &
echo "  zst compression: $file_name.xszst.meta | $file_name.xszst"
"$xs_binaries/xsproc/xspp" "$file" -a zst -m "tmp/$file_name.xszst.meta" -o "tmp/$file_name.xszst" >/dev/null &

echo " ..."
wait
echo " done"
# ---

keywords=("key" "pass" "pattern" "jeez")
regex_keywords=("pas[s|t]" "pat[t]*e")
in_files=("$file" "$file" "tmp/$file_name.xslz4" "tmp/$file_name.xszst")
meta_files=("" "tmp/$file_name.xs.meta" "tmp/$file_name.xslz4.meta" "tmp/$file_name.xszst.meta")
num_threads=(1 4)

errors=0
success=0

# running grep and saving outputs to /tmp
echo " running grep"
for i in "${keywords[@]}"; do
  echo "  keyword: $i"
  grep "$i" "$file" >"tmp/$i.grep_.tmp" &
  grep "$i" "$file" -b >"tmp/$i.grep_b.tmp" &
  grep "$i" "$file" -b -o >"tmp/$i.grep_b_o.tmp" &
  grep "$i" "$file" -n >"tmp/$i.grep_n.tmp" &
  grep "$i" "$file" -n -o >"tmp/$i.grep_n_o.tmp" &
  grep "$i" "$file" -c >"tmp/$i.grep_c.tmp" &
  wait
done
for i in "${regex_keywords[@]}"; do
  echo "  keyword: $i"
  grep "$i" "$file" >"tmp/$i.r_grep_.tmp" &
  grep "$i" "$file" -b >"tmp/$i.r_grep_b.tmp" &
  grep "$i" "$file" -n >"tmp/$i.r_grep_n.tmp" &
  grep "$i" "$file" -c >"tmp/$i.r_grep_c.tmp" &
  grep "$i" "$file" -c -F >"tmp/$i.r_grep_c_F.tmp" &
  wait
done
# ---

# running tests
echo "testing xsgrep output by comparing to grep..."

for nt in "${num_threads[@]}"; do
  echo "num threads: $nt"
  for ((index = 0; index < ${#in_files[@]}; index++)); do
    echo " file: ${in_files[index]} | ${meta_files[index]}"
    echo "  raw lines (no command line options)"
    for i in "${keywords[@]}"; do
      "$xs_binaries/xsgrep/xs" "$i" "${in_files[index]}" "${meta_files[index]}" -j "$nt" >tmp/xsgrep.tmp
      if diff tmp/xsgrep.tmp "tmp/$i.grep_.tmp" >/dev/null; then
        printf "   %s: \x1b[32mPASSED\x1b[m\n" "$i"
        success=$((success + 1))
      else
        printf "   %s: \x1b[31mFAILED!\x1b[m\n" "$i"
        errors=$((errors + 1))
      fi
    done
    for i in "${regex_keywords[@]}"; do
      "$xs_binaries/xsgrep/xs" "$i" "${in_files[index]}" "${meta_files[index]}" -j "$nt" >tmp/xsgrep.tmp
      if diff tmp/xsgrep.tmp "tmp/$i.r_grep_.tmp" >/dev/null; then
        printf "   %s: \x1b[32mPASSED\x1b[m\n" "$i"
        success=$((success + 1))
      else
        printf "   %s: \x1b[31mFAILED!\x1b[m\n" "$i"
        errors=$((errors + 1))
      fi
    done
    echo "  --no-mmap"
    for i in "${keywords[@]}"; do
      "$xs_binaries/xsgrep/xs" "$i" "${in_files[index]}" "${meta_files[index]}" --no-mmap -j "$nt" >tmp/xsgrep.tmp
      if diff tmp/xsgrep.tmp "tmp/$i.grep_.tmp" >/dev/null; then
        printf "   %s: \x1b[32mPASSED\x1b[m\n" "$i"
        success=$((success + 1))
      else
        printf "   %s: \x1b[31mFAILED!\x1b[m\n" "$i"
        errors=$((errors + 1))
      fi
    done
    for i in "${regex_keywords[@]}"; do
      "$xs_binaries/xsgrep/xs" "$i" "${in_files[index]}" "${meta_files[index]}" --no-mmap -j "$nt" >tmp/xsgrep.tmp
      if diff tmp/xsgrep.tmp "tmp/$i.r_grep_.tmp" >/dev/null; then
        printf "   %s: \x1b[32mPASSED\x1b[m\n" "$i"
        success=$((success + 1))
      else
        printf "   %s: \x1b[31mFAILED!\x1b[m\n" "$i"
        errors=$((errors + 1))
      fi
    done

    echo " count matches (-c)"
    for i in "${keywords[@]}"; do
      "$xs_binaries/xsgrep/xs" "$i" "${in_files[index]}" "${meta_files[index]}" -c -j "$nt" >tmp/xsgrep.tmp
      if diff tmp/xsgrep.tmp "tmp/$i.grep_c.tmp" >/dev/null; then
        printf "   %s: \x1b[32mPASSED\x1b[m\n" "$i"
        success=$((success + 1))
      else
        printf "   %s: \x1b[31mFAILED!\x1b[m\n" "$i"
        errors=$((errors + 1))
      fi
    done
    for i in "${regex_keywords[@]}"; do
      "$xs_binaries/xsgrep/xs" "$i" "${in_files[index]}" "${meta_files[index]}" -c -j "$nt" >tmp/xsgrep.tmp
      if diff tmp/xsgrep.tmp "tmp/$i.r_grep_c.tmp" >/dev/null; then
        printf "   %s: \x1b[32mPASSED\x1b[m\n" "$i"
        success=$((success + 1))
      else
        printf "   %s: \x1b[31mFAILED!\x1b[m\n" "$i"
        errors=$((errors + 1))
      fi
    done

    echo "  --no-mmap"
    for i in "${keywords[@]}"; do
      "$xs_binaries/xsgrep/xs" "$i" "${in_files[index]}" "${meta_files[index]}" -c --no-mmap -j "$nt" >tmp/xsgrep.tmp
      if diff tmp/xsgrep.tmp "tmp/$i.grep_c.tmp" >/dev/null; then
        printf "   %s: \x1b[32mPASSED\x1b[m\n" "$i"
        success=$((success + 1))
      else
        printf "   %s: \x1b[31mFAILED!\x1b[m\n" "$i"
        errors=$((errors + 1))
      fi
    done
    for i in "${regex_keywords[@]}"; do
      "$xs_binaries/xsgrep/xs" "$i" "${in_files[index]}" "${meta_files[index]}" -c --no-mmap -j "$nt" >tmp/xsgrep.tmp
      if diff tmp/xsgrep.tmp "tmp/$i.r_grep_c.tmp" >/dev/null; then
        printf "   %s: \x1b[32mPASSED\x1b[m\n" "$i"
        success=$((success + 1))
      else
        printf "   %s: \x1b[31mFAILED!\x1b[m\n" "$i"
        errors=$((errors + 1))
      fi
    done

    echo " count matches, fixed strings (-c -F)"
    for i in "${regex_keywords[@]}"; do
      "$xs_binaries/xsgrep/xs" "$i" "${in_files[index]}" "${meta_files[index]}" -c -F -j "$nt" >tmp/xsgrep.tmp
      if diff tmp/xsgrep.tmp "tmp/$i.r_grep_c_F.tmp" >/dev/null; then
        printf "   %s: \x1b[32mPASSED\x1b[m\n" "$i"
        success=$((success + 1))
      else
        printf "   %s: \x1b[31mFAILED!\x1b[m\n" "$i"
        errors=$((errors + 1))
      fi
    done

    echo " byte offsets (-b)"
    for i in "${keywords[@]}"; do
      "$xs_binaries/xsgrep/xs" "$i" "${in_files[index]}" "${meta_files[index]}" -b -j "$nt" >tmp/xsgrep.tmp
      if diff tmp/xsgrep.tmp "tmp/$i.grep_b.tmp" >/dev/null; then
        printf "   %s: \x1b[32mPASSED\x1b[m\n" "$i"
        success=$((success + 1))
      else
        printf "  %s: \x1b[31mFAILED!\x1b[m\n" "$i"
        errors=$((errors + 1))
      fi
    done
    for i in "${regex_keywords[@]}"; do
      "$xs_binaries/xsgrep/xs" "$i" "${in_files[index]}" "${meta_files[index]}" -b -j "$nt" >tmp/xsgrep.tmp
      if diff tmp/xsgrep.tmp "tmp/$i.r_grep_b.tmp" >/dev/null; then
        printf "   %s: \x1b[32mPASSED\x1b[m\n" "$i"
        success=$((success + 1))
      else
        printf "  %s: \x1b[31mFAILED!\x1b[m\n" "$i"
        errors=$((errors + 1))
      fi
    done

    echo "  --no-mmap"
    for i in "${keywords[@]}"; do
      "$xs_binaries/xsgrep/xs" "$i" "${in_files[index]}" "${meta_files[index]}" -b --no-mmap -j "$nt" >tmp/xsgrep.tmp
      if diff tmp/xsgrep.tmp "tmp/$i.grep_b.tmp" >/dev/null; then
        printf "   %s: \x1b[32mPASSED\x1b[m\n" "$i"
        success=$((success + 1))
      else
        printf "  %s: \x1b[31mFAILED!\x1b[m\n" "$i"
        errors=$((errors + 1))
      fi
    done
    for i in "${regex_keywords[@]}"; do
      "$xs_binaries/xsgrep/xs" "$i" "${in_files[index]}" "${meta_files[index]}" -b --no-mmap -j "$nt" >tmp/xsgrep.tmp
      if diff tmp/xsgrep.tmp "tmp/$i.r_grep_b.tmp" >/dev/null; then
        printf "   %s: \x1b[32mPASSED\x1b[m\n" "$i"
        success=$((success + 1))
      else
        printf "  %s: \x1b[31mFAILED!\x1b[m\n" "$i"
        errors=$((errors + 1))
      fi
    done

    echo " byte offsets of match (-b -o)"
    for i in "${keywords[@]}"; do
      "$xs_binaries/xsgrep/xs" "$i" "${in_files[index]}" "${meta_files[index]}" -b -o -j "$nt" >tmp/xsgrep.tmp
      if diff tmp/xsgrep.tmp "tmp/$i.grep_b_o.tmp" >/dev/null; then
        printf "   %s: \x1b[32mPASSED\x1b[m\n" "$i"
        success=$((success + 1))
      else
        printf "   %s: \x1b[31mFAILED!\x1b[m\n" "$i"
        errors=$((errors + 1))
      fi
    done
    echo "  --no-mmap"
    for i in "${keywords[@]}"; do
      "$xs_binaries/xsgrep/xs" "$i" "${in_files[index]}" "${meta_files[index]}" -b -o --no-mmap -j "$nt" >tmp/xsgrep.tmp
      if diff tmp/xsgrep.tmp "tmp/$i.grep_b_o.tmp" >/dev/null; then
        printf "   %s: \x1b[32mPASSED\x1b[m\n" "$i"
        success=$((success + 1))
      else
        printf "   %s: \x1b[31mFAILED!\x1b[m\n" "$i"
        errors=$((errors + 1))
      fi
    done
    # we do not test the -o (--only-matching) flag for regex because GNU grep returns the last match found in a line,
    # while xsgrep returns the first match. This is not a bug since there is no specification about this scenario.
    # The man pages of GNU grep v3.7 state the following: "Print only the matched (non-empty) parts of a matching line,
    # with each such part on a separate output line.".
    # However in practice we could observe, that only one non empty part of a line is printed, even if a line contains
    # multiple matches.

    echo " line numbers (-n)"
    for i in "${keywords[@]}"; do
      "$xs_binaries/xsgrep/xs" "$i" "${in_files[index]}" "${meta_files[index]}" -n -j "$nt" >tmp/xsgrep.tmp
      if diff tmp/xsgrep.tmp "tmp/$i.grep_n.tmp" >/dev/null; then
        printf "   %s: \x1b[32mPASSED\x1b[m\n" "$i"
        success=$((success + 1))
      else
        printf "   %s: \x1b[31mFAILED!\x1b[m\n" "$i"
        errors=$((errors + 1))
      fi
    done
    for i in "${regex_keywords[@]}"; do
      "$xs_binaries/xsgrep/xs" "$i" "${in_files[index]}" "${meta_files[index]}" -n -j "$nt" >tmp/xsgrep.tmp
      if diff tmp/xsgrep.tmp "tmp/$i.r_grep_n.tmp" >/dev/null; then
        printf "   %s: \x1b[32mPASSED\x1b[m\n" "$i"
        success=$((success + 1))
      else
        printf "   %s: \x1b[31mFAILED!\x1b[m\n" "$i"
        errors=$((errors + 1))
      fi
    done

    echo "  --no-mmap"
    for i in "${keywords[@]}"; do
      "$xs_binaries/xsgrep/xs" "$i" "${in_files[index]}" "${meta_files[index]}" -n --no-mmap -j "$nt" >tmp/xsgrep.tmp
      if diff tmp/xsgrep.tmp "tmp/$i.grep_n.tmp" >/dev/null; then
        printf "   %s: \x1b[32mPASSED\x1b[m\n" "$i"
        success=$((success + 1))
      else
        printf "   %s: \x1b[31mFAILED!\x1b[m\n" "$i"
        errors=$((errors + 1))
      fi
    done
    for i in "${regex_keywords[@]}"; do
      "$xs_binaries/xsgrep/xs" "$i" "${in_files[index]}" "${meta_files[index]}" -n --no-mmap -j "$nt" >tmp/xsgrep.tmp
      if diff tmp/xsgrep.tmp "tmp/$i.r_grep_n.tmp" >/dev/null; then
        printf "   %s: \x1b[32mPASSED\x1b[m\n" "$i"
        success=$((success + 1))
      else
        printf "   %s: \x1b[31mFAILED!\x1b[m\n" "$i"
        errors=$((errors + 1))
      fi
    done

    echo " line numbers match only (-n -o)"
    for i in "${keywords[@]}"; do
      "$xs_binaries/xsgrep/xs" "$i" "${in_files[index]}" "${meta_files[index]}" -n -o -j "$nt" >tmp/xsgrep.tmp
      if diff tmp/xsgrep.tmp "tmp/$i.grep_n_o.tmp" >/dev/null; then
        printf "   %s: \x1b[32mPASSED\x1b[m\n" "$i"
        success=$((success + 1))
      else
        printf "   %s: \x1b[31mFAILED!\x1b[m\n" "$i"
        errors=$((errors + 1))
      fi
    done

    echo "  --no-mmap"
    for i in "${keywords[@]}"; do
      "$xs_binaries/xsgrep/xs" "$i" "${in_files[index]}" "${meta_files[index]}" -n -o --no-mmap -j "$nt" >tmp/xsgrep.tmp
      if diff tmp/xsgrep.tmp "tmp/$i.grep_n_o.tmp" >/dev/null; then
        printf "   %s: \x1b[32mPASSED\x1b[m\n" "$i"
        success=$((success + 1))
      else
        printf "   %s: \x1b[31mFAILED!\x1b[m\n" "$i"
        errors=$((errors + 1))
      fi
    done
    # we do not test the -o (--only-matching) flag for regex because GNU grep returns the last match found in a line,
    # while xsgrep returns the first match. This is not a bug since there is no specification about this scenario.
    # The man pages of GNU grep v3.7 state the following: "Print only the matched (non-empty) parts of a matching line,
    # with each such part on a separate output line.".
    # However in practice we could observe, that only one non empty part of a line is printed, even if a line contains
    # multiple matches.
  done
done

printf "\n%i tests have been performed.\n" "$((errors + success))"

if [ "$((errors))" -ne 0 ]; then
  printf "\x1b[31mTest finished with %i errors.\x1b[m\n" "$errors"
  exit 1
else
  printf "\x1b[32mAll tests passed!\x1b[m\n"
fi
