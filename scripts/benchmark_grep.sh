#!/usr/bin/env bash

BENCHMARK() {
  exec 3>&1 4>&2
  res=$(
    TIMEFORMAT='%E'  # get wall time in seconds
    LC_ALL=C         # use '.' instead of ',' as delimiter
    { time "$@" 1>&3 2>&4; } 2>&1
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

