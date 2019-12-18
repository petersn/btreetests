#include "absl/container/btree_map.h"

extern "C" {
  #include "interface.h"
}

typedef absl::btree_map<Key, Value> Map;

size_t map_size = sizeof (Map);

extern "C"
void map_alloc(void *m) {
  Map *map = (Map *) m;
  new (map) Map;
}

extern "C"
void map_free(void *m) {
  Map *map = (Map *) m;
  map->~Map();
}

extern "C"
void map_assign(void *m, Key key, Value value) {
  Map *map = (Map *) m;
  (*map)[key] = value;
}

extern "C"
size_t map_lookup(void *m, Key key, Value *value_ptr) {
  Map *map = (Map *) m;
  auto it = map->find(key);
  if (it != map->end()) {
    if (value_ptr) *value_ptr = it->second;
    return 1;
  } else {
    return 0;
  }
}

extern "C"
size_t map_delete(void *m, Key key, Value *value_ptr) {
  Map *map = (Map *) m;
  auto it = map->find(key);
  if (it != map->end()) {
    if (value_ptr) *value_ptr = it->second;
    map->erase(it);
    return 1;
  } else {
    return 0;
  }
}

extern "C"
size_t map_lookup_range(void *m, Key key_low, Key key_high,
                        size_t max_pairs_to_retrieve, KVPair *key_value_pairs) {
  Map *map = (Map *) m;
  auto it = map->lower_bound(key_low);
  size_t count;
  for (count = 0; count < max_pairs_to_retrieve; count++) {
    if (it == map->end()) break;
    if (it->first > key_high) break;
    if (key_value_pairs) {
      key_value_pairs[count] = {it->first, it->second};
    }
    ++it;
  }
  return count;
}
