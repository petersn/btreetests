#include "stdlib.h"
#include "interface.h"
#include <Judy.h>

const char *implementation_name = "JudyL";

typedef void *Map;

size_t map_size = sizeof (Map);

void map_alloc(void *m) {
  Map *map = (Map *) m;
  *map = 0;
}

void map_free(void *m) {
  Map *map = (Map *) m;
  int rc;
  JLFA(rc, *map);
}

void map_assign(void *m, Key key, Value value) {
  Map *map = (Map *) m;
  Value *ptr;
  JLI(ptr, *map, key);
  *ptr = value;
}

size_t map_lookup(void *m, Key key, Value *value_ptr) {
  Map *map = (Map *) m;
  Value *ptr;
  JLG(ptr, *map, key);
  if (ptr) {
    if (value_ptr) *value_ptr = *ptr;
    return 1;
  } else {
    return 0;
  }
}

size_t map_delete(void *m, Key key, Value *value_ptr) {
  Map *map = (Map *) m;
  Value *ptr;
  JLG(ptr, *map, key);
  if (ptr) {
    if (value_ptr) *value_ptr = *ptr;
    int rc;
    JLD(rc, *map, key);
    return 1;
  } else {
    return 0;
  }
}

size_t map_lookup_range(void *m, Key key_low, Key key_high,
                        size_t max_pairs_to_retrieve, KVPair *key_value_pairs) {
  Map *map = (Map *) m;
  Key key = key_low;
  Value *ptr;
  JLF(ptr, *map, key);
  size_t i;
  for (i = 0; i < max_pairs_to_retrieve; i++) {
    if (!ptr) break;
    if (key > key_high) break;
    if (key_value_pairs) {
      key_value_pairs[i] = (KVPair){.key = key, .value = *ptr};
    }
    JLN(ptr, *map, key);
  }
  return i;
}

