#![crate_type = "staticlib"]

use std::collections::BTreeMap;
use std::ffi::c_void;
use std::mem::transmute;
use std::ops::Bound::Included;
use std::os::raw::c_char;

#[no_mangle]
pub static mut implementation_name: *const c_char =
    "Rust BTreeMap\0".as_bytes().as_ptr() as *const c_char;

type Key = u64;
type Value = u64;

type Map = BTreeMap<Key, Value>;

#[repr(C)]
pub struct KVPair {
    key: Key,
    value: Value,
}

#[no_mangle]
pub static map_size: usize = std::mem::size_of::<Map>();

#[no_mangle]
pub unsafe extern "C" fn map_alloc(m: *mut c_void) {
    let map = transmute::<*mut c_void, *mut Map>(m);
    std::ptr::write(map, Map::new());
}

#[no_mangle]
pub unsafe extern "C" fn map_free(m: *mut c_void) {
    let map = transmute::<*mut c_void, *mut Map>(m);
    map.drop_in_place();
}

#[no_mangle]
pub unsafe extern "C" fn map_assign(m: *mut c_void, key: Key, value: Value) {
    let map = transmute::<*mut c_void, *mut Map>(m);
    (*map).insert(key, value);
}

#[no_mangle]
pub unsafe extern "C" fn map_lookup(m: *mut c_void, key: Key, value_ptr: *mut Value) -> usize {
    let map = transmute::<*mut c_void, *mut Map>(m);
    if let Some(v) = (*map).get(&key) {
        if !value_ptr.is_null() {
            *value_ptr = *v;
        }
        return 1;
    }
    return 0;
}

#[no_mangle]
pub unsafe extern "C" fn map_delete(m: *mut c_void, key: Key, value_ptr: *mut Value) -> usize {
    let map = transmute::<*mut c_void, *mut Map>(m);
    if let Some(v) = (*map).remove(&key) {
        if !value_ptr.is_null() {
            *value_ptr = v;
        }
        return 1;
    }
    return 0;
}

#[no_mangle]
pub unsafe extern "C" fn map_lookup_range(
    m: *mut c_void,
    key_low: Key,
    key_high: Key,
    max_pairs_to_retrieve: usize,
    key_value_pairs: *mut KVPair,
) -> usize {
    let map = transmute::<*mut c_void, *mut Map>(m);
    let mut count: usize = 0;
    for (&key, &value) in (*map).range((Included(key_low), Included(key_high))) {
        if count == max_pairs_to_retrieve {
            break;
        }
        if !key_value_pairs.is_null() {
            *key_value_pairs.add(count) = KVPair {
                key: key,
                value: value,
            };
        }
        count += 1;
    }
    return count;
}
