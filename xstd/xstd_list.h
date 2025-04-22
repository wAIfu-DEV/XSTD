#pragma once

#include "xstd_core.h"
#include "xstd_alloc.h"
#include "xstd_result.h"

typedef struct _list
{
    void *_data;
    u64 _allocCnt;
    u64 _typeSize;
    u64 _itemCnt;
    Allocator _allocator;
} List;

typedef struct _result_list
{
    List value;
    Error error;
} ResultList;

#define _X_LIST_INIT_SIZE 8

/**
 * @brief Create a dynamic array that can be appended to, modified and resized.
 *
 * ```c
 * ResultList res = list_init(&c_allocator, sizeof(String), 16);
 * if (res.error) // Error!
 * List l = res.value;
 * ```
 * @param alloc
 * @param itemByteSize Size of the type of the items
 * @param initialAllocSize
 * @return ResultList
 */
ResultList list_init(Allocator *alloc, u64 itemByteSize, u64 initialAllocSize)
{
    if (!alloc)
        return (ResultList){
            .value = {0},
            .error = ERR_INVALID_PARAMETER,
        };

    if (initialAllocSize < _X_LIST_INIT_SIZE)
        initialAllocSize = _X_LIST_INIT_SIZE;

    if (itemByteSize == 0)
        return (ResultList){
            .value = {0},
            .error = ERR_INVALID_PARAMETER,
        };

    List l = {
        ._data = NULL,
        ._typeSize = itemByteSize,
        ._allocCnt = initialAllocSize,
        ._itemCnt = 0,
        ._allocator = *alloc,
    };

    l._data = alloc->alloc(alloc, l._allocCnt * l._typeSize);

    if (l._data == NULL)
        return (ResultList){
            .value = {0},
            .error = ERR_OUT_OF_MEMORY,
        };

    return (ResultList){
        .value = l,
        .error = ERR_OK,
    };
}

/**
 * @brief Create a dynamic array that can be appended to, modified and resized.
 *
 * ```c
 * ResultList res = ListInitT(String, &c_allocator);
 * if (res.error) // Error!
 * List l = res.value;
 * ```
 * @param T type of the items
 * @param allocPtr
 * @return ResultList
 */
#define ListInitT(T, allocPtr) list_init((allocPtr), sizeof(T), 16)

/**
 * @brief Frees the memory allocated by the list.
 *
 * ```c
 * ResultList res = ListInitT(String, alloc);
 * if (res.error) // Error!
 * List l = res.value;
 * list_deinit(&l);
 * // List is now invalid, do not use it again.
 * ```
 * @param list
 */
void list_deinit(List *list)
{
    if (!list)
        return;

    if (!list->_data)
        return;

    list->_allocator.free(&list->_allocator, list->_data);
}

/**
 * @brief Returns the amount of elements inside the list.
 *
 * ```c
 * list_push(&list, &elem);
 * list_push(&list, &elem);
 * list_push(&list, &elem);
 *
 * u64 size = list_size(&list);
 * // size == 3
 * ```
 * @param list
 * @return u64
 */
u64 list_size(List *list)
{
    if (!list)
        return 0;

    return list->_itemCnt;
}

void _list_expand(List *l)
{
    l->_data = l->_allocator.realloc(&l->_allocator,
                                     l->_data,
                                     (l->_allocCnt * l->_typeSize) * 2);
    l->_allocCnt *= 2;
}

ibool _list_should_shrink(List *l)
{
    u64 halfAlloc = (u64)((f64)(l->_allocCnt) * 0.5);

    if (halfAlloc < _X_LIST_INIT_SIZE)
        return 0;

    return (l->_itemCnt < halfAlloc);
}

void _list_shrink(List *l)
{
    u64 newCnt = (u64)((f64)(l->_allocCnt) * 0.5);
    l->_allocCnt = newCnt;

    u64 newSize = newCnt * l->_typeSize;

    l->_data = l->_allocator.realloc(&l->_allocator,
                                     l->_data,
                                     newSize);
}

void _list_wipe(List *l)
{
    l->_allocCnt = _X_LIST_INIT_SIZE;
    l->_itemCnt = 0;

    u64 newSize = _X_LIST_INIT_SIZE * l->_typeSize;

    l->_data = l->_allocator.realloc(&l->_allocator,
                                     l->_data,
                                     newSize);
}

void _list_memcpy(List *l, const void *srcPtr, void *dstPtr)
{
    u64 rest = l->_typeSize;

    i8 *d = (i8 *)dstPtr;
    const i8 *s = (i8 *)srcPtr;

    while (rest--)
        *d++ = *s++;
}

void *_list_i_to_ptr(List *l, u64 i)
{
    return ((i8 *)l->_data) + l->_typeSize * i;
}

/**
 * @brief Writes contents of `list[i]` to `out`
 * Prefer `ListGetT` macro as it provides compiler type checking.
 *
 * ```c
 * String s = ConstToHeapStr("Test string");
 * list_push(&list, &s);
 * String resultStr;
 * Error err = list_get(&list, 0, &resultStr);
 * if (err) // Error!
 * // resultStr == &"Test string"
 * ```
 * @param list
 * @param i index
 * @param out Pointer to allocated memory of size sizeof(ItemType)
 * @return Error
 */
Error list_get(List *list, u64 i, void *out)
{
    if (!list)
        return ERR_INVALID_PARAMETER;

    if (i >= list->_itemCnt)
        return ERR_RANGE_ERROR;

    void *ptr = _list_i_to_ptr(list, i);
    _list_memcpy(list, ptr, out);
    return ERR_OK;
}

/**
 * @brief Type checked version of `list_get()`
 * Writes contents of `list[i]` to `out`
 *
 * ```c
 * String s = ConstToHeapStr("Test string");
 * list_push(&list, &s);
 * String resultStr;
 * Error err = list_get(&list, 0, &resultStr);
 * if (err) // Error!
 * // resultStr == &"Test string"
 * ```
 * @param list
 * @param i index
 * @param out Pointer to allocated memory of size sizeof(ItemType)
 */
#define ListGetT(T, listPtr, i, outPtr)     \
    {                                       \
        T *listItemTypeCheck = (outPtr);    \
        (void)listItemTypeCheck;            \
        list_get((listPtr), (i), (outPtr)); \
    }

/**
 * @brief Writes contents of `list[i]` to `out`. Does not do any bounds checking or validation.
 *
 * ```c
 * String s = ConstToHeapStr("Test string");
 * list_push(&list, &s);
 * String resultStr;
 * Error err = list_get_unsafe(&list, 0, &resultStr);
 * // resultStr == &"Test string"
 * ```
 * @param list
 * @param i index
 * @param out Pointer to allocated memory of size sizeof(ItemType)
 */
void list_get_unsafe(List *list, u64 i, void *out)
{
    void *ptr = _list_i_to_ptr(list, i);
    _list_memcpy(list, ptr, out);
}

/**
 * @brief Get pointer to value stored in `list[i]`
 * Memory is NOT owned by the caller.
 * Prefer `ListGetRefT` macro as it provides compiler type checking.
 *
 * ```c
 * String s = ConstToHeapStr("Test string");
 * list_push(&list, &s);
 * String* resultStr = list_getref(&list, 0);
 * if (!resultStr) // Error!
 * // resultStr == &&"Test string"
 * ```
 * @param list
 * @param i index
 * @return void*
 */
void *list_getref(List *list, u64 i)
{
    if (!list)
        return NULL;

    if (i >= list->_itemCnt)
        return NULL;

    return _list_i_to_ptr(list, i);
}

/**
 * @brief Type checked version of `list_getref()`
 * Gets pointer to value stored in `list[i]`
 *
 * Memory is NOT owned by the caller.
 *
 * ```c
 * String s = ConstToHeapStr("Test string");
 * list_push(&list, &s);
 * String* resultStr;
 * ListGetRefT(String, &list, 0, resultStr);
 * if (!resultStr) // Error!
 * // resultStr == &&"Test string"
 * ```
 * @param list
 * @param i index
 */
#define ListGetRefT(T, listPtr, i, outPtr)      \
    {                                           \
        T *listItemTypeCheck = (outPtr);        \
        (void)listItemTypeCheck;                \
        (outPtr) = list_getref((listPtr), (i)); \
    }

/**
 * @brief Get pointer to value stored in `list[i]`. Does not do any bounds checking or validation.
 * Memory is NOT owned by the caller.
 *
 * ```c
 * String s = ConstToHeapStr("Test string");
 * list_push(&list, &s);
 * String* resultStr = list_getref_unsafe(&list, 0);
 * // resultStr == &&"Test string"
 * ```
 * @param list
 * @param i index
 * @return Error
 */
void *list_getref_unsafe(List *list, u64 i)
{
    return _list_i_to_ptr(list, i);
}

/**
 * @brief Get value stored in `list[i]` as a pointer.
 * Memory is NOT owned by the caller.
 *
 * ```c
 * String s = ConstToHeapStr("Test string");
 * list_push(&list, &s);
 * String resultStr = list_get_as_ptr(&list, 0);
 * if (!resultStr) // Error!
 * // resultStr == &"Test string"
 * ```
 * @param list
 * @param i index
 * @return void*
 */
void *list_get_as_ptr(List *list, u64 i)
{
    if (!list)
        return NULL;

    if (list->_typeSize != sizeof(i8 *))
        return NULL;

    if (i >= list->_itemCnt)
        return NULL;

    return *(void **)_list_i_to_ptr(list, i);
}

/**
 * @brief Writes contents of `item` to `list[i]`
 * Emplaces a copy of the item pointed to by `item` into the list slot i.
 * Prefer `ListSetT` macro as it provides compiler type checking.
 *
 * ```c
 * String myStr = ConstToHeapStr("Test string");
 * list_set(&list, 0, &myStr);
 * // list[0] == &"Test string"
 * ```
 * @param list
 * @param i index
 * @param item Pointer to allocated memory of size sizeof(ItemType)
 * @return Error
 */
void list_set(List *list, u64 i, const void *item)
{
    if (!list)
        return;

    if (i >= list->_itemCnt)
        return;

    void *ptr = _list_i_to_ptr(list, i);
    _list_memcpy(list, item, ptr);
}

/**
 * @brief Type checked version of `list_set()`
 * Writes contents of `item` to `list[i]`
 *
 * ```c
 * String myStr = ConstToHeapStr("Test string");
 * ListSetT(String, &list, 0, &myStr);
 * // list[0] == &"Test string"
 * ```
 * @param listPtr
 * @param i index
 * @param itemPtr Pointer to allocated memory of size sizeof(ItemType)
 * @return Error
 */
#define ListSetT(T, listPtr, i, itemPtr)     \
    {                                        \
        T *listItemTypeCheck = (itemPtr);    \
        (void)listItemTypeCheck;             \
        list_set((listPtr), (i), (itemPtr)); \
    }

/**
 * @brief Unsafe version of `list_set`, does not do any bounds checking or argument validation.
 * Writes contents of `item` to `list[i]`
 * Emplaces a copy of the item pointed to by `item` into the list slot i.
 *
 * ```c
 * String myStr = ConstToHeapStr("Test string");
 * list_set_unsafe(&list, 0, &myStr);
 * // list[0] == &"Test string"
 * ```
 * @param list
 * @param i index
 * @param item Pointer to allocated memory of size sizeof(ItemType)
 * @return Error
 */
void list_set_unsafe(List *list, u64 i, const void *item)
{
    _list_memcpy(list, item, _list_i_to_ptr(list, i));
}

/**
 * @brief Calls `Allocator.free` on all items of the list. Items MUST be pointers
 * allocated using the same allocator.
 * Invalidates all items by replacing them by NULL.
 *
 * ```c
 * ResultList res = string_splitc(&alloc, "test string", ' ', false);
 * if (res.error) // Error!
 * List l = res.value;
 * // Do list stuff
 * list_free_items(&alloc, &l); // Frees all strings
 * list_deinit(&l); // Free the list
 * ```
 * @param alloc
 * @param list
 */
void list_free_items(Allocator *alloc, List *list)
{
    if (!list)
        return;

    if (!list->_data)
        return;

    if (list->_typeSize != sizeof(u64))
        return;

    void *nullPtr = NULL;

    u64 bound = list->_itemCnt;
    for (u64 i = 0; i < bound; ++i)
    {
        void **ptr = list_getref_unsafe(list, i);
        alloc->free(alloc, *ptr);
        list_set_unsafe(list, i, &nullPtr);
    }
}

/**
 * @brief Writes contents of `item` to end of the list, increases list size by 1.
 * Prefer `ListPushT` macro as it provides compiler type checking.
 *
 * ```c
 * String myStr = ConstToHeapStr("Test string");
 * list_push(&list, &myStr);
 * // list[list_size(&list) -1] == &"Test string"
 * ```
 * @param list
 * @param item Pointer to allocated memory of size sizeof(ItemType)
 * @return Error
 */
void list_push(List *list, const void *item)
{
    if (!list)
        return;

    if (list->_itemCnt >= list->_allocCnt)
        _list_expand(list);

    u64 i = list->_itemCnt++;
    void *ptr = _list_i_to_ptr(list, i);
    _list_memcpy(list, item, ptr);
}

/**
 * @brief Type checked version of list_push.
 * Writes contents of `item` to end of the list, increases list size by 1.
 *
 * ```c
 * String myStr = ConstToHeapStr("Test string");
 * ListPushT(String, &list, &myStr);
 * // list[list_size(&list) -1] == &"Test string"
 * ```
 * @param T type of the item, should match the type the list has been initialized with.
 * @param list
 * @param itemPtr Pointer to allocated memory of size sizeof(ItemType)
 * @return Error
 */
#define ListPushT(T, listPtr, itemPtr)    \
    {                                     \
        T *listItemTypeCheck = (itemPtr); \
        (void)listItemTypeCheck;          \
        list_push((listPtr), (itemPtr));  \
    }

/**
 * @brief Writes contents of the end of the list to `out`, decreases list size by 1.
 * Memory is NOT owned by the caller.
 * Prefer `ListPopT` macro as it provides compiler type checking.
 *
 * ```c
 * String myStr = ConstToHeapStr("Test string");
 * list_push(&list, &myStr);
 * String resultStr;
 * Error err = list_pop(&list, &resultStr);
 * if (err) // Error!
 * // resultStr == &"Test string"
 * ```
 * @param list
 * @param out
 * @return Error
 */
Error list_pop(List *list, void *out)
{
    if (!list)
        return ERR_INVALID_PARAMETER;

    if (list->_itemCnt == 0)
        return ERR_RANGE_ERROR;

    u64 i = --list->_itemCnt;

    void *ptr = _list_i_to_ptr(list, i);
    _list_memcpy(list, ptr, out);

    if (_list_should_shrink(list))
    {
        _list_shrink(list);
    }
    return ERR_OK;
}

/**
 * @brief Type checked version of `list_pop()`
 * Writes contents of the end of the list to `out`, decreases list size by 1.
 * Memory is NOT owned by the caller.
 *
 * ```c
 * String myStr = ConstToHeapStr("Test string");
 * list_push(&list, &myStr);
 * String resultStr;
 * Error err = list_pop(&list, &resultStr);
 * if (err) // Error!
 * // resultStr == &"Test string"
 * ```
 * @param list
 * @param out
 * @return Error
 */
#define ListPopT(T, listPtr, outPtr)     \
    {                                    \
        T *listItemTypeCheck = (outPtr); \
        (void)listItemTypeCheck;         \
        list_pop((listPtr), (outPtr));   \
    }

/**
 * @brief Clears the list and shrinks the allocated memory to the minimum.
 * Can be reused after clearing, to destroy the list use `list_deinit()`.
 *
 * @param list
 */
void list_clear(List *list)
{
    if (!list)
        return;

    _list_wipe(list);
}

/**
 * @brief Clears the list without shrinking the allocated memory.
 * Can be reused after clearing, to destroy the list use `list_deinit()`.
 *
 * @param list
 */
void list_clear_nofree(List *list)
{
    if (!list)
        return;

    list->_itemCnt = 0;
}

/**
 * @brief Calls `func` for each item in the list.
 *
 * @param list
 * @param func
 */
void list_for_each(List *list, void (*func)(void *itemPtr, u64 index, void* userArg), void* userArg)
{
    u64 bound = list->_itemCnt;
    for (u64 i = 0; i < bound; ++i)
    {
        func(list_getref_unsafe(list, i), i, userArg);
    }
}

// TODO:
// 1. remove_at (useful for pop_front alt)
// 2. insert_at (useful for push_front alt)
// 3. sorting
