#pragma once

#include "xstd_coretypes.h"
#include "xstd_alloc.h"
#include "xstd_io.h"
#include "xstd_str.h"

void *_xstd_bad_alloc_alloc(Allocator *a, u64 s)
{
    (void)a;
    (void)s;
    return NULL;
}

void *_xstd_bad_alloc_realloc(Allocator *a, void *b, u64 s)
{
    (void)a;
    (void)b;
    (void)s;
    return NULL;
}

void _xstd_bad_alloc_free(Allocator *a, void *b)
{
    (void)a;
    (void)b;
}

Allocator _xstd_bad_alloc(void)
{
    return (Allocator){
        ._internalState = NULL,
        .alloc = _xstd_bad_alloc_alloc,
        .realloc = _xstd_bad_alloc_realloc,
        .free = _xstd_bad_alloc_free,
    };
}

void _xstd_print_strlist(void *itemPtr, u64 index)
{
    io_print_uint(index);
    io_print(": ");
    io_println(*(HeapStr *)itemPtr);
}

void _xstd_foreach_test(void *itemPtr, u64 index)
{
    HeapStr itemStr = *(HeapStr *)itemPtr;
    itemStr[0] = ' ';
}

void _xstd_string_tests(Allocator alloc)
{
    Error err;
    Allocator badAlloc = _xstd_bad_alloc();

    // =========================================================================
    // TEST string_size
    // =========================================================================
    {
        ConstStr strLen22 = "This string is 22 long";
        x_assert(string_size(strLen22) == 22, "string_size 22 != 22");

        ConstStr strLen0 = "";
        x_assert(string_size(strLen0) == 0, "string_size 0 != 0");

        ConstStr strLenNull = NULL;
        x_assert(string_size(strLenNull) == 0, "string_size NULL != 0");
    }
    // =========================================================================
    // TEST string_equals
    // =========================================================================
    {
        ConstStr strEq1 = "This is equal.";
        ConstStr strEq2 = "This is equal.";
        x_assert(string_equals(strEq1, strEq2), "string_equals strEq1 != strEq2");
        x_assert(string_equals(strEq2, strEq1), "string_equals strEq2 != strEq1");

        ConstStr strNeq3 = "This is not equal.";
        x_assert(!string_equals(strEq1, strNeq3), "string_equals strEq1 == strNeq3");
        x_assert(!string_equals(strNeq3, strEq1), "string_equals strNeq3 == strEq1");

        ConstStr strShortEq1 = "a";
        ConstStr strShortEq2 = "a";
        x_assert(string_equals(strShortEq1, strShortEq2), "string_equals strShortEq1 != strShortEq2");
        x_assert(string_equals(strShortEq2, strShortEq1), "string_equals strShortEq2 != strShortEq1");

        ConstStr strShortNeq3 = "b";
        x_assert(!string_equals(strShortEq1, strShortNeq3), "string_equals strShortEq1 == strShortNeq3");
        x_assert(!string_equals(strShortNeq3, strShortEq1), "string_equals strShortNeq3 == strShortEq1");

        ConstStr strShortEq4 = "ab";
        x_assert(string_equals(strShortEq4, strShortEq4), "string_equals strShortEq4 != strShortEq4");

        x_assert(!string_equals(strEq1, NULL), "string_equals strEq1 == NULL");
        x_assert(!string_equals(NULL, strEq1), "string_equals NULL == strEq1");
        x_assert(string_equals(NULL, NULL), "string_equals NULL != NULL");
    }
    // =========================================================================
    // TEST string_alloc
    // =========================================================================
    {
        ResultOwnedStr strHeapRes1 = string_alloc(&alloc, 5, ' ');
        x_assert(strHeapRes1.error == ERR_OK, "string_alloc strHeapRes1.error != ERR_OK");

        HeapStr strHeap1 = strHeapRes1.value;
        x_assert(strHeap1 != NULL, "string_alloc strHeapRes1.value == NULL");
        x_assert(string_size(strHeap1) == 5, "string_alloc size strHeapRes1.value != 5");
        x_assert(string_equals(strHeap1, "     "), "string_alloc strHeap1 != \"     \"");

        ResultOwnedStr strHeapRes2 = string_alloc(&alloc, 0, ' ');
        x_assert(strHeapRes2.error == ERR_OK, "string_alloc strHeapRes2.error != ERR_OK");

        HeapStr strHeap2 = strHeapRes2.value;
        x_assert(strHeap2 != NULL, "string_alloc strHeapRes2.value == NULL");
        x_assert(string_size(strHeap2) == 0, "string_alloc size strHeapRes2.value != 0");
        x_assert(string_equals(strHeap2, ""), "string_alloc strHeap2 != \"\"");

        ResultOwnedStr strHeapRes3 = string_alloc(&badAlloc, 5, ' ');
        x_assert(strHeapRes3.error != ERR_OK, "string_alloc strHeapRes3.error == ERR_OK");

        alloc.free(&alloc, strHeap1);
        alloc.free(&alloc, strHeap2);
    }
    // =========================================================================
    // TEST string_copy_unsafe
    // =========================================================================
    {
        ConstStr strCopyUn1 = "Copied.";

        ResultOwnedStr strCopyUnRes2 = string_alloc(&alloc, 7, ' ');
        x_assert(strCopyUnRes2.error == ERR_OK, "string_copy_unsafe strCopyUnRes2.error != ERR_OK");
        HeapStr strCopyUn2 = strCopyUnRes2.value;
        string_copy_unsafe(strCopyUn1, strCopyUn2);
        x_assert(string_equals(strCopyUn1, strCopyUn2), "string_copy_unsafe strCopyUn1 != strCopyUn2");

        ResultOwnedStr strCopyUnRes3 = string_alloc(&alloc, 10, ' ');
        x_assert(strCopyUnRes3.error == ERR_OK, "string_copy_unsafe strCopyUnRes3.error != ERR_OK");
        HeapStr strCopyUn3 = strCopyUnRes3.value;
        string_copy_unsafe(strCopyUn1, strCopyUn3);
        x_assert(string_equals(strCopyUn3, strCopyUn1), "string_copy_unsafe strCopyUn3 != strCopyUn1");

        alloc.free(&alloc, strCopyUn2);
        alloc.free(&alloc, strCopyUn3);
    }
    // =========================================================================
    // TEST string_copy_n_unsafe
    // =========================================================================
    {
        ConstStr strCopyNUn1 = "Copied.";

        ResultOwnedStr strCopyNUnRes2 = string_alloc(&alloc, 7, ' ');
        x_assert(strCopyNUnRes2.error == ERR_OK, "string_copy_n_unsafe strCopyNUnRes2.error != ERR_OK");
        HeapStr strCopyNUn2 = strCopyNUnRes2.value;
        string_copy_n_unsafe(strCopyNUn1, strCopyNUn2, 7, false);
        x_assert(string_equals(strCopyNUn1, strCopyNUn2), "string_copy_n_unsafe strCopyNUn1 != strCopyNUn2");

        ResultOwnedStr strCopyNUnRes3 = string_alloc(&alloc, 10, ' ');
        x_assert(strCopyNUnRes3.error == ERR_OK, "string_copy_n_unsafe strCopyNUnRes3.error != ERR_OK");
        HeapStr strCopyNUn3 = strCopyNUnRes3.value;
        string_copy_n_unsafe(strCopyNUn1, strCopyNUn3, 7, false);
        x_assert(string_equals(strCopyNUn3, "Copied.   "), "string_copy_n_unsafe strCopyNUn3 != \"Copied.   \"");

        ResultOwnedStr strCopyNUnRes4 = string_alloc(&alloc, 10, ' ');
        x_assert(strCopyNUnRes4.error == ERR_OK, "string_copy_n_unsafe strCopyNUnRes4.error != ERR_OK");
        HeapStr strCopyNUn4 = strCopyNUnRes4.value;
        string_copy_n_unsafe(strCopyNUn1, strCopyNUn4, 3, false);
        x_assert(string_equals(strCopyNUn4, "Cop       "), "string_copy_n_unsafe strCopyNUn4 != \"Cop       \"");

        ResultOwnedStr strCopyNUnRes5 = string_alloc(&alloc, 10, ' ');
        x_assert(strCopyNUnRes5.error == ERR_OK, "string_copy_n_unsafe strCopyNUnRes5.error != ERR_OK");
        HeapStr strCopyNUn5 = strCopyNUnRes5.value;
        string_copy_n_unsafe(strCopyNUn1, strCopyNUn5, 0, false);
        x_assert(string_equals(strCopyNUn5, "          "), "string_copy_n_unsafe strCopyNUn5 != \"          \"");

        // TODO: Add testing for terminate==true

        alloc.free(&alloc, strCopyNUn2);
        alloc.free(&alloc, strCopyNUn3);
        alloc.free(&alloc, strCopyNUn4);
        alloc.free(&alloc, strCopyNUn5);
    }
    // =========================================================================
    // TEST string_copy
    // =========================================================================
    {
        ConstStr strCopy1 = "Copied.";

        ResultOwnedStr strCopyRes2 = string_alloc(&alloc, 7, ' ');
        x_assert(strCopyRes2.error == ERR_OK, "string_copy strCopyRes2.error != ERR_OK");
        HeapStr strCopy2 = strCopyRes2.value;
        err = string_copy(strCopy1, strCopy2);
        x_assert(err == ERR_OK, "string_copy strCopy2 err != ERR_OK");
        x_assert(string_equals(strCopy1, strCopy2), "string_copy strCopy1 != strCopy2");

        ResultOwnedStr strCopyRes3 = string_alloc(&alloc, 10, ' ');
        x_assert(strCopyRes3.error == ERR_OK, "string_copy strCopyRes3.error != ERR_OK");
        HeapStr strCopy3 = strCopyRes3.value;
        err = string_copy(strCopy1, strCopy3);
        x_assert(err == ERR_OK, "string_copy strCopy3 err != ERR_OK");
        x_assert(string_equals(strCopy3, strCopy1), "string_copy strCopy3 != strCopy1");

        err = string_copy(strCopy1, NULL);
        x_assert(err != ERR_OK, "string_copy NULL arg2 err == ERR_OK");

        err = string_copy(NULL, strCopy3);
        x_assert(err != ERR_OK, "string_copy NULL arg1 err == ERR_OK");

        err = string_copy(NULL, NULL);
        x_assert(err != ERR_OK, "string_copy NULL args err == ERR_OK");

        err = string_copy("                              ", strCopy2);
        x_assert(err != ERR_OK, "string_copy arg2 smaller err == ERR_OK");

        alloc.free(&alloc, strCopy2);
        alloc.free(&alloc, strCopy3);
    }
    // =========================================================================
    // TEST string_copy_n
    // =========================================================================
    {
        ConstStr strCopyN1 = "Copied.";

        ResultOwnedStr strCopyNRes2 = string_alloc(&alloc, 7, ' ');
        x_assert(strCopyNRes2.error == ERR_OK, "string_copy_n strCopyNRes2.error != ERR_OK");
        HeapStr strCopyN2 = strCopyNRes2.value;
        err = string_copy_n(strCopyN1, strCopyN2, 7, false);
        x_assert(err == ERR_OK, "string_copy_n strCopyN2 err != ERR_OK");
        x_assert(string_equals(strCopyN1, strCopyN2), "string_copy_n strCopyN1 != strCopyN2");

        ResultOwnedStr strCopyNRes3 = string_alloc(&alloc, 10, ' ');
        x_assert(strCopyNRes3.error == ERR_OK, "string_copy_n strCopyNRes3.error != ERR_OK");
        HeapStr strCopyN3 = strCopyNRes3.value;
        err = string_copy_n(strCopyN1, strCopyN3, 7, false);
        x_assert(err == ERR_OK, "string_copy_n strCopyN3 err != ERR_OK");
        x_assert(string_equals(strCopyN3, "Copied.   "), "string_copy_n strCopyN3 != \"Copied.   \"");

        ResultOwnedStr strCopyNRes4 = string_alloc(&alloc, 10, ' ');
        x_assert(strCopyNRes4.error == ERR_OK, "string_copy_n strCopyNRes4.error != ERR_OK");
        HeapStr strCopyN4 = strCopyNRes4.value;
        err = string_copy_n(strCopyN1, strCopyN4, 3, false);
        x_assert(err == ERR_OK, "string_copy_n strCopyN4 err != ERR_OK");
        x_assert(string_equals(strCopyN4, "Cop       "), "string_copy_n strCopyN4 != \"Cop       \"");

        ResultOwnedStr strCopyNRes5 = string_alloc(&alloc, 10, ' ');
        x_assert(strCopyNRes5.error == ERR_OK, "string_copy_n strCopyNRes5.error != ERR_OK");
        HeapStr strCopyN5 = strCopyNRes5.value;
        err = string_copy_n(strCopyN1, strCopyN5, 0, false);
        x_assert(err == ERR_OK, "string_copy_n strCopyN5 err != ERR_OK");
        x_assert(string_equals(strCopyN5, "          "), "string_copy_n strCopyN5 != \"          \"");

        err = string_copy_n(strCopyN1, NULL, 5, false);
        x_assert(err != ERR_OK, "string_copy_n NULL arg2 err == ERR_OK");

        err = string_copy_n(NULL, strCopyN3, 5, false);
        x_assert(err != ERR_OK, "string_copy_n NULL arg1 err == ERR_OK");

        err = string_copy_n(NULL, NULL, 5, false);
        x_assert(err != ERR_OK, "string_copy_n NULL args err == ERR_OK");

        err = string_copy_n(" ", strCopyN2, 5, false);
        x_assert(err != ERR_OK, "string_copy_n arg1 smaller than n err == ERR_OK");

        err = string_copy_n("             ", strCopyN2, 8, false);
        x_assert(err != ERR_OK, "string_copy_n arg2 smaller than n err == ERR_OK");

        err = string_copy_n(strCopyN1, strCopyN3, 0, false);
        x_assert(err == ERR_OK, "string_copy_n 0 arg3 err != ERR_OK");

        err = string_copy_n(strCopyN1, strCopyN3, 99, false);
        x_assert(err != ERR_OK, "string_copy_n 99 arg3 err == ERR_OK");

        // TODO: Add testing for terminate==true

        alloc.free(&alloc, strCopyN2);
        alloc.free(&alloc, strCopyN3);
        alloc.free(&alloc, strCopyN4);
        alloc.free(&alloc, strCopyN5);
    }
    // =========================================================================
    // TEST string_dupe
    // =========================================================================
    {
        ConstStr strDupe0 = "";
        ConstStr strDupe1 = "Copied.";

        ResultOwnedStr strDupe2 = string_dupe(&alloc, strDupe1);
        x_assert(strDupe2.error == ERR_OK, "string_dupe strDupe2.error != ERR_OK");
        x_assert(string_equals(strDupe1, strDupe2.value), "string_dupe strDupe1 != strDupe2");

        ResultOwnedStr strDupe3 = string_dupe(&alloc, strDupe0);
        x_assert(strDupe3.error == ERR_OK, "string_dupe strDupe3.error != ERR_OK");
        x_assert(string_equals(strDupe0, strDupe3.value), "string_dupe strDupe0 != strDupe3");

        ResultOwnedStr strDupe4 = string_dupe(&alloc, NULL);
        x_assert(strDupe4.error != ERR_OK, "string_dupe strDupe4.error == ERR_OK");

        ResultOwnedStr strDupe5 = string_dupe(&badAlloc, strDupe1);
        x_assert(strDupe5.error != ERR_OK, "string_dupe strDupe5.error == ERR_OK");

        alloc.free(&alloc, strDupe2.value);
        alloc.free(&alloc, strDupe3.value);
    }
    // =========================================================================
    // TEST string_dupe_noresult
    // =========================================================================
    {
        ConstStr strDupeNr0 = "";
        ConstStr strDupeNr1 = "Copied.";

        HeapStr strDupeNr2 = string_dupe_noresult(&alloc, strDupeNr1);
        x_assert(strDupeNr2 != NULL, "string_dupe_noresult strDupeNr2 != NULL");
        x_assert(string_equals(strDupeNr1, strDupeNr2), "string_dupe_noresult strDupeNr1 != strDupeNr2");

        HeapStr strDupeNr3 = string_dupe_noresult(&alloc, strDupeNr0);
        x_assert(strDupeNr3 != NULL, "string_dupe_noresult strDupeNr3 != NULL");
        x_assert(string_equals(strDupeNr0, strDupeNr3), "string_dupe_noresult strDupeNr0 != strDupeNr3");

        HeapStr strDupeNr4 = string_dupe_noresult(&alloc, NULL);
        x_assert(strDupeNr4 == NULL, "string_dupe_noresult strDupeNr4 != NULL");

        HeapStr strDupeNr5 = string_dupe_noresult(&badAlloc, strDupeNr1);
        x_assert(strDupeNr5 == NULL, "string_dupe_noresult strDupeNr5 != NULL");

        alloc.free(&alloc, strDupeNr2);
        alloc.free(&alloc, strDupeNr3);
    }
    // =========================================================================
    // TEST string_resize
    // =========================================================================
    {
        ConstStr strRes1 = "Resized";

        ResultOwnedStr strRes2 = string_resize(&alloc, strRes1, 15, '_');
        x_assert(strRes2.error == ERR_OK, "string_resize strRes2.error != ERR_OK");
        x_assert(string_equals(strRes2.value, "Resized________"), "string_resize strRes2 != \"Resized________\"");

        ResultOwnedStr strRes3 = string_resize(&alloc, strRes1, 3, '_');
        x_assert(strRes3.error == ERR_OK, "string_resize strRes3.error != ERR_OK");
        x_assert(string_equals(strRes3.value, "Res"), "string_resize strRes3 != \"Res\"");

        ResultOwnedStr strRes4 = string_resize(&alloc, NULL, 12, '_');
        x_assert(strRes4.error != ERR_OK, "string_resize strRes4.error == ERR_OK");

        ResultOwnedStr strRes5 = string_resize(&alloc, strRes1, 0, '_');
        x_assert(strRes5.error == ERR_OK, "string_resize strRes5.error != ERR_OK");
        x_assert(string_equals(strRes5.value, ""), "string_resize strRes5 != \"\"");

        ResultOwnedStr strRes6 = string_resize(&badAlloc, strRes1, 12, '_');
        x_assert(strRes6.error != ERR_OK, "string_resize strRes6.error == ERR_OK");

        alloc.free(&alloc, strRes2.value);
        alloc.free(&alloc, strRes3.value);
        alloc.free(&alloc, strRes5.value);
    }
    // =========================================================================
    // TEST string_concat
    // =========================================================================
    {
        ConstStr strConc1 = "Left ";
        ConstStr strConc2 = "Right";

        ResultOwnedStr strConc3 = string_concat(&alloc, strConc1, strConc2);
        x_assert(strConc3.error == ERR_OK, "string_concat strConc3.error != ERR_OK");
        x_assert(string_equals(strConc3.value, "Left Right"), "string_concat strConc3 != \"Left Right\"");

        ResultOwnedStr strConc4 = string_concat(&alloc, strConc1, NULL);
        x_assert(strConc4.error != ERR_OK, "string_concat strConc4.error == ERR_OK");

        ResultOwnedStr strConc5 = string_concat(&alloc, NULL, strConc2);
        x_assert(strConc5.error != ERR_OK, "string_concat strConc5.error == ERR_OK");

        ResultOwnedStr strConc6 = string_concat(&badAlloc, strConc1, strConc2);
        x_assert(strConc6.error != ERR_OK, "string_concat strConc6.error == ERR_OK");

        alloc.free(&alloc, strConc3.value);
        alloc.free(&alloc, strConc4.value);
    }
    // =========================================================================
    // TEST string_substr
    // =========================================================================
    {
        ConstStr strSub1 = "This is a substring";

        ResultOwnedStr strSub2 = string_substr(&alloc, strSub1, 10, 19);

        x_assert(strSub2.error == ERR_OK, "string_substr strSub2.error != ERR_OK");
        x_assert(string_equals(strSub2.value, "substring"), "string_substr strSub2 != \"substring\"");

        ResultOwnedStr strSub3 = string_substr(&alloc, strSub1, 0, 4);
        x_assert(strSub3.error == ERR_OK, "string_substr strSub3.error != ERR_OK");
        x_assert(string_equals(strSub3.value, "This"), "string_substr strSub3 != \"This\"");

        ResultOwnedStr strSub4 = string_substr(&alloc, strSub1, 10, 20);
        x_assert(strSub4.error != ERR_OK, "string_substr strSub4.error == ERR_OK");

        ResultOwnedStr strSub5 = string_substr(&alloc, NULL, 10, 20);
        x_assert(strSub5.error != ERR_OK, "string_substr strSub5.error == ERR_OK");

        ResultOwnedStr strSub6 = string_substr(&badAlloc, strSub1, 10, 19);
        x_assert(strSub6.error != ERR_OK, "string_substr strSub6.error == ERR_OK");

        alloc.free(&alloc, strSub2.value);
        alloc.free(&alloc, strSub3.value);
    }
    // =========================================================================
    // TEST string_substr_unsafe
    // =========================================================================
    {
        ConstStr strSub1 = "This is a substring";

        HeapStr strSub2 = string_substr_unsafe(&alloc, strSub1, 10, 19);

        x_assert(strSub2 != NULL, "string_substr_unsafe strSub2 == NULL");
        x_assert(string_equals(strSub2, "substring"), "string_substr_unsafe strSub2 != \"substring\"");

        HeapStr strSub3 = string_substr_unsafe(&alloc, strSub1, 0, 4);
        x_assert(strSub3 != NULL, "string_substr_unsafe strSub3 == NULL");
        x_assert(string_equals(strSub3, "This"), "string_substr_unsafe strSub3 != \"This\"");

        HeapStr strSub6 = string_substr_unsafe(&badAlloc, strSub1, 10, 19);
        x_assert(strSub6 == NULL, "string_substr_unsafe strSub6 != NULL");

        alloc.free(&alloc, strSub2);
        alloc.free(&alloc, strSub3);
    }
    // =========================================================================
    // TEST string_splitc
    // =========================================================================
    {
        ConstStr strSpl0 = " This is a split string ";
        ConstStr strSpl1 = "This is a  split string";

        ResultList strSpl2 = string_splitc(&alloc, strSpl1, ' ');
        x_assertErr(strSpl2.error, "string_splitc strSpl2.error != ERR_OK");

        List l = strSpl2.value;

        u64 listSize = list_size(&l);
        x_assert(listSize == 5, "string_splitc size strSpl2 != 5");

        HeapStr strSpl3;
        ListGetT(HeapStr, &l, 0, &strSpl3);
        x_assertStrEq(strSpl3, "This", "string_splitc strSpl3 != \"This\"");

        HeapStr strSpl3_5;
        ListGetT(HeapStr, &l, 3, &strSpl3_5);
        x_assertStrEq(strSpl3_5, "split", "string_splitc strSpl3_5 != \"split\"");

        HeapStr strSpl4;
        ListGetT(HeapStr, &l, 4, &strSpl4);
        x_assertStrEq(strSpl4, "string", "string_splitc strSpl4 != \"string\"");

        ResultList strSpl5 = string_splitc(&alloc, NULL, ' ');
        x_assert(strSpl5.error != ERR_OK, "string_splitc strSpl5.error == ERR_OK");

        ResultList strSpl6 = string_splitc(&alloc, strSpl0, ' ');
        x_assert(strSpl6.error == ERR_OK, "string_splitc strSpl6.error != ERR_OK");

        List l2 = strSpl6.value;

        u64 listSize2 = list_size(&l2);
        x_assert(listSize2 == 5, "string_splitc size strSpl6 != 5");

        HeapStr strSpl7;
        ListGetT(HeapStr, &l2, 0, &strSpl7);
        x_assertStrEq(strSpl7, "This", "string_splitc strSpl7 != \"This\"");

        HeapStr strSpl8;
        ListGetT(HeapStr, &l2, 4, &strSpl8);
        x_assertStrEq(strSpl8, "string", "string_splitc strSpl8 != \"string\"");

        list_free_items(&alloc, &l);
        list_deinit(&l);

        list_free_items(&alloc, &l2);
        list_deinit(&l2);
    }
    // =========================================================================
    // TEST string_find
    // =========================================================================
    {
        ConstStr strFin1 = "Thus is a test string";

        i64 found = string_find(strFin1, "is");
        x_assert(found == 5, "string_find strFin1 is found != 5");

        found = string_find(strFin1, "lol");
        x_assert(found == -1, "string_find strFin1 lol found != -1");

        found = string_find(strFin1, "string");
        x_assert(found == 15, "string_find strFin1 string found != 15");

        found = string_find(strFin1, "Thus");
        x_assert(found == 0, "string_find strFin1 Thus found != 0");

        found = string_find(strFin1, NULL);
        x_assert(found == -1, "string_find strFin1 NULL found != -1");

        found = string_find(NULL, "is");
        x_assert(found == -1, "string_find NULL2 found != -1");

        found = string_find(NULL, NULL);
        x_assert(found == -1, "string_find NULL3 found != -1");

        found = string_find(strFin1, "");
        x_assert(found == 0, "string_find strFin1 \"\" found != 0");
    }
    // =========================================================================
    // TEST StringBuilder
    // =========================================================================
    {
        ResultStrBuilder resBld = strbuilder_init(&alloc);
        x_assertErr(resBld.error, "StringBuilder resBld.error != ERR_OK");

        ResultStrBuilder resBld2 = strbuilder_init(&badAlloc);
        x_assert(resBld2.error != ERR_OK, "StringBuilder resBld2.error == ERR_OK");

        StringBuilder builder = resBld.value;

        strbuilder_push_copy(&builder, "This");

        ResultOwnedStr resStr = strbuilder_get_string(&builder);
        x_assertErr(resStr.error, "StringBuilder resStr.error != ERR_OK");

        HeapStr built = resStr.value;
        x_assertStrEq(built, "This", "StringBuilder built != \"This\"");

        alloc.free(&alloc, built);

        strbuilder_push_owned(&builder, ConstToHeapStr(" is a"));
        resStr = strbuilder_get_string(&builder);
        x_assertErr(resStr.error, "StringBuilder resStr2.error != ERR_OK");
        built = resStr.value;
        x_assertStrEq(built, "This is a", "StringBuilder built2 != \"This is a\"");

        alloc.free(&alloc, built);

        strbuilder_push_copy(&builder, " test.");
        resStr = strbuilder_get_string(&builder);
        x_assertErr(resStr.error, "StringBuilder resStr3.error != ERR_OK");
        built = resStr.value;
        x_assertStrEq(built, "This is a test.", "StringBuilder built3 != \"This is a test.\"");

        alloc.free(&alloc, built);

        strbuilder_deinit(&builder);
    }
}

void _xstd_list_tests(Allocator alloc)
{
    Error err;
    Allocator badAlloc = _xstd_bad_alloc();

    // =========================================================================
    // TEST list_init
    // =========================================================================
    {
        ResultList res = list_init(&alloc, sizeof(u64), 16);
        x_assertErr(res.error, "list_init res.error != ERR_OK");

        List l = res.value;

        u64 listSize = list_size(&l);
        x_assert(listSize == 0, "list_init size l != 0");

        list_deinit(&l);

        ResultList res2 = list_init(&badAlloc, sizeof(u64), 16);
        x_assert(res2.error != ERR_OK, "list_init res2.error == ERR_OK");

        ResultList res3 = list_init(&alloc, 0, 16);
        x_assert(res3.error != ERR_OK, "list_init res3.error == ERR_OK");
    }
    // =========================================================================
    // TEST list_push
    // =========================================================================
    {
        ResultList res = list_init(&alloc, sizeof(u64), 16);
        x_assertErr(res.error, "list_push res.error != ERR_OK");

        List l = res.value;

        u64 item = 5;
        list_push(&l, &item);

        u64 listSize = list_size(&l);
        x_assert(listSize == 1, "list_push size l != 1");

        for (u64 i = 0; i < 16; ++i)
        {
            list_push(&l, &item);
        }

        listSize = list_size(&l);
        x_assert(listSize == 17, "list_push size l != 17");
        x_assert(l._allocCnt == 16 * 2, "list_push allocsize l != 16*2");

        list_deinit(&l);
    }
    // =========================================================================
    // TEST list_pop
    // =========================================================================
    {
        ResultList res = list_init(&alloc, sizeof(u64), 16);
        x_assertErr(res.error, "list_pop res.error != ERR_OK");

        List l = res.value;

        u64 item = 5;
        list_push(&l, &item);

        u64 out = 0;
        err = list_pop(&l, &out);
        x_assertErr(err, "list_pop err1 != ERR_OK");
        x_assert(out == 5, "list_pop out != 5");

        u64 listSize = list_size(&l);
        x_assert(listSize == 0, "list_push size l != 0");

        for (u64 i = 0; i < 17; ++i)
        {
            list_push(&l, &item);
        }

        err = list_pop(&l, &out);
        x_assertErr(err, "list_pop err2 != ERR_OK");

        err = list_pop(&l, &out);
        x_assertErr(err, "list_pop err3 != ERR_OK");

        listSize = list_size(&l);
        x_assert(listSize == 15, "list_push size l != 15");
        x_assert(l._allocCnt == 16, "list_push allocsize l != 16");

        list_deinit(&l);
    }
    // =========================================================================
    // TEST list_get
    // =========================================================================
    {
        ResultList res = list_init(&alloc, sizeof(u64), 16);
        x_assertErr(res.error, "list_get res.error != ERR_OK");

        List l = res.value;

        u64 item = 5;
        list_push(&l, &item);

        u64 out = 0;
        err = list_get(&l, 0, &out);
        x_assertErr(err, "list_get err != ERR_OK");
        x_assert(out == 5, "list_get out != 5");

        u64 out2 = 0;
        err = list_get(&l, 5, &out2);
        x_assert(err != ERR_OK, "list_get err2 == ERR_OK");

        err = list_get(&l, 5, NULL);
        x_assert(err != ERR_OK, "list_get err3 == ERR_OK");

        u64 out3;
        err = list_get(NULL, 0, &out3);
        x_assert(err != ERR_OK, "list_get err4 == ERR_OK");

        list_deinit(&l);
    }
    // =========================================================================
    // TEST list_set
    // =========================================================================
    {
        ResultList res = list_init(&alloc, sizeof(u64), 16);
        x_assertErr(res.error, "list_set res.error != ERR_OK");

        List l = res.value;

        u64 item = 5;
        list_push(&l, &item);

        item = 1;
        list_set(&l, 0, &item);

        u64 out = 0;
        err = list_get(&l, 0, &out);
        x_assertErr(err, "list_set err != ERR_OK");
        x_assert(out == 1, "list_set out != 1");

        list_deinit(&l);
    }
    // =========================================================================
    // TEST list_getref
    // =========================================================================
    {
        ResultList res = list_init(&alloc, sizeof(u64), 16);
        x_assertErr(res.error, "list_getref res.error != ERR_OK");

        List l = res.value;

        u64 item = 5;
        list_push(&l, &item);

        u64 *ref = list_getref(&l, 0);
        x_assert(ref != NULL, "list_getref ref == NULL");
        x_assert(*ref == 5, "list_getref *ref != 5");

        u64 *ref2 = list_getref(&l, 5);
        x_assert(ref2 == NULL, "list_getref ref2 != NULL");

        u64 *ref3 = list_getref(NULL, 0);
        x_assert(ref3 == NULL, "list_getref ref3 != NULL");

        list_deinit(&l);
    }
    // =========================================================================
    // TEST list_get_unsafe
    // =========================================================================
    {
        ResultList res = list_init(&alloc, sizeof(u64), 16);
        x_assertErr(res.error, "list_get_unsafe res.error != ERR_OK");

        List l = res.value;

        u64 item = 5;
        list_push(&l, &item);

        u64 out = 0;
        list_get_unsafe(&l, 0, &out);
        x_assert(out == 5, "list_get_unsafe out != 5");

        list_deinit(&l);
    }
    // =========================================================================
    // TEST list_set_unsafe
    // =========================================================================
    {
        ResultList res = list_init(&alloc, sizeof(u64), 16);
        x_assertErr(res.error, "list_set_unsafe res.error != ERR_OK");

        List l = res.value;

        u64 item = 5;
        list_push(&l, &item);

        item = 1;
        list_set_unsafe(&l, 0, &item);

        u64 out = 0;
        err = list_get(&l, 0, &out);
        x_assertErr(err, "list_set_unsafe err != ERR_OK");
        x_assert(out == 1, "list_set_unsafe out != 1");

        list_deinit(&l);
    }
    // =========================================================================
    // TEST list_getref_unsafe
    // =========================================================================
    {
        ResultList res = list_init(&alloc, sizeof(u64), 16);
        x_assertErr(res.error, "list_getref_unsafe res.error != ERR_OK");

        List l = res.value;

        u64 item = 5;
        list_push(&l, &item);

        u64 *ref = list_getref_unsafe(&l, 0);
        x_assert(ref != NULL, "list_getref_unsafe ref == NULL");
        x_assert(*ref == 5, "list_getref_unsafe *ref != 5");

        list_deinit(&l);
    }
    // =========================================================================
    // TEST list_free_items
    // =========================================================================
    {
        ResultList res = ListInitT(HeapStr, &alloc);
        x_assertErr(res.error, "list_free_items res.error != ERR_OK");

        List l = res.value;

        HeapStr str1 = ConstToHeapStr("Test string 1");
        HeapStr str2 = ConstToHeapStr("Test string 2");

        ListPushT(HeapStr, &l, &str1);
        ListPushT(HeapStr, &l, &str2);

        list_free_items(&alloc, &l);
        list_deinit(&l);
    }
    // =========================================================================
    // TEST list_clear
    // =========================================================================
    {
        ResultList res = list_init(&alloc, sizeof(u64), 16);
        x_assertErr(res.error, "list_clear res.error != ERR_OK");

        List l = res.value;

        u64 item = 5;
        list_push(&l, &item);

        list_clear(&l);
        u64 listSize = list_size(&l);
        x_assert(listSize == 0, "list_clear listSize != 0");

        list_deinit(&l);
    }
    // =========================================================================
    // TEST list_clear_nofree
    // =========================================================================
    {
        ResultList res = list_init(&alloc, sizeof(u64), 16);
        x_assertErr(res.error, "list_clear_nofree res.error != ERR_OK");

        List l = res.value;

        u64 item = 5;
        list_push(&l, &item);

        list_clear_nofree(&l);
        u64 listSize = list_size(&l);
        x_assert(listSize == 0, "list_clear_nofree listSize != 0");

        list_deinit(&l);
    }
    // =========================================================================
    // TEST list_for_each
    // =========================================================================
    {
        ResultList res = ListInitT(HeapStr, &alloc);
        x_assertErr(res.error, "list_for_each res.error != ERR_OK");

        List l = res.value;

        HeapStr str1 = ConstToHeapStr("Test string 1");
        HeapStr str2 = ConstToHeapStr("Test string 2");

        ListPushT(HeapStr, &l, &str1);
        ListPushT(HeapStr, &l, &str2);

        list_for_each(&l, _xstd_foreach_test);

        HeapStr *strRef1 = list_getref(&l, 0);
        x_assertStrEq(*strRef1, " est string 1", "list_for_each *strRef1 != \" est string 1\"");

        HeapStr *strRef2 = list_getref(&l, 1);
        x_assertStrEq(*strRef2, " est string 2", "list_for_each *strRef2 != \" est string 2\"");

        list_free_items(&alloc, &l);
        list_deinit(&l);
    }
}
