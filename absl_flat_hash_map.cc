#include "absl/container/flat_hash_map.h"

extern "C" {
  #include "interface.h"
  const char *implementation_name = "absl::flat_hash_map";
}

typedef absl::flat_hash_map<Key, Value> Map;

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
size_t map_lookup_range(void *, Key, Key, size_t, KVPair *) {
  fprintf(stderr, "map_lookup_range on %s\n", implementation_name);
  exit(1);
}
