#CC=gcc
#CXX=g++
CC=clang
CXX=clang++

CXXFLAGS=-Ofast -Wall -Wextra -g

all: ordered unordered
ordered: build/std_map build/rust_btreemap build/absl_btree_map build/judy
unordered: build/std_unordered_map build/absl_flat_hash_map build/rust_hashmap

# C++: std::map
build/std_map: test.cpp std_map.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ $^

# C++: std::unordered_map
build/std_unordered_map: test.cpp std_unordered_map.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rust: std::collections::BTreeMap
build/librust_btreemap.a: rust_btreemap.rs
	mkdir -p build
	rustc -g -O -o $@ $^

build/rust_btreemap: test.cpp build/librust_btreemap.a
	mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ $^ -pthread -ldl

# Rust: std::collections::HashMap
build/librust_hashmap.a: rust_hashmap.rs
	mkdir -p build
	rustc -g -O -o $@ $^

build/rust_hashmap: test.cpp build/librust_hashmap.a
	mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ $^ -pthread -ldl

# C++: absl::btree_map
# (git clone https://github.com/abseil/abseil-cpp)
build/absl_btree_map: test.cpp absl_btree_map.cc
	mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ $^ -I abseil-cpp/

# C++: absl::flat_hash_map
# (cd abseil-cpp/; bazel build //absl/...)
build/absl_flat_hash_map: test.cpp absl_flat_hash_map.cc
	mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ $^ -I abseil-cpp/ -pthread \
	  -Wl,--start-group $(shell find abseil-cpp/bazel-bin/absl/ -type f -name '*.a') -Wl,--end-group

# C: libjudy JudyL
# (apt install libjudy-dev)
build/judy.o: judy.c
	mkdir -p build
	$(CC) -g -c -Wall -Wextra -O2 -DNDEBUG -o $@ $^

build/judy: test.cpp build/judy.o
	mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ $^ -lJudy

.PHONY: clean all ordered unordered
clean:
	rm -rf build/

