# btreetests

There are five main tests.
The score is the reciprocal of the weighted geometric mean of the times on each test.

* Linear insertion and lookup, forward and backward. (weight: ½)
* Uniformly random insertion, lookup, and deletion. (weight: 1)
* Zipf distributed insertion, lookup, and deletion. (weight: 1)
* Uniformly random insertion/deletion, and uniformly random range queries. (weight: 1)
* Zipf distributed insertion/deletion, and uniformly random range queries. (weight: 1)

Each individual test runs over a range of parameters.

## Interface

The file `interface.h` specifies the interface your map must conform to.
The file `example.cpp` gives an example implementation of this interface.

The interface is:

```c
typedef int64_t Key;
typedef int64_t Value;

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
// Return the number of pairs found. If key_value_pairs !== nullptr then write the results
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

