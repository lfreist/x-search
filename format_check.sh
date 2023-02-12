#!/usr/bin/env bash

printf "Checking sources for code style\n"
SOURCE_FILES=()
# find all h/c/hpp/cpp files excluding the ad_utility directory
find ./src/ ./test/ ./include/ ./xsgrep/ ./xsproc/ -not \( -path ./include/xsearch/utils/ad_utility -prune \) -regextype egrep -regex '.*\.(h|c)(pp|xx)?$' -print0 >sourcelist

while IFS= read -r -d $'\0'; do
  SOURCE_FILES+=("$REPLY")
done <sourcelist

FAILED_FILES=()
for source in "${SOURCE_FILES[@]}"; do
  clang-format-14 -output-replacements-xml "$source" | grep "<replacement " &>/dev/null
  HAS_WRONG_FILES=$?
  if [ $HAS_WRONG_FILES -ne 1 ]; then
    FAILED_FILES+=("$source")
  fi
done

rm sourcelist

if [ "${#FAILED_FILES}" -eq 0 ]; then
  printf "\x1b[32mCongratulations! All sources match the code style\x1b[m\n"
else
  printf "\x1b[31mclang-format-14 discovered style issues in the following files:\x1b[m\n"
  for f in "${FAILED_FILES[@]}"; do
    echo "  - $f"
  done
  exit 1
fi
