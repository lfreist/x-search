[![Docker](https://github.com/lfreist/x-search/actions/workflows/docker-image.yml/badge.svg)](https://github.com/lfreist/x-search/actions/workflows/docker-image.yml)

[![Linux (clang)](https://github.com/lfreist/x-search/actions/workflows/build-linux-clang.yml/badge.svg)](https://github.com/lfreist/x-search/actions/workflows/build-linux-clang.yml)
[![Linux (gcc)](https://github.com/lfreist/x-search/actions/workflows/build-linux-gcc.yml/badge.svg)](https://github.com/lfreist/x-search/actions/workflows/build-linux-gcc.yml)

[![Thread Sanitizer](https://github.com/lfreist/x-search/actions/workflows/thread-sanitizer-test.yml/badge.svg)](https://github.com/lfreist/x-search/actions/workflows/thread-sanitizer-test.yml)
[![Address Sanitizer](https://github.com/lfreist/x-search/actions/workflows/address-sanitizer-test.yml/badge.svg)](https://github.com/lfreist/x-search/actions/workflows/address-sanitizer-test.yml)

[![Code Style (clang14)](https://github.com/lfreist/x-search/actions/workflows/clang-format.yml/badge.svg)](https://github.com/lfreist/x-search/actions/workflows/clang-format.yml)


# x-search
External string searching library (x-search) written in C++ (C++20)

## Build

### Dependencies

- liblz4-dev
- libzstd-dev
- libboost-program-options1.74-dev
- cmake
- g++ or clang

1. Download x-search and navigate into the directory
    ```bash
    git clone https://github.com/lfreist/x-search
    cd x-search
    ```
2. Proceed with either Native- or Docker-Build as follows:
   1. Native
    ```
    git submodule update --init --recursive
    
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release .. && make -j $(nproc)
    ```
   2. Docker
    ```
    docker build -t leon-freist-bachelorprojekt .
    docker run -it -v $(pwd)/files:/inputfiles/input:ro --name leon-freist-bachelorprojekt leon-freist-bachelorprojekt
    ```

> You can also use the `Makefile` provided within the repository to
> build, test, benchmark, ... x-search.

## API
`x-search` provides a simple one-function API call to search on external files.

### Preprocess a file:
```cpp
#include <xsearch/xsearch.h>

xs::FilePreprocessor::preprocess(
      source_file_path, output_path, meta_file_path, compression_alg,
      compression_level, bytesNewLineMappingDistance, minBlockSize, overflow_size,
      display_progress);
```

### Perform Searches
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
- ... joined (`res->join()`): the main threads sleeps until the search process finished
- ... used to access already created results using the iterator (`for (auto value : *res->getResult()) {...}`)
- ... ignored: the threads started for the search by `Searcher` are joined on destruction.