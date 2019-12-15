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

extern size_t map_size;

void map_alloc(void* m);
void map_free(void* m);
void map_assign(void* m, Key key, Value value);
size_t map_lookup(void* m, Key key, Value* value_ptr);
size_t map_delete(void* m, Key key, Value* value_ptr);
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

