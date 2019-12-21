#![crate_type = "staticlib"]

use std::collections::HashMap;
use std::ffi::c_void;
use std::mem::transmute;
use std::os::raw::c_char;

#[no_mangle]
pub static mut implementation_name: *const c_char =
    "Rust HashMap\0".as_bytes().as_ptr() as *const c_char;

type Key = u64;
type Value = u64;

type Map = HashMap<Key, Value>;

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
    _m: *mut c_void,
    _key_low: Key,
    _key_high: Key,
    _max_pairs_to_retrieve: usize,
    _key_value_pairs: *mut KVPair,
) -> usize {
    panic!("lookup_range on hashmap!");
}
