[![Docker](https://github.com/lfreist/x-search/actions/workflows/docker-image.yml/badge.svg)](https://github.com/lfreist/x-search/actions/workflows/docker-image.yml)

[![Linux (clang)](https://github.com/lfreist/x-search/actions/workflows/build-linux-clang.yml/badge.svg)](https://github.com/lfreist/x-search/actions/workflows/build-linux-clang.yml)
[![Linux (gcc)](https://github.com/lfreist/x-search/actions/workflows/build-linux-gcc.yml/badge.svg)](https://github.com/lfreist/x-search/actions/workflows/build-linux-gcc.yml)

[![Thread Sanitizer](https://github.com/lfreist/x-search/actions/workflows/thread-sanitizer-test.yml/badge.svg)](https://github.com/lfreist/x-search/actions/workflows/thread-sanitizer-test.yml)
[![Address Sanitizer](https://github.com/lfreist/x-search/actions/workflows/address-sanitizer-test.yml/badge.svg)](https://github.com/lfreist/x-search/actions/workflows/address-sanitizer-test.yml)

[![Code Style (clang14)](https://github.com/lfreist/x-search/actions/workflows/clang-format.yml/badge.svg)](https://github.com/lfreist/x-search/actions/workflows/clang-format.yml)


# x-search
External string searching library (x-search) written in C++ (C++20)

# Build, Installation and Usage
### Dependencies

- libboost-program-options1.74-dev (only for examples)
- liblz4-dev
- libzstd-dev
- cmake
- g++ or clang


We refer to the corresponding Wiki entry: [Installation](https://github.com/lfreist/x-search/wiki/Installation)

# Example: building a (basic) grep-like executable using x-search

As a brief example on how to use x-search, we will create a small (very basic) grep-like executable:

```c++
// my_grep.cpp
#include <xsearch/xsearch.h>
#include <iostream>

int main(int argc, char** argv) {
  auto searcher = xs::extern_search<xs::lines>(argv[1], argv[2], false, 1);
  for (auto const& line : *searcher->getResult()) {
    std::cout << line << '\n';
  }
}
```

Now, just build it and link against xsearch as described [here]([here](https://github.com/lfreist/x-search/wiki/Installation#include-into-c-cmake-project-using-git-modules))

Done! We have created a grep-like command line search tool. Let's check if it can be as fast as GNU grep...

```bash
# GNU grep:
$ time grep Sherlock opensubtitles.en.txt > /tmp/grep.result

real    0m3.379s
user    0m2.525s
sys     0m0.843s

# Our implementation using x-search
$ time my_grep Sherlock opensubtitles.en.txt > /tmp/my_grep.result

real    0m1.154s
user    0m0.716s
sys     0m0.469s
```

## API
`x-search` provides a simple one-function API call to search on external files.

### One-Function-Call API
```cpp
#include <xsearch/xsearch.h>

// count number of matches:
auto res = xs::extern_search<xs::count>(pattern, file_path, meta_file_path, num_threads, max_num_readers);
```

> Besides `xs::count`, `xs::extern_search` is specialized for the following template arguments:
>
> - `xs::count_lines`: count lines containing a match
> - `xs::match_byte_offsets`: a vector of the byte offsets of all matches
> - `xs::line_byte_offsets`: a vector of the byte offsets of matching lines
> - `xs::line_indices`: a vector of the line indices of matching lines
> - `xs::lines`: a vector of lines (as std::string) containing the match

After calling `xs::extern_search`, the returned shared_ptr of the Searcher instance can be...
- ... joined (`res->join()`): the main thread sleeps until the search process finishes
- ... used to access already created results using the iterator:
   ```c++
  for (auto r : *res->getResult()) {
    ...
  }
   ```
- ... ignored: the threads started for the search are joined on destruction.