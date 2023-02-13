#!/usr/bin/env bash

BENCHMARK() {
  exec 3>&1 4>&2
  res=$(
    { /usr/bin/time -f "{\"wall [s]\": %e, \"user [s]\": %U, \"sys [s]\": %S, }" "$@" 1>&3 2>&4; } 2>&1
  )
  exec 3>&- 4>&-
  echo "$res"
}

DROP_RAM_CACHE() {
  sync
  echo 1 >/proc/sys/vm/drop_caches
  sleep 1
  LOG "RAM caches dropped"
}

DROP_RAM_CACHE

echo "grep -n:"
time /bin/grep pattern ../30gb.dummy.txt -n | cat > /dev/null

DROP_RAM_CACHE

echo "xs/grep -n:"
time build/xsgrep/grep pattern ../30gb.dummy.txt -n | cat > /dev/null

DROP_RAM_CACHE

echo "xs/grep -n with meta file:"
time /bin/grep pattern ../30gb.dummy.txt ../30gb.dummy.xs.meta -n | cat > /dev/null
