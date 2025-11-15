#pragma once

#include "xstd_core.h"
#include "xstd_alloc.h"
#include "xstd_buffer.h"
#include "xstd_string.h"
#include "xstd_result.h"
#include "xstd_error.h"
#include "xstd_mem.h"

#if _X_ARCH_64BIT
static inline u64 _hashmap_fnv1a64(const void *key, u64 len)
{
    const u8 *data = (const u8 *)key;

    u64 hash = 0xcbf29ce484222325ULL;
    for (u64 i = 0; i < len; ++i)
        hash = (hash ^ data[i]) * 0x100000001b3ULL;

    return hash;
}
#else
static inline u32 _hashmap_fnv1a32(const void *key, u32 len)
{
    const u8 *data = (const u8 *)key;

    u32 hash = 0x811C9DC5U;
    for (u32 i = 0; i < len; ++i)
        hash = (hash ^ data[i]) * 0x01000193U;

    return hash;
}
#endif

typedef struct _hashmap_entry
{
    u64 _hash;
    Buffer _key;
    void *_value;
    struct _hashmap_entry *_next;
} _HashMapEntry;

typedef struct _hashmap
{
    _HashMapEntry **_buckets;
    u64 _bucketCount;
    u64 _size;
    u64 _valueSize;
    Allocator _allocator;
} HashMap;

// If error != ERR_OK, value is invalid.
typedef struct _result_hashmap
{
    HashMap value;
    Error error;
} ResultHashMap;

#define _X_HASHMAP_INITIAL_SIZE 32
#define _X_HASHMAP_LOAD_FACTOR_NUM 3
#define _X_HASHMAP_LOAD_FACTOR_DEN 4

/**
 * @brief Creates a HashMap allowing storing of key-value pairs, with constant time fetching by using hashing.
 *
 * Prefer `HashMapInitT` for a type safe alternative.
 *
 * ```c
 * ResultHashMap mapRes = hashmap_init(&c_alloc, sizeof(u64), 16);
 * if (mapRes.error.code) // Error!
 * HashMap map = mapRes.value;
 * ConstStr key = "This is an example key";
 * u64 value = 52;
 * Error err = hashmap_set_str(&map, key, &value);
 * if (err.code) // Error!
 * u64 outVal;
 * err = hashmap_get_str(&map, key, &outVal);
 * if (err.code) // Error!
 * // outVal == 52
 * hashmap_deinit(&map);
 * ```
 * @param alloc
 * @param valueByteSize
 * @param initialAllocCount
 * @return ResultHashMap
 */
static inline ResultHashMap hashmap_init(Allocator *alloc, u64 valueByteSize, u64 initialAllocCount)
{
    if (!alloc || valueByteSize == 0)
        return (ResultHashMap){
            .value = {0},
            .error = X_ERR_EXT("hashmap", "hashmap_init", ERR_INVALID_PARAMETER, "null or invalid arg"),
        };

    if (initialAllocCount < _X_HASHMAP_INITIAL_SIZE)
        initialAllocCount = _X_HASHMAP_INITIAL_SIZE;

    HashMap map;
    map._buckets = (_HashMapEntry **)alloc->alloc(alloc, sizeof(_HashMapEntry *) * initialAllocCount);

    if (!map._buckets)
        return (ResultHashMap){
            .value = {0},
            .error = X_ERR_EXT("hashmap", "hashmap_init", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    for (u64 i = 0; i < initialAllocCount; ++i)
        map._buckets[i] = 0;

    map._bucketCount = initialAllocCount;
    map._size = 0;
    map._valueSize = valueByteSize;
    map._allocator = *alloc;

    return (ResultHashMap){
        .value = map,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Type safe variant of `hashmap_init`
 *
 * ```c
 * ResultHashMap mapRes = HashMapInitT(u64, &c_alloc);
 * if (mapRes.error.code) // Error!
 * HashMap map = mapRes.value;
 * ConstStr key = "This is an example key";
 * u64 value = 52;
 * Error err = hashmap_set_str(&map, key, &value);
 * if (err.code) // Error!
 * u64 outVal;
 * err = hashmap_get_str(&map, key, &outVal);
 * if (err.code) // Error!
 * // outVal == 52
 * hashmap_deinit(&map);
 * ```
 */
#define HashMapInitT(T, allocPtr) hashmap_init((allocPtr), sizeof(T), _X_HASHMAP_INITIAL_SIZE)

/**
 * @brief Frees the memory allocated for the HashMap.
 *
 * Invalidates the HashMap, usage of it after call to this function is undefined behavior.
 *
 * @param map
 */
static inline void hashmap_deinit(HashMap *map)
{
    if (!map || !map->_buckets)
        return;

    Allocator *alloc = &map->_allocator;
    for (u64 i = 0; i < map->_bucketCount; ++i)
    {
        _HashMapEntry *entry = map->_buckets[i];

        while (entry)
        {
            _HashMapEntry *next = entry->_next;

            if (entry->_key.bytes)
                alloc->free(alloc, entry->_key.bytes);

            if (entry->_value)
                alloc->free(alloc, entry->_value);

            alloc->free(alloc, entry);
            entry = next;
        }
    }
    alloc->free(alloc, map->_buckets);
    *map = (HashMap){0};
}

static inline void _hashmap_memcpy(HashMap *h, const void *srcPtr, void *dstPtr)
{
    u64 rest = h->_valueSize;
    mem_copy(dstPtr, srcPtr, rest);
}

static inline Bool _hashmap_key_equals(Buffer a, Buffer b)
{
    if (a.size != b.size)
        return false;

    if (a.size == 0)
        return true;

    if (!a.bytes || !b.bytes)
        return false;

    for (u64 i = 0; i < a.size; ++i)
        if (a.bytes[i] != b.bytes[i])
            return false;

    return true;
}

static inline u64 _hashmap_bucket_idx(HashMap *map, u64 hash)
{
    return hash % map->_bucketCount;
}

static inline Bool _hashmap_is_invalid_idx(HashMap *map, u64 idx)
{
    if (!map->_bucketCount)
        return true;

    if (idx > map->_bucketCount - 1)
        return true;

    return false;
}

static inline Error _hashmap_set(HashMap *map, Buffer key, const void *value, u64 hash)
{
    Allocator *alloc = &map->_allocator;

    u64 idx = _hashmap_bucket_idx(map, hash);
    if (_hashmap_is_invalid_idx(map, idx))
    {
        return X_ERR_EXT("hashmap", "_hashmap_set", ERR_RANGE_ERROR, "inexistent entry");
    }

    _HashMapEntry *entry = map->_buckets[idx];

    while (entry)
    {
        if (entry->_hash == hash && _hashmap_key_equals(entry->_key, key))
        {
            if (map->_valueSize == 0)
                return X_ERR_OK;

            if (!entry->_value)
            {
                entry->_value = alloc->alloc(alloc, map->_valueSize);
                if (!entry->_value)
                    return X_ERR_EXT("hashmap", "_hashmap_set", ERR_OUT_OF_MEMORY, "alloc failure");
            }

            _hashmap_memcpy(map, value, entry->_value);
            return X_ERR_OK;
        }
        entry = entry->_next;
    }

    _HashMapEntry *newEntry = (_HashMapEntry *)alloc->alloc(alloc, sizeof(_HashMapEntry));

    if (!newEntry)
        return X_ERR_EXT("hashmap", "_hashmap_set", ERR_OUT_OF_MEMORY, "alloc failure");

    newEntry->_hash = hash;
    u64 keyAllocSize = key.size ? key.size : 1;
    newEntry->_key = (Buffer){
        .bytes = (i8 *)alloc->alloc(alloc, keyAllocSize),
        .size = key.size,
    };

    if (!newEntry->_key.bytes)
    {
        alloc->free(alloc, newEntry);
        return X_ERR_EXT("hashmap", "_hashmap_set", ERR_OUT_OF_MEMORY, "alloc failure");
    }

    if (key.size)
    {
        mem_copy(newEntry->_key.bytes, key.bytes, key.size);
    }
    else
    {
        newEntry->_key.bytes[0] = 0;
    }

    if (map->_valueSize == 0)
    {
        newEntry->_value = NULL;
    }
    else
    {
        newEntry->_value = alloc->alloc(alloc, map->_valueSize);
        if (!newEntry->_value)
        {
            alloc->free(alloc, newEntry->_key.bytes);
            alloc->free(alloc, newEntry);
            return X_ERR_EXT("hashmap", "_hashmap_set", ERR_OUT_OF_MEMORY, "alloc failure");
        }

        _hashmap_memcpy(map, value, newEntry->_value);
    }

    newEntry->_next = map->_buckets[idx];
    map->_buckets[idx] = newEntry;
    map->_size += 1;
    return X_ERR_OK;
}

static inline Error _hashmap_rehash(HashMap *map, u64 newBucketCount)
{
    if (newBucketCount == 0)
        return X_ERR_EXT("hashmap", "_hashmap_rehash", ERR_INVALID_PARAMETER, "new bucket count is 0");

    Allocator *alloc = &map->_allocator;
    _HashMapEntry **newBuckets = (_HashMapEntry **)alloc->alloc(alloc, sizeof(_HashMapEntry *) * newBucketCount);

    if (!newBuckets)
        return X_ERR_EXT("hashmap", "_hashmap_rehash", ERR_OUT_OF_MEMORY, "alloc failure");

    for (u64 i = 0; i < newBucketCount; ++i)
        newBuckets[i] = 0;

    // Move all entries
    for (u64 i = 0; i < map->_bucketCount; ++i)
    {
        _HashMapEntry *entry = map->_buckets[i];
        while (entry)
        {
            _HashMapEntry *next = entry->_next;
            u64 idx = entry->_hash % newBucketCount;
            entry->_next = newBuckets[idx];
            newBuckets[idx] = entry;
            entry = next;
        }
    }
    alloc->free(alloc, map->_buckets);
    map->_buckets = newBuckets;
    map->_bucketCount = newBucketCount;
    return X_ERR_OK;
}

/**
 * @brief Sets or overwrites value for provided `key` of type `Buffer`
 *
 * @param map
 * @param key
 * @param value
 * @return Error
 */
static inline Error hashmap_set(HashMap *map, Buffer key, const void *value)
{
    if (!map || !map->_buckets)
        return X_ERR_EXT("hashmap", "hashmap_set", ERR_INVALID_PARAMETER, "null or invalid arg");

    if (key.size > 0 && !key.bytes)
        return X_ERR_EXT("hashmap", "hashmap_set", ERR_INVALID_PARAMETER, "null key buffer");

    if (map->_valueSize > 0 && !value)
        return X_ERR_EXT("hashmap", "hashmap_set", ERR_INVALID_PARAMETER, "null value buffer");

    if ((map->_size + 1) * _X_HASHMAP_LOAD_FACTOR_DEN > map->_bucketCount * _X_HASHMAP_LOAD_FACTOR_NUM)
    {
        Error err = _hashmap_rehash(map, map->_bucketCount * 2);
        if (err.code != ERR_OK)
            return err;
    }
    static const i8 _x_hashmap_empty_key = 0;
    const i8 *hashKeyBytes = key.bytes ? key.bytes : &_x_hashmap_empty_key;
    return _hashmap_set(map, key, value,
        #if _X_ARCH_64BIT
        _hashmap_fnv1a64(hashKeyBytes, key.size)
        #else
        _hashmap_fnv1a32(hashKeyBytes, key.size)
        #endif
    );
}

#define HashMapSetBuffT(T, mapPtr, keyBuff, valPtr) \
    { \
        T *mapItemTypeCheck = (valPtr); \
        (void)mapItemTypeCheck; \
        hashmap_set((mapPtr), (keyBuff), (valPtr)); \
    }

/**
 * @brief Sets or overwrites value for provided `key` of type `String`
 *
 * @param map
 * @param key
 * @param value
 * @return Error
 */
static inline Error hashmap_set_str(HashMap *map, ConstStr key, const void *value)
{
    if (!key)
        return X_ERR_EXT("hashmap", "hashmap_set_str", ERR_INVALID_PARAMETER, "null key");

    Buffer keyBuff = {.bytes = (i8 *)key, .size = string_size(key)};
    return hashmap_set(map, keyBuff, value);
}

#define HashMapSetStrT(T, mapPtr, keyStr, valPtr) \
    { \
        T *mapItemTypeCheck = (valPtr); \
        (void)mapItemTypeCheck; \
        hashmap_set_str((mapPtr), (keyStr), (valPtr)); \
    }

/**
 * @brief Fetches a value from a provided `key` of type `Buffer`
 *
 * If the map does not contain a value for the provided key, will return the Error ERR_FAILED
 *
 * @param map
 * @param key
 * @param value
 * @return Error
 */
static inline Error hashmap_get(HashMap *map, Buffer key, void *outValue)
{
    if (!map || !map->_buckets)
        return X_ERR_EXT("hashmap", "hashmap_get", ERR_INVALID_PARAMETER, "null or invalid arg");

    if (key.size > 0 && !key.bytes)
        return X_ERR_EXT("hashmap", "hashmap_get", ERR_INVALID_PARAMETER, "null key buffer");

    static const i8 _x_hashmap_empty_key = 0;
    const i8 *hashKeyBytes = key.bytes ? key.bytes : &_x_hashmap_empty_key;

    #if _X_ARCH_64BIT
    u64 hash = _hashmap_fnv1a64(hashKeyBytes, key.size);
    #else
    u64 hash = _hashmap_fnv1a32(hashKeyBytes, key.size);
    #endif

    u64 idx = _hashmap_bucket_idx(map, hash);

    if (_hashmap_is_invalid_idx(map, idx))
    {
        return X_ERR_EXT("hashmap", "hashmap_get", ERR_RANGE_ERROR, "inexistent key");
    }

    _HashMapEntry *entry = map->_buckets[idx];
    while (entry)
    {
        if (entry->_hash == hash && _hashmap_key_equals(entry->_key, key))
        {
            if (outValue && entry->_value)
            {
                _hashmap_memcpy(map, entry->_value, outValue);
            }
            return X_ERR_OK;
        }
        entry = entry->_next;
    }
    return X_ERR_EXT("hashmap", "hashmap_get", ERR_RANGE_ERROR, "inexistent key");
}

#define HashMapGetBuffT(T, mapPtr, keyBuff, outPtr) \
    { \
        T *mapItemTypeCheck = (outPtr); \
        (void)mapItemTypeCheck; \
        hashmap_get((mapPtr), (keyBuff), (outPtr)); \
    }

/**
 * @brief Fetches a value from a provided `key` of type `String`
 *
 * Prefer `HashMapGetStrT` as a type safe alternative.
 *
 * If the map does not contain a value for the provided key, will return the Error ERR_FAILED
 *
 * @param map
 * @param key
 * @param value
 * @return Error
 */
static inline Error hashmap_get_str(HashMap *map, ConstStr key, void *outValue)
{
    Buffer keyBuff = {.bytes = (i8 *)key, .size = string_size(key)};
    return hashmap_get(map, keyBuff, outValue);
}

/**
 * @brief Type safe variant of `hashmap_get_str`
 *
 * Fetches a value from a provided `key` of type `String`
 *
 * If the map does not contain a value for the provided key, will return the Error ERR_FAILED
 *
 * @param map
 * @param key
 * @param value
 * @return Error
 */
#define HashMapGetStrT(T, mapPtr, keyStr, outPtr) \
    { \
        T *mapItemTypeCheck = (outPtr); \
        (void)mapItemTypeCheck; \
        hashmap_get_str((mapPtr), (keyStr), (outPtr)); \
    }

/**
 * @brief Removes a value associated with the provided key of type `Buffer`
 *
 * @param map
 * @param key
 * @return Error
 */
static inline Error hashmap_remove(HashMap *map, Buffer key)
{
    if (!map || !map->_buckets || !key.bytes)
        return X_ERR_EXT("hashmap", "hashmap_remove", ERR_INVALID_PARAMETER, "null or invalid arg");

    #if _X_ARCH_64BIT
    u64 hash = _hashmap_fnv1a64(key.bytes, key.size);
    #else
    u64 hash = _hashmap_fnv1a32(key.bytes, key.size);
    #endif

    u64 idx = _hashmap_bucket_idx(map, hash);

    if (_hashmap_is_invalid_idx(map, idx))
    {
        return X_ERR_EXT("hashmap", "hashmap_remove", ERR_RANGE_ERROR, "inexistent key");
    }

    _HashMapEntry **prev = &map->_buckets[idx];
    _HashMapEntry *entry = map->_buckets[idx];

    while (entry)
    {
        if (entry->_hash == hash && _hashmap_key_equals(entry->_key, key))
        {
            *prev = entry->_next;
            Allocator *alloc = &map->_allocator;

            if (entry->_key.bytes)
                alloc->free(alloc, entry->_key.bytes);

            if (entry->_value)
                alloc->free(alloc, entry->_value);

            alloc->free(alloc, entry);
            map->_size -= 1;
            return X_ERR_OK;
        }
        prev = &entry->_next;
        entry = entry->_next;
    }
    return X_ERR_EXT("hashmap", "hashmap_remove", ERR_RANGE_ERROR, "inexistent key");
}

/**
 * @brief Removes a value associated with the provided key of type `String`
 *
 * @param map
 * @param key
 * @return Error
 */
static inline Error hashmap_remove_str(HashMap *map, ConstStr key)
{
    Buffer keyBuff = {.bytes = (i8 *)key, .size = string_size(key)};
    return hashmap_remove(map, keyBuff);
}

/**
 * @brief Calls a function for each key-value pairs in the map.
 *
 * @param map
 * @param func
 * @param userArg
 */
static inline void hashmap_for_each(HashMap *map, void (*func)(Buffer key, void *value, void *userArg), void *userArg)
{
    if (!map || !map->_buckets || !func)
        return;

    for (u64 i = 0; i < map->_bucketCount; ++i)
    {
        _HashMapEntry *entry = map->_buckets[i];
        while (entry)
        {
            func(entry->_key, entry->_value, userArg);
            entry = entry->_next;
        }
    }
}

/**
 * @brief Returns the count of key-value pairs in the map.
 *
 * @param map
 * @return u64
 */
static inline u64 hashmap_size(HashMap *map)
{
    if (!map)
        return 0;

    return map->_size;
}
