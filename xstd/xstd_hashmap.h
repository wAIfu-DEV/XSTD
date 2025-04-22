#pragma once

#include "xstd_core.h"
#include "xstd_alloc.h"
#include "xstd_buffer.h"
#include "xstd_string.h"
#include "xstd_result.h"
#include "xstd_error.h"

u64 __hashmap_fnv1a64(const void *key, u64 len)
{
    const u8 *data = (const u8 *)key;

    u64 hash = 0xcbf29ce484222325ULL;
    for (u64 i = 0; i < len; ++i)
        hash = (hash ^ data[i]) * 0x100000001b3ULL;
    
    return hash;
}

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
 * if (mapRes.error) // Error!
 * HashMap map = mapRes.value;
 * ConstStr key = "This is an example key";
 * u64 value = 52;
 * Error err = hashmap_set_str(&map, key, &value);
 * if (err) // Error!
 * u64 outVal;
 * err = hashmap_get_str(&map, key, &outVal);
 * if (err) // Error!
 * // outVal == 52
 * hashmap_deinit(&map);
 * ```
 * @param alloc 
 * @param valueByteSize 
 * @param initialAllocCount 
 * @return ResultHashMap 
 */
ResultHashMap hashmap_init(Allocator *alloc, u64 valueByteSize, u64 initialAllocCount)
{
    if (!alloc || valueByteSize == 0)
        return (ResultHashMap){
            .value = {0},
            .error = ERR_INVALID_PARAMETER,
        };

    if (initialAllocCount < _X_HASHMAP_INITIAL_SIZE)
        initialAllocCount = _X_HASHMAP_INITIAL_SIZE;

    HashMap map;
    map._buckets = (_HashMapEntry **)alloc->alloc(alloc, sizeof(_HashMapEntry *) * initialAllocCount);

    if (!map._buckets)
        return (ResultHashMap){
            .value = {0},
            .error = ERR_OUT_OF_MEMORY,
        };

    for (u64 i = 0; i < initialAllocCount; ++i)
        map._buckets[i] = 0;

    map._bucketCount = initialAllocCount;
    map._size = 0;
    map._valueSize = valueByteSize;
    map._allocator = *alloc;

    return (ResultHashMap){
        .value = map,
        .error = ERR_OK,
    };
}

/**
 * @brief Type safe variant of `hashmap_init`
 * 
 * ```c
 * ResultHashMap mapRes = HashMapInitT(u64, &c_alloc);
 * if (mapRes.error) // Error!
 * HashMap map = mapRes.value;
 * ConstStr key = "This is an example key";
 * u64 value = 52;
 * Error err = hashmap_set_str(&map, key, &value);
 * if (err) // Error!
 * u64 outVal;
 * err = hashmap_get_str(&map, key, &outVal);
 * if (err) // Error!
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
void hashmap_deinit(HashMap *map)
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

ibool __hashmap_key_equals(Buffer a, Buffer b)
{
    if (a.size != b.size)
        return false;

    if (!a.bytes || !b.bytes)
        return false;

    for (u64 i = 0; i < a.size; ++i)
        if (a.bytes[i] != b.bytes[i])
            return false;
    
    return true;
}

u64 __hashmap_bucket_idx(HashMap *map, u64 hash)
{
    return hash % map->_bucketCount;
}

ibool __hashmap_is_invalid_idx(HashMap *map, u64 idx)
{
    if (!map->_bucketCount)
        return true;

    if (idx > map->_bucketCount - 1)
        return true;
    
    return false;
}

Error __hashmap_set(HashMap *map, Buffer key, const void *value, u64 hash)
{
    Allocator *alloc = &map->_allocator;

    u64 idx = __hashmap_bucket_idx(map, hash);
    if (__hashmap_is_invalid_idx(map, idx))
    {
        return ERR_RANGE_ERROR;
    }

    _HashMapEntry *entry = map->_buckets[idx];

    while (entry)
    {
        if (entry->_hash == hash && __hashmap_key_equals(entry->_key, key))
        {
            if (!entry->_value)
            {
                entry->_value = alloc->alloc(alloc, map->_valueSize);
                if (!entry->_value)
                    return ERR_OUT_OF_MEMORY;
            }

            for (u64 i = 0; i < map->_valueSize; ++i)
                ((u8 *)entry->_value)[i] = ((const u8 *)value)[i];
            
            return ERR_OK;
        }
        entry = entry->_next;
    }

    _HashMapEntry *newEntry = (_HashMapEntry *)alloc->alloc(alloc, sizeof(_HashMapEntry));

    if (!newEntry)
        return ERR_OUT_OF_MEMORY;
    
    newEntry->_hash = hash;
    newEntry->_key = (Buffer){
        .bytes = (i8 *)alloc->alloc(alloc, key.size),
        .size = key.size,
    };

    if (!newEntry->_key.bytes)
    {
        alloc->free(alloc, newEntry);
        return ERR_OUT_OF_MEMORY;
    }

    for (u64 i = 0; i < key.size; ++i)
        newEntry->_key.bytes[i] = key.bytes[i];
    
    newEntry->_value = alloc->alloc(alloc, map->_valueSize);
    if (!newEntry->_value)
    {
        alloc->free(alloc, newEntry->_key.bytes);
        alloc->free(alloc, newEntry);
        return ERR_OUT_OF_MEMORY;
    }

    for (u64 i = 0; i < map->_valueSize; ++i)
        ((u8 *)newEntry->_value)[i] = ((const u8 *)value)[i];

    newEntry->_next = map->_buckets[idx];
    map->_buckets[idx] = newEntry;
    map->_size += 1;
    return ERR_OK;
}

Error __hashmap_rehash(HashMap *map, u64 newBucketCount)
{
    Allocator *alloc = &map->_allocator;
    _HashMapEntry **newBuckets = (_HashMapEntry **)alloc->alloc(alloc, sizeof(_HashMapEntry *) * newBucketCount);

    if (!newBuckets)
        return ERR_OUT_OF_MEMORY;
    
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
    return ERR_OK;
}

/**
 * @brief Sets or overwrites value for provided `key` of type `Buffer`
 * 
 * @param map 
 * @param key 
 * @param value 
 * @return Error 
 */
Error hashmap_set(HashMap *map, Buffer key, const void *value)
{
    if (!map || !map->_buckets || !key.bytes)
        return ERR_INVALID_PARAMETER;
    
    if ((map->_size + 1) * _X_HASHMAP_LOAD_FACTOR_DEN > map->_bucketCount * _X_HASHMAP_LOAD_FACTOR_NUM)
    {
        Error err = __hashmap_rehash(map, map->_bucketCount * 2);
        if (err != ERR_OK)
            return err;
    }
    return __hashmap_set(map, key, value, __hashmap_fnv1a64(key.bytes, key.size));
}

/**
 * @brief Sets or overwrites value for provided `key` of type `String`
 * 
 * @param map 
 * @param key 
 * @param value 
 * @return Error 
 */
Error hashmap_set_str(HashMap *map, ConstStr key, const void *value)
{
    if (!key)
        return ERR_INVALID_PARAMETER;
    
    Buffer keyBuff = {.bytes = (i8 *)key, .size = string_size(key)};
    return hashmap_set(map, keyBuff, value);
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
Error hashmap_get(HashMap *map, Buffer key, void *outValue)
{
    if (!map || !map->_buckets || !key.bytes)
        return ERR_INVALID_PARAMETER;
    
    u64 hash = __hashmap_fnv1a64(key.bytes, key.size);
    u64 idx = __hashmap_bucket_idx(map, hash);

    if (__hashmap_is_invalid_idx(map, idx))
    {
        return ERR_RANGE_ERROR;
    }

    _HashMapEntry *entry = map->_buckets[idx];
    while (entry)
    {
        if (entry->_hash == hash && __hashmap_key_equals(entry->_key, key))
        {
            if (outValue && entry->_value)
                for (u64 i = 0; i < map->_valueSize; ++i)
                    ((u8 *)outValue)[i] = ((u8 *)entry->_value)[i];
            return ERR_OK;
        }
        entry = entry->_next;
    }
    return ERR_FAILED;
}

#define HashMapGetBuffT(T, mapPtr, keyBuff, outPtr) \
    { \
        T *mapItemTypeCheck = (outPtr); \
        (void)mapItemTypeCheck; \
        hashmap_get((mapPtr), (keyBuff), (outPtr)) \
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
Error hashmap_get_str(HashMap *map, ConstStr key, void *outValue)
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
        hashmap_get_str((mapPtr), (keyStr), (outPtr)) \
    }

/**
 * @brief Removes a value associated with the provided key of type `Buffer`
 * 
 * @param map 
 * @param key 
 * @return Error 
 */
Error hashmap_remove(HashMap *map, Buffer key)
{
    if (!map || !map->_buckets || !key.bytes)
        return ERR_INVALID_PARAMETER;
    
    u64 hash = __hashmap_fnv1a64(key.bytes, key.size);
    u64 idx = __hashmap_bucket_idx(map, hash);

    if (__hashmap_is_invalid_idx(map, idx))
    {
        return ERR_RANGE_ERROR;
    }

    _HashMapEntry **prev = &map->_buckets[idx];
    _HashMapEntry *entry = map->_buckets[idx];

    while (entry)
    {
        if (entry->_hash == hash && __hashmap_key_equals(entry->_key, key))
        {
            *prev = entry->_next;
            Allocator *alloc = &map->_allocator;

            if (entry->_key.bytes)
                alloc->free(alloc, entry->_key.bytes);
            
            if (entry->_value)
                alloc->free(alloc, entry->_value);
            
            alloc->free(alloc, entry);
            map->_size -= 1;
            return ERR_OK;
        }
        prev = &entry->_next;
        entry = entry->_next;
    }
    return ERR_FAILED;
}

/**
 * @brief Removes a value associated with the provided key of type `String`
 * 
 * @param map 
 * @param key 
 * @return Error 
 */
Error hashmap_remove_str(HashMap *map, ConstStr key)
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
void hashmap_for_each(HashMap *map, void (*func)(Buffer key, void *value, void *userArg), void *userArg)
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
u64 hashmap_size(HashMap *map)
{
    if (!map)
        return 0;
    
    return map->_size;
}
