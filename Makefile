NAME=x-search

.PHONY: all test lib_test grep_test check_style benchmark clean help

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
	@echo " - 'make build'"
	@echo " - 'make benchmark'"
	@echo " - 'make test'"
	@echo " - ..."
	@echo "--------------------------------------------------------------------------------"

all: check_style test benchmark clean

init:
	git submodule update --init --recursive 2>/dev/null

init_test_runs:
	if [ ! -d tmp ]; then mkdir tmp; fi

build: init
	cmake -B build -DCMAKE_BUILD_TYPE=Release -DRE2_BUILD_TESTING=off 2>/dev/null
	cmake --build build --config Release -j $(nproc) 2>/dev/null

build_benchmark: init
	cmake -B build-benchmark -DCMAKE_BUILD_TYPE=Benchmark -DRE2_BUILD_TESTING=off -DXS_BENCHMARKS=ON 2>/dev/null
	cmake --build build-benchmark --config Benchmark -j $(nproc) 2>/dev/null

build_sanitizer: init
	cmake -B build-thread-sanitizer -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS="-fsanitize=thread" -DCMAKE_CXX_COMPILER=/usr/bin/clang++-15 -DRE2_BUILD_TESTING=off
	cmake --build build-thread-sanitizer -j $(nproc) 2>/dev/null
	cmake -B build-address-sanitizer -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS="-fsanitize=address" -DCMAKE_CXX_COMPILER=/usr/bin/clang++-15 -DRE2_BUILD_TESTING=off
	cmake --build build-address-sanitizer -j $(nproc) 2>/dev/null

build_exe_test: init
	cmake -B build-exe-test-thread-sanitizer -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS="-fsanitize=thread" -DCMAKE_CXX_COMPILER=/usr/bin/clang++-15 -DRE2_BUILD_TESTING=off -DXS_EXE_TESTS=ON
	cmake --build build-exe-test-thread-sanitizer -j $(nproc) 2>/dev/null
	cmake -B build-exe-test-address-sanitizer -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS="-fsanitize=address" -DCMAKE_CXX_COMPILER=/usr/bin/clang++-15 -DRE2_BUILD_TESTING=off -DXS_EXE_TESTS=ON
	cmake --build build-exe-test-address-sanitizer -j $(nproc) 2>/dev/null

test: grep_test preprocessor_test lib_test

lib_test: build_sanitizer init_test_runs
	ctest -C Release --test-dir build-thread-sanitizer
	ctest -C Release --test-dir build-address-sanitizer

grep_test: build_exe_test init_test_runs
	ctest --test-dir build-exe-test-thread-sanitizer --label-regex grep_exe_test
	ctest --test-dir build-exe-test-address-sanitizer --label-regex grep_exe_test

preprocessor_test: build_exe_test init_test_runs
	ctest --test-dir build-exe-test-thread-sanitizer --label-regex preprocessor_exe_test
	ctest --test-dir build-exe-test-address-sanitizer --label-regex preprocessor_exe_test

check_style:
	bash ./format_check.sh

benchmark: build_benchmark init_test_runs
	sudo ctest --test-dir build-benchmark --label-regex benchmark_

clean:
	rm -rf build
	rm -rf build-benchmark
	rm -rf build-address-sanitizer
	rm -rf build-thread-sanitizer
	rm -rf build-exe-test-address-sanitizer
	rm -rf build-exe-test-thread-sanitizer