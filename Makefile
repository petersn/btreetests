
CXXFLAGS=-Ofast -Wall -Wextra
CXXFLAGS+=-g

all: example rust absl

# C++: std::map
example: test.cpp example.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rust: std::collections::BTreeMap
librust.a: rust.rs
	rustc -g -O rust.rs

rust: test.cpp librust.a
	$(CXX) $(CXXFLAGS) -o $@ $^ -pthread -ldl

# C++: absl::btree_map
# (git clone https://github.com/abseil/abseil-cpp)
absl: test.cpp absl.cc
	$(CXX) $(CXXFLAGS) -o $@ $^ -I abseil-cpp/

.PHONY: clean
clean:
	rm -f *.o *.a
	rm -f example rust absl

