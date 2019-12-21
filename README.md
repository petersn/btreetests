# btreetests

There are five main sections of tests.
The score is the reciprocal of the weighted geometric mean of the times on each test.

* Linear insertion and lookup, forward and backward. (28 tests, total weight: ½)
* Uniformly random insertion, lookup, and deletion. (5 tests, total weight: 1)
* Zipf distributed insertion, lookup, and deletion. (5 tests, total weight: 1)
* Uniformly random insertion/deletion, and range queries. (6 tests, total weight: 1)
* Zipf distributed insertion/deletion, and range queries. (6 tests, total weight: 1)

The result is a number of points proportional to speed (that is, taking half the time on every test doubles your score).
Testing the example implementation (which just uses `std::map`) on my laptop's i7-9750H CPU @ 2.60GHz I get:
```
$ make build/std_map && time ./build/std_map
mkdir -p build
clang++ -Ofast -Wall -Wextra -g -o build/std_map test.cpp std_map.cpp
Testing std::map (with range queries)
  Checking correctness... pass.
  Map size in bytes: 48
  Section 1: Linear insertion points:   14.47
  Section 2: Random usage points:       20.40
      Uniform random usage points:      18.45
      Zipf random usage points:         22.56
  Section 3: Random range query points: 16.58
      Uniform random usage points:      13.79
      Zipf random usage points:         19.94

  Benchmark points: 19.22

real	0m3.944s
user	0m3.820s
sys	0m0.124s

```

Pass the flag `-no_range_queries` to skip range queries.

## Interface

The file `interface.h` specifies the interface your map must conform to.
The file `example.cpp` gives an example implementation of this interface.

The interface is:

```c
extern const char* implementation_name;

typedef uint64_t Key;
typedef uint64_t Value;

typedef struct KVPair {
    Key key;
    Value value;
} KVPair;

// Set to the size (in bytes) of your map struct.
extern size_t map_size;

// Allocate/free a map struct in place.
void map_alloc(void* m);
void map_free(void* m);

// m[key] = value
void map_assign(void* m, Key key, Value value);

// Should return 1 if key is found, and 0 otherwise.
// If they key is found and value_ptr != nullptr, then write found value to *value_ptr.
size_t map_lookup(void* m, Key key, Value* value_ptr);

// Identical to map_lookup, but also deletes the map entry if present.
size_t map_delete(void* m, Key key, Value* value_ptr);

// Find the first max_pairs_to_retrieve key/value pairs between key_low and key_high inclusive.
// Return the number of pairs found. If key_value_pairs != nullptr then write the results
// to key_value_pairs[0], key_value_pairs[1], ...
size_t map_lookup_range(
    void* m,
    Key key_low,
    Key key_high,
    size_t max_pairs_to_retrieve,
    KVPair* key_value_pairs
);
```

## License

All code here is CC0 (public domain).

