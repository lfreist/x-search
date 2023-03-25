NAME=x-search

.PHONY: all test check_style clean help

help:
	@echo "--------------------------------------------------------------------------------"
	@echo "                                    x-search"
	@echo "External string searching library written in C++ (C++20)"
	@echo "Undergraduate Thesis, 2023"
	@echo "Author    : Leon Freist <freist@informatik.uni-freiburg.de>"
	@echo "URL       : https://github.com/lfreist/x-search"
	@echo "Examiner  : Prof. Dr. Hannah Bast"
	@echo "Supervisor: Johannes Kalmbach"
	@echo "--------------------------------------------------------------------------------"
	@echo "The following commands are available:"
	@echo " - 'make build'             build x-search into ./build/"
	@echo " - 'make check_style'       check the style of all source files"
	@echo " - 'make test'              run tests"
	@echo "--------------------------------------------------------------------------------"

all: check_style build test clean

init:
	git submodule update --init --recursive 2>/dev/null

build: init
	cmake -B build -DCMAKE_BUILD_TYPE=Release -DRE2_BUILD_TESTING=off 2>/dev/null
	cmake --build build --config Release -j $(nproc) 2>/dev/null

build_sanitizer: init
	cmake -B build-thread-sanitizer -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS="-fsanitize=thread" -DCMAKE_CXX_COMPILER=/usr/bin/clang++-15 -DRE2_BUILD_TESTING=off
	cmake --build build-thread-sanitizer -j $(nproc) 2>/dev/null
	cmake -B build-address-sanitizer -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS="-fsanitize=address" -DCMAKE_CXX_COMPILER=/usr/bin/clang++-15 -DRE2_BUILD_TESTING=off
	cmake --build build-address-sanitizer -j $(nproc) 2>/dev/null

test: build_sanitizer
	ctest -C Release --test-dir build-thread-sanitizer
	ctest -C Release --test-dir build-address-sanitizer

check_style:
	bash ./format_check.sh

clean:
	rm -rf build
	rm -rf build-benchmark
	rm -rf build-address-sanitizer
	rm -rf build-thread-sanitizer