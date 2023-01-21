# Questions

## Reading with multiple threads:
1. run `./sfgrep [...] -j 1`:
   1. reading takes ~20s for 11gb
2. run `./sfgrep [...] -j 2`:
   1. reading takes ~19s per thread for 11gb