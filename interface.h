
#include <stdint.h>

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

