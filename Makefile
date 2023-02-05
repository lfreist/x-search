NAME=x-search

.PHONY: all test lib_test sfgrep_test check_style benchmark clean help

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

all: check_style build test benchmark clean

init:
	git submodule update --init --recursive 2>/dev/null

init_test_runs:
	if [ ! -d tmp ]; then mkdir tmp; fi

build: init
	cmake -B build -DCMAKE_BUILD_TYPE=Release -DRE2_BUILD_TESTING=off 2>/dev/null
	cmake --build build --config Release -j $(nproc) 2>/dev/null

build_benchmark: init
	cmake -B build-benchmark -DCMAKE_BUILD_TYPE=Benchmark -DRE2_BUILD_TESTING=off 2>/dev/null
	cmake --build build-benchmark --config Benchmark -j $(nproc) 2>/dev/null

test: lib_test sfgrep_test

lib_test: build init_test_runs
	ctest -C Release --test-dir build 2>/dev/null

sfgrep_test: build init_test_runs
	if [ ! -f tmp/1gb.dummy.txt ]; then python3 ./scripts/createTestFile.py --size 1 files/words.txt --output tmp/1gb.dummy.txt --progress; fi
	bash ./test/test_sfgrep.sh ./build tmp/1gb.dummy.txt

check_style:
	bash ./format_check.sh

benchmark: build_benchmark init_test_runs
	bash ./scripts/benchmark_pattern_size.sh
	bash ./scripts/benchmark_pattern_density.sh
	bash ./scripts/benchmark_file_size.sh
	bash ./scripts/benchmark_compression.sh
	bash ./scripts/benchmark_nl_mapping.sh

clean:
	rm -rf build
	rm -rf build-benchmark
	rm -rf tmp