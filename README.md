[![Docker](https://github.com/lfreist/x-search/actions/workflows/docker-image.yml/badge.svg)](https://github.com/lfreist/x-search/actions/workflows/docker-image.yml)

[![Linux (clang)](https://github.com/lfreist/x-search/actions/workflows/build-linux-clang.yml/badge.svg)](https://github.com/lfreist/x-search/actions/workflows/build-linux-clang.yml)
[![Linux (gcc)](https://github.com/lfreist/x-search/actions/workflows/build-linux-gcc.yml/badge.svg)](https://github.com/lfreist/x-search/actions/workflows/build-linux-gcc.yml)

[![Code Style (clang14)](https://github.com/lfreist/x-search/actions/workflows/clang-format.yml/badge.svg)](https://github.com/lfreist/x-search/actions/workflows/clang-format.yml)


# x-search
External string searching library (x-search) written in C++ (C++20)

## Build

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