#pragma once

#include "../xstd/xstd_core.h"
#include "../xstd/xstd_alloc.h"
#include "../xstd/xstd_io.h"
#include "../xstd/xstd_string.h"

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

void _xstd_foreach_test(void *itemPtr, u64 index, void* userArg)
{
    (void)userArg;
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
        assert_true(string_size(strLen22) == 22, "string_size 22 != 22");

        ConstStr strLen0 = "";
        assert_true(string_size(strLen0) == 0, "string_size 0 != 0");

        ConstStr strLenNull = NULL;
        assert_true(string_size(strLenNull) == 0, "string_size NULL != 0");
    }
    // =========================================================================
    // TEST string_equals
    // =========================================================================
    {
        ConstStr strEq1 = "This is equal.";
        ConstStr strEq2 = "This is equal.";
        assert_true(string_equals(strEq1, strEq2), "string_equals strEq1 != strEq2");
        assert_true(string_equals(strEq2, strEq1), "string_equals strEq2 != strEq1");

        ConstStr strNeq3 = "This is not equal.";
        assert_true(!string_equals(strEq1, strNeq3), "string_equals strEq1 == strNeq3");
        assert_true(!string_equals(strNeq3, strEq1), "string_equals strNeq3 == strEq1");

        ConstStr strShortEq1 = "a";
        ConstStr strShortEq2 = "a";
        assert_true(string_equals(strShortEq1, strShortEq2), "string_equals strShortEq1 != strShortEq2");
        assert_true(string_equals(strShortEq2, strShortEq1), "string_equals strShortEq2 != strShortEq1");

        ConstStr strShortNeq3 = "b";
        assert_true(!string_equals(strShortEq1, strShortNeq3), "string_equals strShortEq1 == strShortNeq3");
        assert_true(!string_equals(strShortNeq3, strShortEq1), "string_equals strShortNeq3 == strShortEq1");

        ConstStr strShortEq4 = "ab";
        assert_true(string_equals(strShortEq4, strShortEq4), "string_equals strShortEq4 != strShortEq4");

        assert_true(!string_equals(strEq1, NULL), "string_equals strEq1 == NULL");
        assert_true(!string_equals(NULL, strEq1), "string_equals NULL == strEq1");
        assert_true(string_equals(NULL, NULL), "string_equals NULL != NULL");
    }
    // =========================================================================
    // TEST string_alloc
    // =========================================================================
    {
        ResultOwnedStr strHeapRes1 = string_alloc(&alloc, 5, ' ');
        assert_true(strHeapRes1.error.code == ERR_OK, "string_alloc strHeapRes1.error.code != ERR_OK");

        HeapStr strHeap1 = strHeapRes1.value;
        assert_true(strHeap1 != NULL, "string_alloc strHeapRes1.value == NULL");
        assert_true(string_size(strHeap1) == 5, "string_alloc size strHeapRes1.value != 5");
        assert_true(string_equals(strHeap1, "     "), "string_alloc strHeap1 != \"     \"");

        ResultOwnedStr strHeapRes2 = string_alloc(&alloc, 0, ' ');
        assert_true(strHeapRes2.error.code == ERR_OK, "string_alloc strHeapRes2.error.code != ERR_OK");

        HeapStr strHeap2 = strHeapRes2.value;
        assert_true(strHeap2 != NULL, "string_alloc strHeapRes2.value == NULL");
        assert_true(string_size(strHeap2) == 0, "string_alloc size strHeapRes2.value != 0");
        assert_true(string_equals(strHeap2, ""), "string_alloc strHeap2 != \"\"");

        ResultOwnedStr strHeapRes3 = string_alloc(&badAlloc, 5, ' ');
        assert_true(strHeapRes3.error.code != ERR_OK, "string_alloc strHeapRes3.error.code == ERR_OK");

        alloc.free(&alloc, strHeap1);
        alloc.free(&alloc, strHeap2);
    }
    // =========================================================================
    // TEST string_copy_unsafe
    // =========================================================================
    {
        ConstStr strCopyUn1 = "Copied.";

        ResultOwnedStr strCopyUnRes2 = string_alloc(&alloc, 7, ' ');
        assert_true(strCopyUnRes2.error.code == ERR_OK, "string_copy_unsafe strCopyUnRes2.error.code != ERR_OK");
        HeapStr strCopyUn2 = strCopyUnRes2.value;
        string_copy_unsafe(strCopyUn1, strCopyUn2);
        assert_true(string_equals(strCopyUn1, strCopyUn2), "string_copy_unsafe strCopyUn1 != strCopyUn2");

        ResultOwnedStr strCopyUnRes3 = string_alloc(&alloc, 10, ' ');
        assert_true(strCopyUnRes3.error.code == ERR_OK, "string_copy_unsafe strCopyUnRes3.error.code != ERR_OK");
        HeapStr strCopyUn3 = strCopyUnRes3.value;
        string_copy_unsafe(strCopyUn1, strCopyUn3);
        assert_true(string_equals(strCopyUn3, strCopyUn1), "string_copy_unsafe strCopyUn3 != strCopyUn1");

        alloc.free(&alloc, strCopyUn2);
        alloc.free(&alloc, strCopyUn3);
    }
    // =========================================================================
    // TEST string_copy_n_unsafe
    // =========================================================================
    {
        ConstStr strCopyNUn1 = "Copied.";

        ResultOwnedStr strCopyNUnRes2 = string_alloc(&alloc, 7, ' ');
        assert_true(strCopyNUnRes2.error.code == ERR_OK, "string_copy_n_unsafe strCopyNUnRes2.error.code != ERR_OK");
        HeapStr strCopyNUn2 = strCopyNUnRes2.value;
        string_copy_n_unsafe(strCopyNUn1, strCopyNUn2, 7, false);
        assert_true(string_equals(strCopyNUn1, strCopyNUn2), "string_copy_n_unsafe strCopyNUn1 != strCopyNUn2");

        ResultOwnedStr strCopyNUnRes3 = string_alloc(&alloc, 10, ' ');
        assert_true(strCopyNUnRes3.error.code == ERR_OK, "string_copy_n_unsafe strCopyNUnRes3.error.code != ERR_OK");
        HeapStr strCopyNUn3 = strCopyNUnRes3.value;
        string_copy_n_unsafe(strCopyNUn1, strCopyNUn3, 7, false);
        assert_true(string_equals(strCopyNUn3, "Copied.   "), "string_copy_n_unsafe strCopyNUn3 != \"Copied.   \"");

        ResultOwnedStr strCopyNUnRes4 = string_alloc(&alloc, 10, ' ');
        assert_true(strCopyNUnRes4.error.code == ERR_OK, "string_copy_n_unsafe strCopyNUnRes4.error.code != ERR_OK");
        HeapStr strCopyNUn4 = strCopyNUnRes4.value;
        string_copy_n_unsafe(strCopyNUn1, strCopyNUn4, 3, false);
        assert_true(string_equals(strCopyNUn4, "Cop       "), "string_copy_n_unsafe strCopyNUn4 != \"Cop       \"");

        ResultOwnedStr strCopyNUnRes5 = string_alloc(&alloc, 10, ' ');
        assert_true(strCopyNUnRes5.error.code == ERR_OK, "string_copy_n_unsafe strCopyNUnRes5.error.code != ERR_OK");
        HeapStr strCopyNUn5 = strCopyNUnRes5.value;
        string_copy_n_unsafe(strCopyNUn1, strCopyNUn5, 0, false);
        assert_true(string_equals(strCopyNUn5, "          "), "string_copy_n_unsafe strCopyNUn5 != \"          \"");

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
        assert_true(strCopyRes2.error.code == ERR_OK, "string_copy strCopyRes2.error.code != ERR_OK");
        HeapStr strCopy2 = strCopyRes2.value;
        err = string_copy(strCopy1, strCopy2);
        assert_true(err.code == ERR_OK, "string_copy strCopy2 err.code != ERR_OK");
        assert_true(string_equals(strCopy1, strCopy2), "string_copy strCopy1 != strCopy2");

        ResultOwnedStr strCopyRes3 = string_alloc(&alloc, 10, ' ');
        assert_true(strCopyRes3.error.code == ERR_OK, "string_copy strCopyRes3.error.code != ERR_OK");
        HeapStr strCopy3 = strCopyRes3.value;
        err = string_copy(strCopy1, strCopy3);
        assert_true(err.code == ERR_OK, "string_copy strCopy3 err.code != ERR_OK");
        assert_true(string_equals(strCopy3, strCopy1), "string_copy strCopy3 != strCopy1");

        err = string_copy(strCopy1, NULL);
        assert_true(err.code != ERR_OK, "string_copy NULL arg2 err.code == ERR_OK");

        err = string_copy(NULL, strCopy3);
        assert_true(err.code != ERR_OK, "string_copy NULL arg1 err.code == ERR_OK");

        err = string_copy(NULL, NULL);
        assert_true(err.code != ERR_OK, "string_copy NULL args err.code == ERR_OK");

        err = string_copy("                              ", strCopy2);
        assert_true(err.code != ERR_OK, "string_copy arg2 smaller err.code == ERR_OK");

        alloc.free(&alloc, strCopy2);
        alloc.free(&alloc, strCopy3);
    }
    // =========================================================================
    // TEST string_copy_n
    // =========================================================================
    {
        ConstStr strCopyN1 = "Copied.";

        ResultOwnedStr strCopyNRes2 = string_alloc(&alloc, 7, ' ');
        assert_true(strCopyNRes2.error.code == ERR_OK, "string_copy_n strCopyNRes2.error.code != ERR_OK");
        HeapStr strCopyN2 = strCopyNRes2.value;
        err = string_copy_n(strCopyN1, strCopyN2, 7, false);
        assert_true(err.code == ERR_OK, "string_copy_n strCopyN2 err.code != ERR_OK");
        assert_true(string_equals(strCopyN1, strCopyN2), "string_copy_n strCopyN1 != strCopyN2");

        ResultOwnedStr strCopyNRes3 = string_alloc(&alloc, 10, ' ');
        assert_true(strCopyNRes3.error.code == ERR_OK, "string_copy_n strCopyNRes3.error.code != ERR_OK");
        HeapStr strCopyN3 = strCopyNRes3.value;
        err = string_copy_n(strCopyN1, strCopyN3, 7, false);
        assert_true(err.code == ERR_OK, "string_copy_n strCopyN3 err.code != ERR_OK");
        assert_true(string_equals(strCopyN3, "Copied.   "), "string_copy_n strCopyN3 != \"Copied.   \"");

        ResultOwnedStr strCopyNRes4 = string_alloc(&alloc, 10, ' ');
        assert_true(strCopyNRes4.error.code == ERR_OK, "string_copy_n strCopyNRes4.error.code != ERR_OK");
        HeapStr strCopyN4 = strCopyNRes4.value;
        err = string_copy_n(strCopyN1, strCopyN4, 3, false);
        assert_true(err.code == ERR_OK, "string_copy_n strCopyN4 err.code != ERR_OK");
        assert_true(string_equals(strCopyN4, "Cop       "), "string_copy_n strCopyN4 != \"Cop       \"");

        ResultOwnedStr strCopyNRes5 = string_alloc(&alloc, 10, ' ');
        assert_true(strCopyNRes5.error.code == ERR_OK, "string_copy_n strCopyNRes5.error.code != ERR_OK");
        HeapStr strCopyN5 = strCopyNRes5.value;
        err = string_copy_n(strCopyN1, strCopyN5, 0, false);
        assert_true(err.code == ERR_OK, "string_copy_n strCopyN5 err.code != ERR_OK");
        assert_true(string_equals(strCopyN5, "          "), "string_copy_n strCopyN5 != \"          \"");

        err = string_copy_n(strCopyN1, NULL, 5, false);
        assert_true(err.code != ERR_OK, "string_copy_n NULL arg2 err.code == ERR_OK");

        err = string_copy_n(NULL, strCopyN3, 5, false);
        assert_true(err.code != ERR_OK, "string_copy_n NULL arg1 err.code == ERR_OK");

        err = string_copy_n(NULL, NULL, 5, false);
        assert_true(err.code != ERR_OK, "string_copy_n NULL args err.code == ERR_OK");

        err = string_copy_n(" ", strCopyN2, 5, false);
        assert_true(err.code != ERR_OK, "string_copy_n arg1 smaller than n err.code == ERR_OK");

        err = string_copy_n("             ", strCopyN2, 8, false);
        assert_true(err.code != ERR_OK, "string_copy_n arg2 smaller than n err.code == ERR_OK");

        err = string_copy_n(strCopyN1, strCopyN3, 0, false);
        assert_true(err.code == ERR_OK, "string_copy_n 0 arg3 err.code != ERR_OK");

        err = string_copy_n(strCopyN1, strCopyN3, 99, false);
        assert_true(err.code != ERR_OK, "string_copy_n 99 arg3 err.code == ERR_OK");

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
        assert_true(strDupe2.error.code == ERR_OK, "string_dupe strDupe2.error.code != ERR_OK");
        assert_true(string_equals(strDupe1, strDupe2.value), "string_dupe strDupe1 != strDupe2");

        ResultOwnedStr strDupe3 = string_dupe(&alloc, strDupe0);
        assert_true(strDupe3.error.code == ERR_OK, "string_dupe strDupe3.error.code != ERR_OK");
        assert_true(string_equals(strDupe0, strDupe3.value), "string_dupe strDupe0 != strDupe3");

        ResultOwnedStr strDupe4 = string_dupe(&alloc, NULL);
        assert_true(strDupe4.error.code != ERR_OK, "string_dupe strDupe4.error.code == ERR_OK");

        ResultOwnedStr strDupe5 = string_dupe(&badAlloc, strDupe1);
        assert_true(strDupe5.error.code != ERR_OK, "string_dupe strDupe5.error.code == ERR_OK");

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
        assert_true(strDupeNr2 != NULL, "string_dupe_noresult strDupeNr2 != NULL");
        assert_true(string_equals(strDupeNr1, strDupeNr2), "string_dupe_noresult strDupeNr1 != strDupeNr2");

        HeapStr strDupeNr3 = string_dupe_noresult(&alloc, strDupeNr0);
        assert_true(strDupeNr3 != NULL, "string_dupe_noresult strDupeNr3 != NULL");
        assert_true(string_equals(strDupeNr0, strDupeNr3), "string_dupe_noresult strDupeNr0 != strDupeNr3");

        HeapStr strDupeNr4 = string_dupe_noresult(&alloc, NULL);
        assert_true(strDupeNr4 == NULL, "string_dupe_noresult strDupeNr4 != NULL");

        HeapStr strDupeNr5 = string_dupe_noresult(&badAlloc, strDupeNr1);
        assert_true(strDupeNr5 == NULL, "string_dupe_noresult strDupeNr5 != NULL");

        alloc.free(&alloc, strDupeNr2);
        alloc.free(&alloc, strDupeNr3);
    }
    // =========================================================================
    // TEST string_resize
    // =========================================================================
    {
        ConstStr strRes1 = "Resized";

        ResultOwnedStr strRes2 = string_resize(&alloc, strRes1, 15, '_');
        assert_true(strRes2.error.code == ERR_OK, "string_resize strRes2.error.code != ERR_OK");
        assert_true(string_equals(strRes2.value, "Resized________"), "string_resize strRes2 != \"Resized________\"");

        ResultOwnedStr strRes3 = string_resize(&alloc, strRes1, 3, '_');
        assert_true(strRes3.error.code == ERR_OK, "string_resize strRes3.error.code != ERR_OK");
        assert_true(string_equals(strRes3.value, "Res"), "string_resize strRes3 != \"Res\"");

        ResultOwnedStr strRes4 = string_resize(&alloc, NULL, 12, '_');
        assert_true(strRes4.error.code != ERR_OK, "string_resize strRes4.error.code == ERR_OK");

        ResultOwnedStr strRes5 = string_resize(&alloc, strRes1, 0, '_');
        assert_true(strRes5.error.code == ERR_OK, "string_resize strRes5.error.code != ERR_OK");
        assert_true(string_equals(strRes5.value, ""), "string_resize strRes5 != \"\"");

        ResultOwnedStr strRes6 = string_resize(&badAlloc, strRes1, 12, '_');
        assert_true(strRes6.error.code != ERR_OK, "string_resize strRes6.error.code == ERR_OK");

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
        assert_true(strConc3.error.code == ERR_OK, "string_concat strConc3.error.code != ERR_OK");
        assert_true(string_equals(strConc3.value, "Left Right"), "string_concat strConc3 != \"Left Right\"");

        ResultOwnedStr strConc4 = string_concat(&alloc, strConc1, NULL);
        assert_true(strConc4.error.code != ERR_OK, "string_concat strConc4.error.code == ERR_OK");

        ResultOwnedStr strConc5 = string_concat(&alloc, NULL, strConc2);
        assert_true(strConc5.error.code != ERR_OK, "string_concat strConc5.error.code == ERR_OK");

        ResultOwnedStr strConc6 = string_concat(&badAlloc, strConc1, strConc2);
        assert_true(strConc6.error.code != ERR_OK, "string_concat strConc6.error.code == ERR_OK");

        alloc.free(&alloc, strConc3.value);
        alloc.free(&alloc, strConc4.value);
    }
    // =========================================================================
    // TEST string_substr
    // =========================================================================
    {
        ConstStr strSub1 = "This is a substring";

        ResultOwnedStr strSub2 = string_substr(&alloc, strSub1, 10, 19);

        assert_true(strSub2.error.code == ERR_OK, "string_substr strSub2.error.code != ERR_OK");
        assert_true(string_equals(strSub2.value, "substring"), "string_substr strSub2 != \"substring\"");

        ResultOwnedStr strSub3 = string_substr(&alloc, strSub1, 0, 4);
        assert_true(strSub3.error.code == ERR_OK, "string_substr strSub3.error.code != ERR_OK");
        assert_true(string_equals(strSub3.value, "This"), "string_substr strSub3 != \"This\"");

        ResultOwnedStr strSub4 = string_substr(&alloc, strSub1, 10, 20);
        assert_true(strSub4.error.code != ERR_OK, "string_substr strSub4.error.code == ERR_OK");

        ResultOwnedStr strSub5 = string_substr(&alloc, NULL, 10, 20);
        assert_true(strSub5.error.code != ERR_OK, "string_substr strSub5.error.code == ERR_OK");

        ResultOwnedStr strSub6 = string_substr(&badAlloc, strSub1, 10, 19);
        assert_true(strSub6.error.code != ERR_OK, "string_substr strSub6.error.code == ERR_OK");

        alloc.free(&alloc, strSub2.value);
        alloc.free(&alloc, strSub3.value);
    }
    // =========================================================================
    // TEST string_substr_unsafe
    // =========================================================================
    {
        ConstStr strSub1 = "This is a substring";

        HeapStr strSub2 = string_substr_unsafe(&alloc, strSub1, 10, 19);

        assert_true(strSub2 != NULL, "string_substr_unsafe strSub2 == NULL");
        assert_true(string_equals(strSub2, "substring"), "string_substr_unsafe strSub2 != \"substring\"");

        HeapStr strSub3 = string_substr_unsafe(&alloc, strSub1, 0, 4);
        assert_true(strSub3 != NULL, "string_substr_unsafe strSub3 == NULL");
        assert_true(string_equals(strSub3, "This"), "string_substr_unsafe strSub3 != \"This\"");

        HeapStr strSub6 = string_substr_unsafe(&badAlloc, strSub1, 10, 19);
        assert_true(strSub6 == NULL, "string_substr_unsafe strSub6 != NULL");

        alloc.free(&alloc, strSub2);
        alloc.free(&alloc, strSub3);
    }
    // =========================================================================
    // TEST string_splitc
    // =========================================================================
    {
        ConstStr strSpl0 = " This is a split string ";
        ConstStr strSpl1 = "This is a  split string";

        ResultList strSpl2 = string_split_char(&alloc, strSpl1, ' ');
        assert_ok(strSpl2.error, "string_splitc strSpl2.error.code != ERR_OK");

        List l = strSpl2.value;

        u64 listSize = list_size(&l);
        assert_true(listSize == 6, "string_splitc size strSpl2 != 6");

        HeapStr strSpl3 = NULL;
        ListGetT(HeapStr, &l, 0, &strSpl3);
        assert_true(!!strSpl3, "string_splitc strSpl3 == NULL");
        assert_str_eq(strSpl3, "This", "string_splitc strSpl3 != \"This\"");

        HeapStr strSpl3_2 = NULL;
        ListGetT(HeapStr, &l, 3, &strSpl3_2);
        assert_true(!!strSpl3_2, "string_splitc strSpl3_2 == NULL");
        assert_str_eq(strSpl3_2, "", "string_splitc strSpl3_2 != \"\"");

        HeapStr strSpl3_5 = NULL;
        ListGetT(HeapStr, &l, 4, &strSpl3_5);
        assert_true(!!strSpl3_5, "string_splitc strSpl3_5 == NULL");
        assert_str_eq(strSpl3_5, "split", "string_splitc strSpl3_5 != \"split\"");

        HeapStr strSpl4 = NULL;
        ListGetT(HeapStr, &l, 5, &strSpl4);
        assert_true(!!strSpl4, "string_splitc strSpl4 == NULL");
        assert_str_eq(strSpl4, "string", "string_splitc strSpl4 != \"string\"");

        ResultList strSpl5 = string_split_char(&alloc, NULL, ' ');
        assert_true(strSpl5.error.code != ERR_OK, "string_splitc strSpl5.error.code == ERR_OK");

        ResultList strSpl6 = string_split_char(&alloc, strSpl0, ' ');
        assert_true(strSpl6.error.code == ERR_OK, "string_splitc strSpl6.error.code != ERR_OK");

        List l2 = strSpl6.value;

        u64 listSize2 = list_size(&l2);
        assert_true(listSize2 == 7, "string_splitc size strSpl7 != 5");

        HeapStr strSpl7 = NULL;
        ListGetT(HeapStr, &l2, 0, &strSpl7);
        assert_true(!!strSpl7, "string_splitc strSpl7 == NULL");
        assert_str_eq(strSpl7, "", "string_splitc strSpl7 != \"\"");

        HeapStr strSpl7_5 = NULL;
        ListGetT(HeapStr, &l2, 1, &strSpl7_5);
        assert_true(!!strSpl7_5, "string_splitc strSpl7_5 == NULL");
        assert_str_eq(strSpl7_5, "This", "string_splitc strSpl7_5 != \"This\"");

        HeapStr strSpl8 = NULL;
        ListGetT(HeapStr, &l2, 5, &strSpl8);
        assert_true(!!strSpl8, "string_splitc strSpl8 == NULL");
        assert_str_eq(strSpl8, "string", "string_splitc strSpl8 != \"string\"");

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
        assert_true(found == 5, "string_find strFin1 is found != 5");

        found = string_find(strFin1, "lol");
        assert_true(found == -1, "string_find strFin1 lol found != -1");

        found = string_find(strFin1, "string");
        assert_true(found == 15, "string_find strFin1 string found != 15");

        found = string_find(strFin1, "Thus");
        assert_true(found == 0, "string_find strFin1 Thus found != 0");

        found = string_find(strFin1, NULL);
        assert_true(found == -1, "string_find strFin1 NULL found != -1");

        found = string_find(NULL, "is");
        assert_true(found == -1, "string_find NULL2 found != -1");

        found = string_find(NULL, NULL);
        assert_true(found == -1, "string_find NULL3 found != -1");

        found = string_find(strFin1, "");
        assert_true(found == 0, "string_find strFin1 \"\" found != 0");
    }
    // =========================================================================
    // TEST string_find_char
    // =========================================================================
    {
        ConstStr strFin1 = "Thus is a test string";

        i64 found = string_find_char(strFin1, 'u');
        assert_true(found == 2, "string_find_char strFin1 u found != 2");

        found = string_find_char(strFin1, 'r');
        assert_true(found == 17, "string_find_char strFin1 r found != 17");

        found = string_find_char(strFin1, 'w');
        assert_true(found == -1, "string_find_char strFin1 w found != -1");

        found = string_find_char(strFin1, 0);
        assert_true(found == -1, "string_find_char strFin1 0 found != -1");

        found = string_find_char(NULL, 'a');
        assert_true(found == -1, "string_find_char NULL2 found != -1");

        found = string_find_char(NULL, 0);
        assert_true(found == -1, "string_find_char NULL3 found != -1");
    }
    // =========================================================================
    // TEST StringBuilder
    // =========================================================================
    {
        ResultStrBuilder resBld = strbuilder_init(&alloc);
        assert_ok(resBld.error, "StringBuilder resBld.error.code != ERR_OK");

        ResultStrBuilder resBld2 = strbuilder_init(&badAlloc);
        assert_true(resBld2.error.code != ERR_OK, "StringBuilder resBld2.error.code == ERR_OK");

        StringBuilder builder = resBld.value;

        strbuilder_push_copy(&builder, "This");

        ResultOwnedStr resStr = strbuilder_get_string(&builder);
        assert_ok(resStr.error, "StringBuilder resStr.error.code != ERR_OK");

        HeapStr built = resStr.value;
        assert_str_eq(built, "This", "StringBuilder built != \"This\"");

        alloc.free(&alloc, built);

        strbuilder_push_owned(&builder, ConstToHeapStr(" is a"));
        resStr = strbuilder_get_string(&builder);
        assert_ok(resStr.error, "StringBuilder resStr2.error.code != ERR_OK");
        built = resStr.value;
        assert_str_eq(built, "This is a", "StringBuilder built2 != \"This is a\"");

        alloc.free(&alloc, built);

        strbuilder_push_copy(&builder, " test.");
        resStr = strbuilder_get_string(&builder);
        assert_ok(resStr.error, "StringBuilder resStr3.error.code != ERR_OK");
        built = resStr.value;
        assert_str_eq(built, "This is a test.", "StringBuilder built3 != \"This is a test.\"");

        alloc.free(&alloc, built);

        strbuilder_deinit(&builder);
    }
    // =========================================================================
    // TEST string_replace
    // =========================================================================
    {
        ConstStr strRep1 = "This is a test";

        ResultOwnedStr resRep1 = string_replace(&alloc, strRep1, "is", "os");
        assert_ok(resRep1.error, "string_replace resRep1 != ERR_OK");
        assert_str_eq(resRep1.value, "Thos os a test", "string_replace resRep1 != \"Thos os a test\"");

        ResultOwnedStr resRep2 = string_replace(&alloc, strRep1, " a ", " a burger ");
        assert_ok(resRep2.error, "string_replace resRep2 != ERR_OK");
        assert_str_eq(resRep2.value, "This is a burger test", "string_replace resRep2 != \"This is a burger test\"");

        ResultOwnedStr resRep3 = string_replace(&alloc, strRep1, NULL, " a burger ");
        assert_true(resRep3.error.code != ERR_OK, "string_replace resRep3 == ERR_OK");

        ResultOwnedStr resRep4 = string_replace(&alloc, strRep1, " ", NULL);
        assert_true(resRep4.error.code != ERR_OK, "string_replace resRep4 == ERR_OK");

        ResultOwnedStr resRep5 = string_replace(&alloc, strRep1, NULL, NULL);
        assert_true(resRep5.error.code != ERR_OK, "string_replace resRep5 == ERR_OK");

        ResultOwnedStr resRep6 = string_replace(&alloc, NULL, NULL, NULL);
        assert_true(resRep6.error.code != ERR_OK, "string_replace resRep6 == ERR_OK");

        alloc.free(&alloc, resRep1.value);
        alloc.free(&alloc, resRep2.value);
    }
    // =========================================================================
    // TEST string_starts_with
    // =========================================================================
    {
        ibool strStaWth1 = string_starts_with("This is a test", "This");
        assert_true(strStaWth1, "string_starts_with strStaWth1 != true");

        ibool strStaWth2 = string_starts_with("This is a test", "Though");
        assert_true(!strStaWth2, "string_starts_with strStaWth2 == true");

        ibool strStaWth3 = string_starts_with("This is a test", NULL);
        assert_true(!strStaWth3, "string_starts_with strStaWth3 == true");

        ibool strStaWth4 = string_starts_with(NULL, "");
        assert_true(!strStaWth4, "string_starts_with strStaWth4 == true");

        ibool strStaWth5 = string_starts_with(NULL, NULL);
        assert_true(!strStaWth5, "string_starts_with strStaWth5 == true");

        ibool strStaWth6 = string_starts_with("This is a test", "");
        assert_true(strStaWth6, "string_starts_with strStaWth6 != true");
    }
    // =========================================================================
    // TEST string_ends_with
    // =========================================================================
    {
        ibool strEndWth1 = string_ends_with("This is a test", "test");
        assert_true(strEndWth1, "string_ends_with strEndWth1 != true");

        ibool strEndWth2 = string_ends_with("This is a test", "tes");
        assert_true(!strEndWth2, "string_ends_with strEndWth2 == true");

        ibool strEndWth3 = string_ends_with("This is a test", NULL);
        assert_true(!strEndWth3, "string_ends_with strEndWth3 == true");

        ibool strEndWth4 = string_ends_with(NULL, "");
        assert_true(!strEndWth4, "string_ends_with strEndWth4 == true");

        ibool strEndWth5 = string_ends_with(NULL, NULL);
        assert_true(!strEndWth5, "string_ends_with strEndWth5 == true");

        ibool strEndWth6 = string_ends_with("This is a test", "");
        assert_true(strEndWth6, "string_ends_with strEndWth6 != true");
    }
    // =========================================================================
    // TEST char_is_alpha
    // =========================================================================
    {
        assert_true(!char_is_alpha('a' - 1), "char_is_alpha invalid range <a");

        i8 c = 'a';
        while (c <= 'z')
        {
            assert_true(char_is_alpha(c), "char_is_alpha invalid range a-z");
            ++c;
        }

        assert_true(!char_is_alpha('z' + 1), "char_is_alpha invalid range >z");

        assert_true(!char_is_alpha('A' - 1), "char_is_alpha invalid range <A");
        
        c = 'A';
        while (c <= 'Z')
        {
            assert_true(char_is_alpha(c), "char_is_alpha invalid range A-Z");
            ++c;
        }

        assert_true(!char_is_alpha('Z' + 1), "char_is_alpha invalid range >Z");
    }
    // =========================================================================
    // TEST char_is_digit
    // =========================================================================
    {
        assert_true(!char_is_digit('0' - 1), "char_is_digit invalid range <0");

        i8 c = '0';
        while (c <= '9')
        {
            assert_true(char_is_digit(c), "char_is_digit invalid range 0-9");
            ++c;
        }

        assert_true(!char_is_digit('9' + 1), "char_is_digit invalid range >9");
    }
    // =========================================================================
    // TEST char_is_alphanum
    // =========================================================================
    {
        assert_true(!char_is_alphanum('a' - 1), "char_is_alphanum invalid range <a");

        i8 c = 'a';
        while (c <= 'z')
        {
            assert_true(char_is_alphanum(c), "char_is_alphanum invalid range a-z");
            ++c;
        }

        assert_true(!char_is_alphanum('z' + 1), "char_is_alphanum invalid range >z");

        assert_true(!char_is_alphanum('A' - 1), "char_is_alphanum invalid range <A");
        
        c = 'A';
        while (c <= 'Z')
        {
            assert_true(char_is_alphanum(c), "char_is_alphanum invalid range A-Z");
            ++c;
        }

        assert_true(!char_is_alphanum('Z' + 1), "char_is_alphanum invalid range >Z");

        assert_true(!char_is_alphanum('0' - 1), "char_is_alphanum invalid range <0");

        c = '0';
        while (c <= '9')
        {
            assert_true(char_is_alphanum(c), "char_is_alphanum invalid range 0-9");
            ++c;
        }

        assert_true(!char_is_alphanum('9' + 1), "char_is_alphanum invalid range >9");
    }
    // =========================================================================
    // TEST string_trim_whitespace
    // =========================================================================
    {
        ResultOwnedStr strTrim1 = string_trim_whitespace(&alloc, "  \n  This is a test.", true, true);
        assert_ok(strTrim1.error, "string_trim_whitespace strTrim1 != OK");
        assert_str_eq(strTrim1.value, "This is a test.", "string_trim_whitespace strTrim1 != \"This is a test.\"");

        ResultOwnedStr strTrim2 = string_trim_whitespace(&alloc, "  \n  This is a test. \t  ", true, true);
        assert_ok(strTrim2.error, "string_trim_whitespace strTrim2 != OK");
        assert_str_eq(strTrim2.value, "This is a test.", "string_trim_whitespace strTrim2 != \"This is a test.\"");

        ResultOwnedStr strTrim3 = string_trim_whitespace(&alloc, NULL, true, true);
        assert_true(strTrim3.error.code != ERR_OK, "string_trim_whitespace strTrim3 == OK");

        alloc.free(&alloc, strTrim1.value);
        alloc.free(&alloc, strTrim2.value);
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
        assert_ok(res.error, "list_init res.error.code != ERR_OK");

        List l = res.value;

        u64 listSize = list_size(&l);
        assert_true(listSize == 0, "list_init size l != 0");

        list_deinit(&l);

        ResultList res2 = list_init(&badAlloc, sizeof(u64), 16);
        assert_true(res2.error.code != ERR_OK, "list_init res2.error.code == ERR_OK");

        ResultList res3 = list_init(&alloc, 0, 16);
        assert_true(res3.error.code != ERR_OK, "list_init res3.error.code == ERR_OK");
    }
    // =========================================================================
    // TEST list_push
    // =========================================================================
    {
        ResultList res = list_init(&alloc, sizeof(u64), 16);
        assert_ok(res.error, "list_push res.error.code != ERR_OK");

        List l = res.value;

        u64 item = 5;
        list_push(&l, &item);

        u64 listSize = list_size(&l);
        assert_true(listSize == 1, "list_push size l != 1");

        for (u64 i = 0; i < 16; ++i)
        {
            list_push(&l, &item);
        }

        listSize = list_size(&l);
        assert_true(listSize == 17, "list_push size l != 17");
        assert_true(l._allocCnt == 16 * 2, "list_push allocsize l != 16*2");

        list_deinit(&l);
    }
    // =========================================================================
    // TEST list_pop
    // =========================================================================
    {
        ResultList res = list_init(&alloc, sizeof(u64), 16);
        assert_ok(res.error, "list_pop res.error.code != ERR_OK");

        List l = res.value;

        u64 item = 5;
        list_push(&l, &item);

        u64 out = 0;
        err = list_pop(&l, &out);
        assert_ok(err, "list_pop err1 != ERR_OK");
        assert_true(out == 5, "list_pop out != 5");

        u64 listSize = list_size(&l);
        assert_true(listSize == 0, "list_push size l != 0");

        for (u64 i = 0; i < 17; ++i)
        {
            list_push(&l, &item);
        }

        err = list_pop(&l, &out);
        assert_ok(err, "list_pop err2 != ERR_OK");

        err = list_pop(&l, &out);
        assert_ok(err, "list_pop err3 != ERR_OK");

        listSize = list_size(&l);
        assert_true(listSize == 15, "list_push size l != 15");
        assert_true(l._allocCnt == 16, "list_push allocsize l != 16");

        list_deinit(&l);
    }
    // =========================================================================
    // TEST list_get
    // =========================================================================
    {
        ResultList res = list_init(&alloc, sizeof(u64), 16);
        assert_ok(res.error, "list_get res.error.code != ERR_OK");

        List l = res.value;

        u64 item = 5;
        list_push(&l, &item);

        u64 out = 0;
        err = list_get(&l, 0, &out);
        assert_ok(err, "list_get err.code != ERR_OK");
        assert_true(out == 5, "list_get out != 5");

        u64 out2 = 0;
        err = list_get(&l, 5, &out2);
        assert_true(err.code != ERR_OK, "list_get err2 == ERR_OK");

        err = list_get(&l, 5, NULL);
        assert_true(err.code != ERR_OK, "list_get err3 == ERR_OK");

        u64 out3;
        err = list_get(NULL, 0, &out3);
        assert_true(err.code != ERR_OK, "list_get err4 == ERR_OK");

        list_deinit(&l);
    }
    // =========================================================================
    // TEST list_set
    // =========================================================================
    {
        ResultList res = list_init(&alloc, sizeof(u64), 16);
        assert_ok(res.error, "list_set res.error.code != ERR_OK");

        List l = res.value;

        u64 item = 5;
        list_push(&l, &item);

        item = 1;
        list_set(&l, 0, &item);

        u64 out = 0;
        err = list_get(&l, 0, &out);
        assert_ok(err, "list_set err.code != ERR_OK");
        assert_true(out == 1, "list_set out != 1");

        list_deinit(&l);
    }
    // =========================================================================
    // TEST list_getref
    // =========================================================================
    {
        ResultList res = list_init(&alloc, sizeof(u64), 16);
        assert_ok(res.error, "list_getref res.error.code != ERR_OK");

        List l = res.value;

        u64 item = 5;
        list_push(&l, &item);

        u64 *ref = list_getref(&l, 0);
        assert_true(ref != NULL, "list_getref ref == NULL");
        assert_true(*ref == 5, "list_getref *ref != 5");

        u64 *ref2 = list_getref(&l, 5);
        assert_true(ref2 == NULL, "list_getref ref2 != NULL");

        u64 *ref3 = list_getref(NULL, 0);
        assert_true(ref3 == NULL, "list_getref ref3 != NULL");

        list_deinit(&l);
    }
    // =========================================================================
    // TEST list_get_unsafe
    // =========================================================================
    {
        ResultList res = list_init(&alloc, sizeof(u64), 16);
        assert_ok(res.error, "list_get_unsafe res.error.code != ERR_OK");

        List l = res.value;

        u64 item = 5;
        list_push(&l, &item);

        u64 out = 0;
        list_get_unsafe(&l, 0, &out);
        assert_true(out == 5, "list_get_unsafe out != 5");

        list_deinit(&l);
    }
    // =========================================================================
    // TEST list_set_unsafe
    // =========================================================================
    {
        ResultList res = list_init(&alloc, sizeof(u64), 16);
        assert_ok(res.error, "list_set_unsafe res.error.code != ERR_OK");

        List l = res.value;

        u64 item = 5;
        list_push(&l, &item);

        item = 1;
        list_set_unsafe(&l, 0, &item);

        u64 out = 0;
        err = list_get(&l, 0, &out);
        assert_ok(err, "list_set_unsafe err.code != ERR_OK");
        assert_true(out == 1, "list_set_unsafe out != 1");

        list_deinit(&l);
    }
    // =========================================================================
    // TEST list_getref_unsafe
    // =========================================================================
    {
        ResultList res = list_init(&alloc, sizeof(u64), 16);
        assert_ok(res.error, "list_getref_unsafe res.error.code != ERR_OK");

        List l = res.value;

        u64 item = 5;
        list_push(&l, &item);

        u64 *ref = list_getref_unsafe(&l, 0);
        assert_true(ref != NULL, "list_getref_unsafe ref == NULL");
        assert_true(*ref == 5, "list_getref_unsafe *ref != 5");

        list_deinit(&l);
    }
    // =========================================================================
    // TEST list_free_items
    // =========================================================================
    {
        ResultList res = ListInitT(HeapStr, &alloc);
        assert_ok(res.error, "list_free_items res.error.code != ERR_OK");

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
        assert_ok(res.error, "list_clear res.error.code != ERR_OK");

        List l = res.value;

        u64 item = 5;
        list_push(&l, &item);

        list_clear(&l);
        u64 listSize = list_size(&l);
        assert_true(listSize == 0, "list_clear listSize != 0");

        list_deinit(&l);
    }
    // =========================================================================
    // TEST list_clear_nofree
    // =========================================================================
    {
        ResultList res = list_init(&alloc, sizeof(u64), 16);
        assert_ok(res.error, "list_clear_nofree res.error.code != ERR_OK");

        List l = res.value;

        u64 item = 5;
        list_push(&l, &item);

        list_clear_nofree(&l);
        u64 listSize = list_size(&l);
        assert_true(listSize == 0, "list_clear_nofree listSize != 0");

        list_deinit(&l);
    }
    // =========================================================================
    // TEST list_for_each
    // =========================================================================
    {
        ResultList res = ListInitT(HeapStr, &alloc);
        assert_ok(res.error, "list_for_each res.error.code != ERR_OK");

        List l = res.value;

        HeapStr str1 = ConstToHeapStr("Test string 1");
        HeapStr str2 = ConstToHeapStr("Test string 2");

        ListPushT(HeapStr, &l, &str1);
        ListPushT(HeapStr, &l, &str2);

        list_for_each(&l, _xstd_foreach_test, NULL);

        HeapStr *strRef1 = list_getref(&l, 0);
        assert_str_eq(*strRef1, " est string 1", "list_for_each *strRef1 != \" est string 1\"");

        HeapStr *strRef2 = list_getref(&l, 1);
        assert_str_eq(*strRef2, " est string 2", "list_for_each *strRef2 != \" est string 2\"");

        list_free_items(&alloc, &l);
        list_deinit(&l);
    }
}

void _xstd_math_tests(Allocator alloc)
{
    (void)alloc;
    // =========================================================================
    // TEST add
    // =========================================================================
    {
        u8 addRes1 = math_u8_add(0, 1);
        assert_true(addRes1 == 1, "math_u8_add addRes1 != 1");

        i8 addResI1 = math_i8_add(0, 1);
        assert_true(addResI1 == 1, "math_i8_add addResI1 != 1");

        u8 addRes2 = math_u8_add(U8_MAXVAL, 1);
        assert_true(addRes2 == 0, "math_u8_add addRes2 != 0");

        u16 addRes3 = math_u16_add(0, 1);
        assert_true(addRes3 == 1, "math_u16_add addRes3 != 1");

        u16 addRes4 = math_u16_add(U16_MAXVAL, 1);
        assert_true(addRes4 == 0, "math_u16_add addRes4 != 0");

        u16 addRes5 = math_u32_add(0, 1);
        assert_true(addRes5 == 1, "math_u32_add addRes5 != 1");

        u16 addRes6 = math_u32_add(U32_MAXVAL, 1);
        assert_true(addRes6 == 0, "math_u32_add addRes6 != 0");

        u16 addRes7 = math_u64_add(0, 1);
        assert_true(addRes7 == 1, "math_u64_add addRes7 != 1");

        u16 addRes8 = math_u64_add(U64_MAXVAL, 1);
        assert_true(addRes8 == 0, "math_u64_add addRes8 != 0");
    }
    // =========================================================================
    // TEST add_nooverflow
    // =========================================================================
    {
        ResultU8 addResNo1 = math_u8_add_nooverflow(0, 1);
        assert_ok(addResNo1.error, "math_u8_add_nooverflow addResNo1 != OK");
        assert_true(addResNo1.value == 1, "math_u8_add_nooverflow addResNo1 != 1");

        ResultU8 addResNo2 = math_u8_add_nooverflow(U8_MAXVAL, 1);
        assert_true(addResNo2.error.code != ERR_OK, "math_u8_add_nooverflow addResNo2 == OK");

        ResultU16 addResNo3 = math_u16_add_nooverflow(0, 1);
        assert_ok(addResNo3.error, "math_u16_add_nooverflow addResNo3 != OK");
        assert_true(addResNo3.value == 1, "math_u16_add_nooverflow addResNo3 != 1");

        ResultU16 addResNo4 = math_u16_add_nooverflow(U16_MAXVAL, 1);
        assert_true(addResNo4.error.code != ERR_OK, "math_u16_add_nooverflow addResNo4 == OK");

        ResultU32 addResNo5 = math_u32_add_nooverflow(0, 1);
        assert_ok(addResNo5.error, "math_u32_add_nooverflow addResNo5 != OK");
        assert_true(addResNo5.value == 1, "math_u32_add_nooverflow addResNo5 != 1");

        ResultU32 addResNo6 = math_u32_add_nooverflow(U32_MAXVAL, 1);
        assert_true(addResNo6.error.code != ERR_OK, "math_u32_add_nooverflow addResNo6 == OK");

        ResultU64 addResNo7 = math_u64_add_nooverflow(0, 1);
        assert_ok(addResNo7.error, "math_u32_add_nooverflow addResNo7 != OK");
        assert_true(addResNo7.value == 1, "math_u32_add_nooverflow addResNo7 != 1");

        ResultU64 addResNo8 = math_u64_add_nooverflow(U64_MAXVAL, 1);
        assert_true(addResNo8.error.code != ERR_OK, "math_u32_add_nooverflow addResNo8 == OK");
    }
}
