// Example implementation.

#include <cstdint>
#include <map>

extern "C" {
	#include "interface.h"
}

typedef std::map<Key, Value> Map;

size_t map_size = sizeof(Map);

extern "C" void map_alloc(void* m) {
	new(m) Map;
}

extern "C" void map_free(void* m) {
	static_cast<Map*>(m)->~map();
}

extern "C" void map_assign(void* m, Key key, Value value) {
	(*static_cast<Map*>(m))[key] = value;
}

template <bool do_erase>
size_t map_lookup_erase(void* m, Key key, Value* value_ptr) {
	auto _m = static_cast<Map*>(m);
	auto it = _m->find(key);
	if (it == _m->end())
		return 0;
	if (value_ptr != nullptr)
		*value_ptr = (*it).second;
	if (do_erase)
		_m->erase(it);
	return 1;
}

extern "C" size_t map_lookup(void* m, Key key, Value* value_ptr) {
	return map_lookup_erase<false>(m, key, value_ptr);
}

extern "C" size_t map_delete(void* m, Key key, Value* value_ptr) {
	return map_lookup_erase<true>(m, key, value_ptr);
}

extern "C" size_t map_lookup_range(
	void* m,
	Key key_low,
	Key key_high,
	size_t max_pairs_to_retrieve,
	KVPair* key_value_pairs
) {
	auto _m = static_cast<Map*>(m);
	auto it = _m->lower_bound(key_low);
	size_t count = 0;
	while (it != _m->end() and (*it).first <= key_high and max_pairs_to_retrieve--) {
		auto p = *it++;
		if (key_value_pairs != nullptr)
			*key_value_pairs++ = {p.first, p.second};
		count++;
	}
	return count;
}

