#pragma once

#include "../../xstd/xstd_core.h"
#include "../../xstd/xstd_alloc.h"
#include "../../xstd/xstd_buffer.h"
#include "../../xstd/xstd_file.h"
#include "../../xstd/xstd_io.h"
#include "../../xstd/xstd_string.h"
#include "../../xstd/xstd_utf8.h"
#include "../../xstd/xstd_math.h"
#include "../../xstd/xstd_writer.h"
#include "../../xstd/xstd_mem.h"

/*
// FOR DEBUGGING
static void pause(void)
{
    io_println("[PAUSE] Press any key to continue");
    io_read_line(default_allocator());
}*/

static void *_xstd_bad_alloc_alloc(Allocator *a, u64 s)
{
    (void)a;
    (void)s;
    return NULL;
}

static void *_xstd_bad_alloc_realloc(Allocator *a, void *b, u64 s)
{
    (void)a;
    (void)b;
    (void)s;
    return NULL;
}

static void _xstd_bad_alloc_free(Allocator *a, void *b)
{
    (void)a;
    (void)b;
}

static Allocator _xstd_bad_alloc(void)
{
    return (Allocator){
        ._internalState = NULL,
        .alloc = _xstd_bad_alloc_alloc,
        .realloc = _xstd_bad_alloc_realloc,
        .free = _xstd_bad_alloc_free,
    };
}

/*
static void _xstd_print_strlist(void *itemPtr, u64 index)
{
    io_print_uint(index);
    io_print(": ");
    io_println(*(HeapStr *)itemPtr);
}*/

static void _xstd_foreach_test(void *itemPtr, u64 index, void* userArg)
{
    (void)userArg;
    (void)index;
    HeapStr itemStr = *(HeapStr *)itemPtr;
    itemStr[0] = ' ';
}

static void _xstd_file_tests(Allocator alloc)
{
    io_println("file_create");
    {
        ConstStr filePath = "xstd_file_test.tmp";
        ResultFile createRes = file_create(filePath);
        assert_ok(createRes.error, "file_create createRes.error.code != ERR_OK");

        io_println("file_write_str");

        File file = createRes.value;
        Error err = file_write_str(&file, "hello_world");
        assert_ok(err, "file_write_str err.code != ERR_OK");

        io_println("file_write_char");

        err = file_write_char(&file, '!');
        assert_ok(err, "file_write_char err.code != ERR_OK");

        io_println("file_size");

        u64 writtenSize = file_size(&file);
        assert_true(writtenSize == 12, "file_size writtenSize != 12");

        io_println("file_read_lines lf");

        file_close(&file);
        ResultFile linesFile = file_create(filePath);
        assert_ok(linesFile.error, "file_read_lines failed to recreate file for LF");
        file = linesFile.value;

        ConstStr lfContent = "line1\nline2\nline3";
        err = file_write_str(&file, lfContent);
        assert_ok(err, "file_read_lines failed to write LF content");

        err = file_rewind(&file);
        assert_ok(err, "file_read_lines LF rewind err.code != ERR_OK");

        ResultList lfLinesRes = file_read_lines(&alloc, &file);
        assert_ok(lfLinesRes.error, "file_read_lines LF lfLinesRes.error.code != ERR_OK");
        List lfLines = lfLinesRes.value;
        assert_true(list_size(&lfLines) == 3, "file_read_lines LF count != 3");

        String lfLine = NULL;
        ListGetT(String, &lfLines, 0, &lfLine);
        assert_str_eq(lfLine, "line1", "file_read_lines LF line0 != \"line1\"");
        ListGetT(String, &lfLines, 1, &lfLine);
        assert_str_eq(lfLine, "line2", "file_read_lines LF line1 != \"line2\"");
        ListGetT(String, &lfLines, 2, &lfLine);
        assert_str_eq(lfLine, "line3", "file_read_lines LF line2 != \"line3\"");

        list_free_items(&alloc, &lfLines);
        list_deinit(&lfLines);

        io_println("file_read_lines crlf");

        file_close(&file);
        ResultFile crlfFile = file_create(filePath);
        assert_ok(crlfFile.error, "file_read_lines failed to recreate file for CRLF");
        file = crlfFile.value;

        ConstStr crlfContent = "alpha\r\nbeta\r\ngamma\r\n";
        err = file_write_str(&file, crlfContent);
        assert_ok(err, "file_read_lines failed to write CRLF content");

        err = file_rewind(&file);
        assert_ok(err, "file_read_lines CRLF rewind err.code != ERR_OK");

        ResultList crlfLinesRes = file_read_lines(&alloc, &file);
        assert_ok(crlfLinesRes.error, "file_read_lines CRLF crlfLinesRes.error.code != ERR_OK");
        List crlfLines = crlfLinesRes.value;
        assert_true(list_size(&crlfLines) == 4, "file_read_lines CRLF count != 4");

        String crlfLine = NULL;
        ListGetT(String, &crlfLines, 0, &crlfLine);
        assert_str_eq(crlfLine, "alpha", "file_read_lines CRLF line0 != \"alpha\"");
        ListGetT(String, &crlfLines, 1, &crlfLine);
        assert_str_eq(crlfLine, "beta", "file_read_lines CRLF line1 != \"beta\"");
        ListGetT(String, &crlfLines, 2, &crlfLine);
        assert_str_eq(crlfLine, "gamma", "file_read_lines CRLF line2 != \"gamma\"");
        ListGetT(String, &crlfLines, 3, &crlfLine);
        assert_str_eq(crlfLine, "", "file_read_lines CRLF line3 != \"\"");

        list_free_items(&alloc, &crlfLines);
        list_deinit(&crlfLines);

        io_println("file_read_lines empty");

        file_close(&file);
        ResultFile emptyLinesFile = file_create(filePath);
        assert_ok(emptyLinesFile.error, "file_read_lines failed to recreate file for empty");
        file = emptyLinesFile.value;

        ResultList emptyLinesRes = file_read_lines(&alloc, &file);
        assert_ok(emptyLinesRes.error, "file_read_lines emptyLinesRes.error.code != ERR_OK");
        List emptyLines = emptyLinesRes.value;
        assert_true(list_size(&emptyLines) == 1, "file_read_lines empty count != 1");

        String emptyLine = NULL;
        ListGetT(String, &emptyLines, 0, &emptyLine);
        assert_str_eq(emptyLine, "", "file_read_lines empty line0 != \"\"");

        list_free_items(&alloc, &emptyLines);
        list_deinit(&emptyLines);

        io_println("file_rewind");

        err = file_rewind(&file);
        assert_ok(err, "file_rewind err.code != ERR_OK");

        io_println("file_write_str restore");

        err = file_write_str(&file, "hello_world!");
        assert_ok(err, "file_readall_str restore write err.code != ERR_OK");
        err = file_rewind(&file);
        assert_ok(err, "file_readall_str restore rewind err.code != ERR_OK");

        io_println("file_readall_str");

        ResultOwnedStr readAll = file_readall_str(&alloc, &file);
        assert_ok(readAll.error, "file_readall_str readAll.error.code != ERR_OK");

        io_print("content: ");
        io_println(readAll.value);

        assert_str_eq(readAll.value, "hello_world!", "file_readall_str content != \"hello_world!\"");
        alloc.free(&alloc, readAll.value);

        io_println("file_readall_str empty");

        file_close(&file);
        ResultFile emptyFileRes = file_create(filePath);
        assert_ok(emptyFileRes.error, "file_readall_str empty recreate err");
        file = emptyFileRes.value;
        err = file_rewind(&file);
        assert_ok(err, "file_readall_str empty rewind err.code != ERR_OK");

        ResultOwnedStr emptyRead = file_readall_str(&alloc, &file);
        assert_ok(emptyRead.error, "file_readall_str emptyRead.error.code != ERR_OK");
        assert_true(emptyRead.value && emptyRead.value[0] == 0, "file_readall_str emptyRead not empty string");
        alloc.free(&alloc, emptyRead.value);

        ResultOwnedBuff emptyBytes = file_readall_bytes(&alloc, &file);
        assert_ok(emptyBytes.error, "file_readall_bytes emptyBytes.error.code != ERR_OK");
        assert_true(emptyBytes.value.bytes == NULL && emptyBytes.value.size == 0, "file_readall_bytes emptyBytes not empty");

        io_println("file_write_str reseed");

        err = file_write_str(&file, "hello_world!");
        assert_ok(err, "file_readall_str reseed write err.code != ERR_OK");
        err = file_rewind(&file);
        assert_ok(err, "file_readall_str reseed rewind err.code != ERR_OK");

        io_println("file_rewind");

        err = file_rewind(&file);
        assert_ok(err, "file_rewind err.code != ERR_OK");

        io_println("file_read_bytes");

        ResultOwnedBuff readBytes = file_read_bytes(&alloc, &file, 5);
        assert_ok(readBytes.error, "file_read_bytes readBytes.error.code != ERR_OK");
        assert_true(readBytes.value.size == 5, "file_read_bytes size != 5");
        assert_true(readBytes.value.bytes[0] == 'h', "file_read_bytes first byte != 'h'");
        assert_true(readBytes.value.bytes[4] == 'o', "file_read_bytes fifth byte != 'o'");
        buffer_free(&alloc, &readBytes.value);

        io_println("file_rewind");

        err = file_rewind(&file);
        assert_ok(err, "file_rewind err.code != ERR_OK");

        io_println("file_read_str partial");

        ResultOwnedStr readPartial = file_read_str(&alloc, &file, 5);
        assert_ok(readPartial.error, "file_read_str readPartial.error.code != ERR_OK");
        assert_str_eq(readPartial.value, "hello", "file_read_str content != \"hello\"");
        alloc.free(&alloc, readPartial.value);

        io_println("file_seek");

        err = file_seek(&file, 6, 0);
        assert_ok(err, "file_seek err.code != ERR_OK");

        io_println("file_tell");

        ResultU64 tellRes = file_tell(&file);
        assert_ok(tellRes.error, "file_tell tellRes.error.code != ERR_OK");
        assert_true(tellRes.value == 6, "file_tell tellRes.value != 6");

        io_println("file_read_str tail");

        ResultOwnedStr readTail = file_read_str(&alloc, &file, 6);
        assert_ok(readTail.error, "file_read_str readTail.error.code != ERR_OK");
        assert_str_eq(readTail.value, "world!", "file_read_str readTail != \"world!\"");
        alloc.free(&alloc, readTail.value);

        io_println("file_rewind");

        err = file_rewind(&file);
        assert_ok(err, "file_rewind err.code != ERR_OK");

        io_println("file_read_bytes from null");

        ResultOwnedBuff nullAllocRead = file_read_bytes(NULL, &file, 1);
        assert_true(nullAllocRead.error.code == ERR_INVALID_PARAMETER, "file_read_bytes NULL alloc error.code != ERR_INVALID_PARAMETER");

        io_println("file_close");

        file_close(&file);

        io_println("file_read_bytes invalid");

        ResultOwnedBuff invalidFileRead = file_read_bytes(&alloc, &file, 1);
        assert_true(invalidFileRead.error.code == ERR_INVALID_PARAMETER, "file_read_bytes invalid file error.code != ERR_INVALID_PARAMETER");

        io_println("file_write_char invalid");

        Error invalidWrite = file_write_char(&file, 'X');
        assert_true(invalidWrite.code == ERR_INVALID_PARAMETER, "file_write_char invalid file code != ERR_INVALID_PARAMETER");

        io_println("file_exists");

        ibool exists = file_exists(filePath);
        assert_true(exists, "file_exists existing file == false");

        io_println("file_exists missing");

        ibool missing = file_exists("xstd_file_missing.tmp");
        assert_true(!missing, "file_exists missing file != false");

        io_println("file_open");

        ResultFile reopenRes = file_open(filePath, EnumFileOpenMode.READ);
        assert_ok(reopenRes.error, "file_open reopenRes.error.code != ERR_OK");
        File readFile = reopenRes.value;

        io_println("file_tell");

        ResultU64 tellStart = file_tell(&readFile);
        assert_ok(tellStart.error, "file_tell tellStart.error.code != ERR_OK");
        assert_true(tellStart.value == 0, "file_tell tellStart.value != 0");

        io_println("file_readall_str again");

        ResultOwnedStr readAllAgain = file_readall_str(&alloc, &readFile);
        assert_ok(readAllAgain.error, "file_readall_str readAllAgain.error.code != ERR_OK");
        assert_str_eq(readAllAgain.value, "hello_world!", "file_readall_str readAllAgain != \"hello_world!\"");
        alloc.free(&alloc, readAllAgain.value);

        io_println("file_close");

        file_close(&readFile);

        ResultFile openNullPath = file_open(NULL, EnumFileOpenMode.READ);
        assert_true(openNullPath.error.code == ERR_INVALID_PARAMETER, "file_open NULL path error.code != ERR_INVALID_PARAMETER");
    }
}

static void _xstd_string_tests(Allocator alloc)
{
    Error err;
    Allocator badAlloc = _xstd_bad_alloc();

    io_println("string_size");
    {
        ConstStr strLen22 = "This string is 22 long";
        assert_true(string_size(strLen22) == 22, "string_size 22 != 22");

        ConstStr strLen27 = "This string is 27 long 😔";
        assert_true(string_size(strLen27) == 27, "string_size 27 != 27");

        ConstStr strLen0 = "";
        assert_true(string_size(strLen0) == 0, "string_size 0 != 0");

        ConstStr strLenNull = NULL;
        assert_true(string_size(strLenNull) == 0, "string_size NULL != 0");
    }
    io_println("string_equals");
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
    io_println("string_alloc");
    {
        ResultOwnedStr strHeapRes1 = string_alloc(&alloc, 5, ' ');
        assert_true(strHeapRes1.error.code == ERR_OK, "string_alloc strHeapRes1.error.code != ERR_OK");

        OwnedStr strHeap1 = strHeapRes1.value;
        assert_true(strHeap1 != NULL, "string_alloc strHeapRes1.value == NULL");
        assert_true(string_size(strHeap1) == 5, "string_alloc size strHeapRes1.value != 5");
        assert_true(string_equals(strHeap1, "     "), "string_alloc strHeap1 != \"     \"");

        ResultOwnedStr strHeapRes2 = string_alloc(&alloc, 0, ' ');
        assert_true(strHeapRes2.error.code == ERR_OK, "string_alloc strHeapRes2.error.code != ERR_OK");

        OwnedStr strHeap2 = strHeapRes2.value;
        assert_true(strHeap2 != NULL, "string_alloc strHeapRes2.value == NULL");
        assert_true(string_size(strHeap2) == 0, "string_alloc size strHeapRes2.value != 0");
        assert_true(string_equals(strHeap2, ""), "string_alloc strHeap2 != \"\"");

        ResultOwnedStr strHeapRes3 = string_alloc(&badAlloc, 5, ' ');
        assert_true(strHeapRes3.error.code != ERR_OK, "string_alloc strHeapRes3.error.code == ERR_OK");

        alloc.free(&alloc, strHeap1);
        alloc.free(&alloc, strHeap2);
    }
    io_println("string_copy_unsafe");
    {
        ConstStr strCopyUn1 = "Copied.";

        ResultOwnedStr strCopyUnRes2 = string_alloc(&alloc, 7, ' ');
        assert_true(strCopyUnRes2.error.code == ERR_OK, "string_copy_unsafe strCopyUnRes2.error.code != ERR_OK");
        OwnedStr strCopyUn2 = strCopyUnRes2.value;
        string_copy_unsafe(strCopyUn1, strCopyUn2);
        assert_true(string_equals(strCopyUn1, strCopyUn2), "string_copy_unsafe strCopyUn1 != strCopyUn2");

        ResultOwnedStr strCopyUnRes3 = string_alloc(&alloc, 10, ' ');
        assert_true(strCopyUnRes3.error.code == ERR_OK, "string_copy_unsafe strCopyUnRes3.error.code != ERR_OK");
        OwnedStr strCopyUn3 = strCopyUnRes3.value;
        string_copy_unsafe(strCopyUn1, strCopyUn3);
        assert_true(string_equals(strCopyUn3, strCopyUn1), "string_copy_unsafe strCopyUn3 != strCopyUn1");

        alloc.free(&alloc, strCopyUn2);
        alloc.free(&alloc, strCopyUn3);
    }
    io_println("string_copy_n_unsafe");
    {
        ConstStr strCopyNUn1 = "Copied.";

        ResultOwnedStr strCopyNUnRes2 = string_alloc(&alloc, 7, ' ');
        assert_true(strCopyNUnRes2.error.code == ERR_OK, "string_copy_n_unsafe strCopyNUnRes2.error.code != ERR_OK");
        OwnedStr strCopyNUn2 = strCopyNUnRes2.value;
        string_copy_n_unsafe(strCopyNUn1, strCopyNUn2, 7, false);
        assert_true(string_equals(strCopyNUn1, strCopyNUn2), "string_copy_n_unsafe strCopyNUn1 != strCopyNUn2");

        ResultOwnedStr strCopyNUnRes3 = string_alloc(&alloc, 10, ' ');
        assert_true(strCopyNUnRes3.error.code == ERR_OK, "string_copy_n_unsafe strCopyNUnRes3.error.code != ERR_OK");
        OwnedStr strCopyNUn3 = strCopyNUnRes3.value;
        string_copy_n_unsafe(strCopyNUn1, strCopyNUn3, 7, false);
        assert_true(string_equals(strCopyNUn3, "Copied.   "), "string_copy_n_unsafe strCopyNUn3 != \"Copied.   \"");

        ResultOwnedStr strCopyNUnRes4 = string_alloc(&alloc, 10, ' ');
        assert_true(strCopyNUnRes4.error.code == ERR_OK, "string_copy_n_unsafe strCopyNUnRes4.error.code != ERR_OK");
        OwnedStr strCopyNUn4 = strCopyNUnRes4.value;
        string_copy_n_unsafe(strCopyNUn1, strCopyNUn4, 3, false);
        assert_true(string_equals(strCopyNUn4, "Cop       "), "string_copy_n_unsafe strCopyNUn4 != \"Cop       \"");

        ResultOwnedStr strCopyNUnRes5 = string_alloc(&alloc, 10, ' ');
        assert_true(strCopyNUnRes5.error.code == ERR_OK, "string_copy_n_unsafe strCopyNUnRes5.error.code != ERR_OK");
        OwnedStr strCopyNUn5 = strCopyNUnRes5.value;
        string_copy_n_unsafe(strCopyNUn1, strCopyNUn5, 0, false);
        assert_true(string_equals(strCopyNUn5, "          "), "string_copy_n_unsafe strCopyNUn5 != \"          \"");

        // TODO: Add testing for terminate==true

        alloc.free(&alloc, strCopyNUn2);
        alloc.free(&alloc, strCopyNUn3);
        alloc.free(&alloc, strCopyNUn4);
        alloc.free(&alloc, strCopyNUn5);
    }
    io_println("string_copy");
    {
        ConstStr strCopy1 = "Copied.";

        ResultOwnedStr strCopyRes2 = string_alloc(&alloc, 7, ' ');
        assert_true(strCopyRes2.error.code == ERR_OK, "string_copy strCopyRes2.error.code != ERR_OK");
        OwnedStr strCopy2 = strCopyRes2.value;
        err = string_copy(strCopy1, strCopy2);
        assert_true(err.code == ERR_OK, "string_copy strCopy2 err.code != ERR_OK");
        assert_true(string_equals(strCopy1, strCopy2), "string_copy strCopy1 != strCopy2");

        ResultOwnedStr strCopyRes3 = string_alloc(&alloc, 10, ' ');
        assert_true(strCopyRes3.error.code == ERR_OK, "string_copy strCopyRes3.error.code != ERR_OK");
        OwnedStr strCopy3 = strCopyRes3.value;
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
    io_println("string_copy_n");
    {
        ConstStr strCopyN1 = "Copied.";

        ResultOwnedStr strCopyNRes2 = string_alloc(&alloc, 7, ' ');
        assert_true(strCopyNRes2.error.code == ERR_OK, "string_copy_n strCopyNRes2.error.code != ERR_OK");
        OwnedStr strCopyN2 = strCopyNRes2.value;
        err = string_copy_n(strCopyN1, strCopyN2, 7, false);
        assert_true(err.code == ERR_OK, "string_copy_n strCopyN2 err.code != ERR_OK");
        assert_true(string_equals(strCopyN1, strCopyN2), "string_copy_n strCopyN1 != strCopyN2");

        ResultOwnedStr strCopyNRes3 = string_alloc(&alloc, 10, ' ');
        assert_true(strCopyNRes3.error.code == ERR_OK, "string_copy_n strCopyNRes3.error.code != ERR_OK");
        OwnedStr strCopyN3 = strCopyNRes3.value;
        err = string_copy_n(strCopyN1, strCopyN3, 7, false);
        assert_true(err.code == ERR_OK, "string_copy_n strCopyN3 err.code != ERR_OK");
        assert_true(string_equals(strCopyN3, "Copied.   "), "string_copy_n strCopyN3 != \"Copied.   \"");

        ResultOwnedStr strCopyNRes4 = string_alloc(&alloc, 10, ' ');
        assert_true(strCopyNRes4.error.code == ERR_OK, "string_copy_n strCopyNRes4.error.code != ERR_OK");
        OwnedStr strCopyN4 = strCopyNRes4.value;
        err = string_copy_n(strCopyN1, strCopyN4, 3, false);
        assert_true(err.code == ERR_OK, "string_copy_n strCopyN4 err.code != ERR_OK");
        assert_true(string_equals(strCopyN4, "Cop       "), "string_copy_n strCopyN4 != \"Cop       \"");

        ResultOwnedStr strCopyNRes5 = string_alloc(&alloc, 10, ' ');
        assert_true(strCopyNRes5.error.code == ERR_OK, "string_copy_n strCopyNRes5.error.code != ERR_OK");
        OwnedStr strCopyN5 = strCopyNRes5.value;
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
    io_println("string_dupe");
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
    io_println("string_dupe_noresult");
    {
        ConstStr strDupeNr0 = "";
        ConstStr strDupeNr1 = "Copied.";

        OwnedStr strDupeNr2 = string_dupe_noresult(&alloc, strDupeNr1);
        assert_true(strDupeNr2 != NULL, "string_dupe_noresult strDupeNr2 != NULL");
        assert_true(string_equals(strDupeNr1, strDupeNr2), "string_dupe_noresult strDupeNr1 != strDupeNr2");

        OwnedStr strDupeNr3 = string_dupe_noresult(&alloc, strDupeNr0);
        assert_true(strDupeNr3 != NULL, "string_dupe_noresult strDupeNr3 != NULL");
        assert_true(string_equals(strDupeNr0, strDupeNr3), "string_dupe_noresult strDupeNr0 != strDupeNr3");

        OwnedStr strDupeNr4 = string_dupe_noresult(&alloc, NULL);
        assert_true(strDupeNr4 == NULL, "string_dupe_noresult strDupeNr4 != NULL");

        OwnedStr strDupeNr5 = string_dupe_noresult(&badAlloc, strDupeNr1);
        assert_true(strDupeNr5 == NULL, "string_dupe_noresult strDupeNr5 != NULL");

        alloc.free(&alloc, strDupeNr2);
        alloc.free(&alloc, strDupeNr3);
    }
    io_println("string_resize");
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
    io_println("string_concat");
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
    io_println("string_substr");
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
    io_println("string_substr_unsafe");
    {
        ConstStr strSub1 = "This is a substring";

        OwnedStr strSub2 = string_substr_unsafe(&alloc, strSub1, 10, 19);

        assert_true(strSub2 != NULL, "string_substr_unsafe strSub2 == NULL");
        assert_true(string_equals(strSub2, "substring"), "string_substr_unsafe strSub2 != \"substring\"");

        OwnedStr strSub3 = string_substr_unsafe(&alloc, strSub1, 0, 4);
        assert_true(strSub3 != NULL, "string_substr_unsafe strSub3 == NULL");
        assert_true(string_equals(strSub3, "This"), "string_substr_unsafe strSub3 != \"This\"");

        OwnedStr strSub6 = string_substr_unsafe(&badAlloc, strSub1, 10, 19);
        assert_true(strSub6 == NULL, "string_substr_unsafe strSub6 != NULL");

        alloc.free(&alloc, strSub2);
        alloc.free(&alloc, strSub3);
    }
    io_println("string_splitc");
    {
        ConstStr strSpl0 = " This is a split string ";
        ConstStr strSpl1 = "This is a  split string";

        ResultList strSpl2 = string_split_char(&alloc, strSpl1, ' ');
        assert_ok(strSpl2.error, "string_splitc strSpl2.error.code != ERR_OK");

        List l = strSpl2.value;

        u64 listSize = list_size(&l);
        assert_true(listSize == 6, "string_splitc size strSpl2 != 6");

        String strSpl3 = NULL;
        ListGetT(String, &l, 0, &strSpl3);
        assert_true(!!strSpl3, "string_splitc strSpl3 == NULL");
        assert_str_eq(strSpl3, "This", "string_splitc strSpl3 != \"This\"");

        String strSpl3_2 = NULL;
        ListGetT(String, &l, 3, &strSpl3_2);
        assert_true(!!strSpl3_2, "string_splitc strSpl3_2 == NULL");
        assert_str_eq(strSpl3_2, "", "string_splitc strSpl3_2 != \"\"");

        String strSpl3_5 = NULL;
        ListGetT(String, &l, 4, &strSpl3_5);
        assert_true(!!strSpl3_5, "string_splitc strSpl3_5 == NULL");
        assert_str_eq(strSpl3_5, "split", "string_splitc strSpl3_5 != \"split\"");

        String strSpl4 = NULL;
        ListGetT(String, &l, 5, &strSpl4);
        assert_true(!!strSpl4, "string_splitc strSpl4 == NULL");
        assert_str_eq(strSpl4, "string", "string_splitc strSpl4 != \"string\"");

        ResultList strSpl5 = string_split_char(&alloc, NULL, ' ');
        assert_true(strSpl5.error.code != ERR_OK, "string_splitc strSpl5.error.code == ERR_OK");

        ResultList strSpl6 = string_split_char(&alloc, strSpl0, ' ');
        assert_true(strSpl6.error.code == ERR_OK, "string_splitc strSpl6.error.code != ERR_OK");

        List l2 = strSpl6.value;

        u64 listSize2 = list_size(&l2);
        assert_true(listSize2 == 7, "string_splitc size strSpl7 != 5");

        String strSpl7 = NULL;
        ListGetT(String, &l2, 0, &strSpl7);
        assert_true(!!strSpl7, "string_splitc strSpl7 == NULL");
        assert_str_eq(strSpl7, "", "string_splitc strSpl7 != \"\"");

        String strSpl7_5 = NULL;
        ListGetT(String, &l2, 1, &strSpl7_5);
        assert_true(!!strSpl7_5, "string_splitc strSpl7_5 == NULL");
        assert_str_eq(strSpl7_5, "This", "string_splitc strSpl7_5 != \"This\"");

        String strSpl8 = NULL;
        ListGetT(String, &l2, 5, &strSpl8);
        assert_true(!!strSpl8, "string_splitc strSpl8 == NULL");
        assert_str_eq(strSpl8, "string", "string_splitc strSpl8 != \"string\"");

        list_free_items(&alloc, &l);
        list_deinit(&l);

        list_free_items(&alloc, &l2);
        list_deinit(&l2);
    }
    io_println("string_find");
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
    io_println("string_find_char");
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
    io_println("StringBuilder");
    {
        ResultStrBuilder resBld = strbuilder_init(&alloc);
        assert_ok(resBld.error, "StringBuilder resBld.error.code != ERR_OK");

        ResultStrBuilder resBld2 = strbuilder_init(&badAlloc);
        assert_true(resBld2.error.code != ERR_OK, "StringBuilder resBld2.error.code == ERR_OK");

        StringBuilder builder = resBld.value;

        strbuilder_push_copy(&builder, "This");

        ResultOwnedStr resStr = strbuilder_get_string(&builder);
        assert_ok(resStr.error, "StringBuilder resStr.error.code != ERR_OK");

        OwnedStr built = resStr.value;
        assert_str_eq(built, "This", "StringBuilder built != \"This\"");

        alloc.free(&alloc, built);

        strbuilder_push_owned(&builder, ConstToHeapStr(&alloc, " is a"));
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
    io_println("string_replace");
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
    io_println("string_starts_with");
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
    io_println("string_ends_with");
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
    io_println("char_is_alpha");
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
    io_println("char_is_digit");
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
    io_println("char_is_alphanum");
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
    io_println("string_trim_whitespace");
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
    io_println("string_char_at");
    {
        ConstStr utf8Sample = "naïve ☕";

        ResultUtf8Codepoint cp0 = string_char_at(utf8Sample, 0);
        assert_ok(cp0.error, "string_char_at cp0.error.code != ERR_OK");
        assert_true(cp0.value.codepoint == 'n', "string_char_at cp0 != 'n'");

        ResultUtf8Codepoint cp2 = string_char_at(utf8Sample, 2);
        assert_ok(cp2.error, "string_char_at cp2.error.code != ERR_OK");
        assert_true(cp2.value.codepoint == 0x00EF, "string_char_at cp2 != U+00EF");

        ResultUtf8Codepoint cp6 = string_char_at(utf8Sample, 6);
        assert_ok(cp6.error, "string_char_at cp6.error.code != ERR_OK");
        assert_true(cp6.value.codepoint == 0x2615, "string_char_at cp6 != U+2615");

        ResultUtf8Codepoint cpFail = string_char_at(utf8Sample, 99);
        assert_true(cpFail.error.code != ERR_OK, "string_char_at cpFail == ERR_OK");

        ResultUtf8Codepoint cpNull = string_char_at(NULL, 0);
        assert_true(cpNull.error.code != ERR_OK, "string_char_at cpNull == ERR_OK");
    }
    io_println("string_char_at_ascii");
    {
        ConstStr asciiInput = "example";
        u64 asciiLen = string_size(asciiInput);

        ResultByte asciiRes = string_char_at_ascii(asciiInput, 3, asciiLen);
        assert_ok(asciiRes.error, "string_char_at_ascii asciiRes.error.code != ERR_OK");
        assert_true(asciiRes.value == 'm', "string_char_at_ascii asciiRes != 'm'");

        ResultByte asciiOob = string_char_at_ascii(asciiInput, asciiLen, asciiLen);
        assert_true(asciiOob.error.code != ERR_OK, "string_char_at_ascii asciiOob == ERR_OK");

        ResultByte asciiNull = string_char_at_ascii(NULL, 0, 0);
        assert_true(asciiNull.error.code != ERR_OK, "string_char_at_ascii asciiNull == ERR_OK");
    }
    io_println("string_substr_ascii");
    {
        ConstStr asciiSrc = "Ascii substring sample";

        ResultOwnedStr asciiSub = string_substr_ascii(&alloc, asciiSrc, 6, 15);
        assert_ok(asciiSub.error, "string_substr_ascii asciiSub.error.code != ERR_OK");
        assert_str_eq(asciiSub.value, "substring", "string_substr_ascii asciiSub != \"substring\"");

        ResultOwnedStr asciiBadRange = string_substr_ascii(&alloc, asciiSrc, 10, 5);
        assert_true(asciiBadRange.error.code != ERR_OK, "string_substr_ascii asciiBadRange == ERR_OK");

        ResultOwnedStr asciiTooLarge = string_substr_ascii(&alloc, asciiSrc, 0, 100);
        assert_true(asciiTooLarge.error.code != ERR_OK, "string_substr_ascii asciiTooLarge == ERR_OK");

        alloc.free(&alloc, asciiSub.value);
    }
    io_println("string_substr_ascii_unsafe");
    {
        ConstStr unsafeSrc = "Unsafe ascii segment";

        OwnedStr unsafeSub = string_substr_ascii_unsafe(&alloc, unsafeSrc, 7, 12);
        assert_true(unsafeSub != NULL, "string_substr_ascii_unsafe unsafeSub == NULL");
        assert_str_eq(unsafeSub, "ascii", "string_substr_ascii_unsafe unsafeSub != \"ascii\"");
        alloc.free(&alloc, unsafeSub);

        OwnedStr unsafeFail = string_substr_ascii_unsafe(&badAlloc, unsafeSrc, 0, 3);
        assert_true(unsafeFail == NULL, "string_substr_ascii_unsafe unsafeFail != NULL");
    }
    io_println("string_split_char_ascii");
    {
        ConstStr splitAscii = "one,,two,three";

        ResultList listRes = string_split_char_ascii(&alloc, splitAscii, ',');
        assert_ok(listRes.error, "string_split_char_ascii listRes.error.code != ERR_OK");

        List asciiList = listRes.value;
        u64 asciiCount = list_size(&asciiList);
        assert_true(asciiCount == 4, "string_split_char_ascii asciiCount != 4");

        String token = NULL;
        ListGetT(String, &asciiList, 0, &token);
        assert_true(!!token, "string_split_char_ascii token0 == NULL");
        assert_str_eq(token, "one", "string_split_char_ascii token0 != \"one\"");

        ListGetT(String, &asciiList, 1, &token);
        assert_true(!!token, "string_split_char_ascii token1 == NULL");
        assert_str_eq(token, "", "string_split_char_ascii token1 != \"\"");

        ListGetT(String, &asciiList, 2, &token);
        assert_str_eq(token, "two", "string_split_char_ascii token2 != \"two\"");

        ListGetT(String, &asciiList, 3, &token);
        assert_str_eq(token, "three", "string_split_char_ascii token3 != \"three\"");

        list_free_items(&alloc, &asciiList);
        list_deinit(&asciiList);

        ResultList listErr = string_split_char_ascii(&alloc, NULL, ',');
        assert_true(listErr.error.code != ERR_OK, "string_split_char_ascii listErr == ERR_OK");
    }
    io_println("string_split_lines");
    {
        ConstStr linesUtf8 = "line1\nlínea2\r\nline3 ☕\n";

        ResultList linesRes = string_split_lines(&alloc, linesUtf8);
        assert_ok(linesRes.error, "string_split_lines linesRes.error.code != ERR_OK");

        List utf8Lines = linesRes.value;
        u64 utf8Count = list_size(&utf8Lines);
        assert_true(utf8Count == 4, "string_split_lines utf8Count != 4");

        String line = NULL;
        ListGetT(String, &utf8Lines, 0, &line);
        assert_str_eq(line, "line1", "string_split_lines line0 != \"line1\"");

        ListGetT(String, &utf8Lines, 1, &line);
        assert_str_eq(line, "línea2", "string_split_lines line1 != \"línea2\"");

        ListGetT(String, &utf8Lines, 2, &line);
        assert_str_eq(line, "line3 ☕", "string_split_lines line2 != \"line3 ☕\"");

        ListGetT(String, &utf8Lines, 3, &line);
        assert_str_eq(line, "", "string_split_lines line3 != \"\"");

        list_free_items(&alloc, &utf8Lines);
        list_deinit(&utf8Lines);

        ResultList linesErr = string_split_lines(&alloc, NULL);
        assert_true(linesErr.error.code != ERR_OK, "string_split_lines linesErr == ERR_OK");
    }
    io_println("string_split_lines_ascii");
    {
        ConstStr linesAscii = "first\r\nsecond\nthird";

        ResultList asciiLinesRes = string_split_lines_ascii(&alloc, linesAscii);
        assert_ok(asciiLinesRes.error, "string_split_lines_ascii asciiLinesRes.error.code != ERR_OK");

        List asciiLines = asciiLinesRes.value;
        u64 lineCount = list_size(&asciiLines);
        assert_true(lineCount == 3, "string_split_lines_ascii lineCount != 3");

        String asciiLine = NULL;
        ListGetT(String, &asciiLines, 0, &asciiLine);
        assert_str_eq(asciiLine, "first", "string_split_lines_ascii asciiLine0 != \"first\"");

        ListGetT(String, &asciiLines, 1, &asciiLine);
        assert_str_eq(asciiLine, "second", "string_split_lines_ascii asciiLine1 != \"second\"");

        ListGetT(String, &asciiLines, 2, &asciiLine);
        assert_str_eq(asciiLine, "third", "string_split_lines_ascii asciiLine2 != \"third\"");

        list_free_items(&alloc, &asciiLines);
        list_deinit(&asciiLines);

        ResultList asciiLinesErr = string_split_lines_ascii(&alloc, NULL);
        assert_true(asciiLinesErr.error.code != ERR_OK, "string_split_lines_ascii asciiLinesErr == ERR_OK");
    }
    io_println("string_lower");
    {
        ResultOwnedStr lowerRes = string_lower(&alloc, "MiXeD Case CAFÉ");
        assert_ok(lowerRes.error, "string_lower lowerRes.error.code != ERR_OK");
        assert_str_eq(lowerRes.value, "mixed case cafÉ", "string_lower lowerRes != \"mixed case cafÉ\"");
        alloc.free(&alloc, lowerRes.value);
    }
    io_println("string_upper");
    {
        ResultOwnedStr upperRes = string_upper(&alloc, "mixed case café");
        assert_ok(upperRes.error, "string_upper upperRes.error.code != ERR_OK");
        assert_str_eq(upperRes.value, "MIXED CASE CAFé", "string_upper upperRes != \"MIXED CASE CAFé\"");
        alloc.free(&alloc, upperRes.value);
    }
    io_println("string_lower_ascii");
    {
        ResultOwnedStr lowerAscii = string_lower_ascii(&alloc, "HELLO ASCII");
        assert_ok(lowerAscii.error, "string_lower_ascii lowerAscii.error.code != ERR_OK");
        assert_str_eq(lowerAscii.value, "hello ascii", "string_lower_ascii lowerAscii != \"hello ascii\"");
        alloc.free(&alloc, lowerAscii.value);
    }
    io_println("string_upper_ascii");
    {
        ResultOwnedStr upperAscii = string_upper_ascii(&alloc, "hello ascii");
        assert_ok(upperAscii.error, "string_upper_ascii upperAscii.error.code != ERR_OK");
        assert_str_eq(upperAscii.value, "HELLO ASCII", "string_upper_ascii upperAscii != \"HELLO ASCII\"");
        alloc.free(&alloc, upperAscii.value);
    }
    io_println("string_to_lower_inplace");
    {
        HeapStr lowerBuff = ConstToHeapStr(&alloc, "ModIfY Me É");
        assert_true(lowerBuff != NULL, "string_to_lower_inplace lowerBuff == NULL");
        string_to_lower_inplace(lowerBuff);
        assert_str_eq(lowerBuff, "modify me É", "string_to_lower_inplace lowerBuff != \"modify me É\"");
        alloc.free(&alloc, lowerBuff);
    }
    io_println("string_to_upper_inplace");
    {
        HeapStr upperBuff = ConstToHeapStr(&alloc, "modify me é");
        assert_true(upperBuff != NULL, "string_to_upper_inplace upperBuff == NULL");
        string_to_upper_inplace(upperBuff);
        assert_str_eq(upperBuff, "MODIFY ME é", "string_to_upper_inplace upperBuff != \"MODIFY ME é\"");
        alloc.free(&alloc, upperBuff);
    }
    io_println("string_from_int");
    {
        ResultOwnedStr intStr = string_from_int(&alloc, -12345);
        assert_ok(intStr.error, "string_from_int intStr.error.code != ERR_OK");
        assert_str_eq(intStr.value, "-12345", "string_from_int intStr != \"-12345\"");
        alloc.free(&alloc, intStr.value);

        ResultOwnedStr zeroStr = string_from_int(&alloc, 0);
        assert_ok(zeroStr.error, "string_from_int zeroStr.error.code != ERR_OK");
        assert_str_eq(zeroStr.value, "0", "string_from_int zeroStr != \"0\"");
        alloc.free(&alloc, zeroStr.value);
    }
    io_println("string_from_uint");
    {
        ResultOwnedStr uintStr = string_from_uint(&alloc, 9876543210ULL);
        assert_ok(uintStr.error, "string_from_uint uintStr.error.code != ERR_OK");
        assert_str_eq(uintStr.value, "9876543210", "string_from_uint uintStr != \"9876543210\"");
        alloc.free(&alloc, uintStr.value);
    }
    io_println("string_from_float");
    {
        ResultOwnedStr floatStr = string_from_float(&alloc, -12.5, 1);
        assert_ok(floatStr.error, "string_from_float floatStr.error.code != ERR_OK");
        assert_str_eq(floatStr.value, "-12.5", "string_from_float floatStr != \"-12.5\"");
        alloc.free(&alloc, floatStr.value);

        ResultOwnedStr floatStr2 = string_from_float(&alloc, 3.125, 3);
        assert_ok(floatStr2.error, "string_from_float floatStr2.error.code != ERR_OK");
        assert_str_eq(floatStr2.value, "3.125", "string_from_float floatStr2 != \"3.125\"");
        alloc.free(&alloc, floatStr2.value);

        ResultOwnedStr bigFloat = string_from_float(&alloc, 123456789012345.75, 2);
        assert_ok(bigFloat.error, "string_from_float bigFloat.error.code != ERR_OK");
        assert_str_eq(bigFloat.value, "123456789012345.75", "string_from_float bigFloat != \"123456789012345.75\"");
        alloc.free(&alloc, bigFloat.value);

        ResultOwnedStr bigNegative = string_from_float(&alloc, -98765432109876.03125, 5);
        assert_ok(bigNegative.error, "string_from_float bigNegative.error.code != ERR_OK");
        assert_str_eq(bigNegative.value, "-98765432109876.03125", "string_from_float bigNegative != \"-98765432109876.03125\"");
        alloc.free(&alloc, bigNegative.value);

        ResultOwnedStr roundingCarry = string_from_float(&alloc, 99999999999.9996, 3);
        assert_ok(roundingCarry.error, "string_from_float roundingCarry.error.code != ERR_OK");
        assert_str_eq(roundingCarry.value, "100000000000.000", "string_from_float roundingCarry != \"100000000000.000\"");
        alloc.free(&alloc, roundingCarry.value);
    }
    io_println("string_parse_int_ascii");
    {
        ResultI64 parseInt = string_parse_int_ascii("  -42  ");
        assert_ok(parseInt.error, "string_parse_int_ascii parseInt.error.code != ERR_OK");
        assert_true(parseInt.value == -42, "string_parse_int_ascii parseInt != -42");

        ResultI64 parseErr = string_parse_int_ascii("abc");
        assert_true(parseErr.error.code != ERR_OK, "string_parse_int_ascii parseErr == ERR_OK");
    }
    io_println("string_parse_int");
    {
        ConstStr utf8Int = " +256 ";
        ResultI64 parseUtf8 = string_parse_int(utf8Int);
        assert_ok(parseUtf8.error, "string_parse_int parseUtf8.error.code != ERR_OK");
        assert_true(parseUtf8.value == 256, "string_parse_int parseUtf8 != 256");

        ResultI64 parseUtf8Err = string_parse_int("12a");
        assert_true(parseUtf8Err.error.code != ERR_OK, "string_parse_int parseUtf8Err == ERR_OK");
    }
    io_println("string_parse_uint_ascii");
    {
        ResultU64 parseUInt = string_parse_uint_ascii("  4096 ");
        assert_ok(parseUInt.error, "string_parse_uint_ascii parseUInt.error.code != ERR_OK");
        assert_true(parseUInt.value == 4096, "string_parse_uint_ascii parseUInt != 4096");

        ResultU64 parseUIntErr = string_parse_uint_ascii("-1");
        assert_true(parseUIntErr.error.code != ERR_OK, "string_parse_uint_ascii parseUIntErr == ERR_OK");
    }
    io_println("string_parse_uint");
    {
        ConstStr utf8UInt = " 1024 ";
        ResultU64 parseUtf8UInt = string_parse_uint(utf8UInt);
        assert_ok(parseUtf8UInt.error, "string_parse_uint parseUtf8UInt.error.code != ERR_OK");
        assert_true(parseUtf8UInt.value == 1024, "string_parse_uint parseUtf8UInt != 1024");

        ResultU64 parseUtf8UIntErr = string_parse_uint("++1");
        assert_true(parseUtf8UIntErr.error.code != ERR_OK, "string_parse_uint parseUtf8UIntErr == ERR_OK");
    }
    io_println("string_parse_float_ascii");
    {
        ResultF64 parseFloat = string_parse_float_ascii(" +3.25 ");
        assert_ok(parseFloat.error, "string_parse_float_ascii parseFloat.error.code != ERR_OK");
        assert_true(parseFloat.value == 3.25, "string_parse_float_ascii parseFloat != 3.25");

        ResultF64 parseFloatErr = string_parse_float_ascii("3.");
        assert_true(parseFloatErr.error.code != ERR_OK, "string_parse_float_ascii parseFloatErr == ERR_OK");
    }
    io_println("string_parse_float");
    {
        ConstStr utf8Float = " -12.5 ";
        ResultF64 parseUtf8Float = string_parse_float(utf8Float);
        assert_ok(parseUtf8Float.error, "string_parse_float parseUtf8Float.error.code != ERR_OK");
        assert_true(parseUtf8Float.value == -12.5, "string_parse_float parseUtf8Float != -12.5");

        ResultF64 parseUtf8FloatErr = string_parse_float("nan");
        assert_true(parseUtf8FloatErr.error.code != ERR_OK, "string_parse_float parseUtf8FloatErr == ERR_OK");
    }
    io_println("string_trim_whitespace_ascii");
    {
        ResultOwnedStr trimAscii = string_trim_whitespace_ascii(&alloc, "  padded ascii  ", true, true);
        assert_ok(trimAscii.error, "string_trim_whitespace_ascii trimAscii.error.code != ERR_OK");
        assert_str_eq(trimAscii.value, "padded ascii", "string_trim_whitespace_ascii trimAscii != \"padded ascii\"");
        alloc.free(&alloc, trimAscii.value);

        ResultOwnedStr trimStartOnly = string_trim_whitespace_ascii(&alloc, "\t spaced", true, false);
        assert_ok(trimStartOnly.error, "string_trim_whitespace_ascii trimStartOnly.error.code != ERR_OK");
        assert_str_eq(trimStartOnly.value, "spaced", "string_trim_whitespace_ascii trimStartOnly != \"spaced\"");
        alloc.free(&alloc, trimStartOnly.value);

        ResultOwnedStr trimNull = string_trim_whitespace_ascii(&alloc, NULL, true, true);
        assert_true(trimNull.error.code != ERR_OK, "string_trim_whitespace_ascii trimNull == ERR_OK");
    }
}

static void _xstd_writer_tests(Allocator alloc)
{
    Allocator badAlloc = _xstd_bad_alloc();

    io_println("buffwriter_init");
    {
        char storage[8] = {0};
        Buffer buff = (Buffer){.bytes = (i8*)storage, .size = sizeof(storage)};

        ResultWriter res = buffwriter_init(&alloc, &buff);
        assert_ok(res.error, "buffwriter_init res.error.code != ERR_OK");

        Writer writer = res.value;
        Error err = writer_write_str(&writer, "abc");
        assert_ok(err, "buffwriter_init writer_write_str != ERR_OK");
        storage[3] = 0;
        assert_str_eq(storage, "abc", "buffwriter_init storage != \"abc\"");

        for (i32 i = 3; i < 8; ++i)
        {
            err = writer_write_byte(&writer, 'x');
            assert_ok(err, "buffwriter_init fill err.code != ERR_OK");
        }

        Error overflowErr = writer_write_byte(&writer, 'y');
        assert_true(overflowErr.code == ERR_WOULD_OVERFLOW, "buffwriter_init overflowErr.code != ERR_WOULD_OVERFLOW");

        buff = (Buffer){.bytes = NULL, .size = 4};
        ResultWriter errRes = buffwriter_init(&alloc, &buff);
        assert_true(errRes.error.code != ERR_OK, "buffwriter_init errRes.error.code == ERR_OK");

        buffwriter_deinit(&writer);
    }

    io_println("growbuffwriter_init");
    {
        ResultWriter res = growbuffwriter_init(alloc, 4);
        assert_ok(res.error, "growbuffwriter_init res.error.code != ERR_OK");

        Writer writer = res.value;
        const char *text = "abcdef";
        for (const char *cursor = text; *cursor; ++cursor)
        {
            Error err = writer_write_byte(&writer, *cursor);
            assert_ok(err, "growbuffwriter_init writer_write_byte != ERR_OK");
        }

        ResultOwnedBuff resData = growbuffwriter_data_copy(&writer);
        assert_ok(resData.error, "growbuffwriter_init growbuffwriter_buffer != ERR_OK");
        assert_true(resData.value.size >= 6, "growbuffwriter_init buff.size < 6");

        resData = growbuffwriter_data(&writer);
        assert_ok(resData.error, "growbuffwriter_init growbuffwriter_data != ERR_OK");

        HeapBuff dataBuff = resData.value;
        assert_true(dataBuff.size == 6, "growbuffwriter_init used.size != 6");
        assert_true(dataBuff.bytes[0] == 'a', "growbuffwriter_init first byte != 'a'");
        assert_true(dataBuff.bytes[5] == 'f', "growbuffwriter_init last byte != 'f'");

        Error resetErr = growbuffwriter_reset(&writer, 16);
        assert_ok(resetErr, "growbuffwriter_init growbuffwriter_reset != ERR_OK");

        Error strErr = writer_write_str(&writer, "grow");
        assert_ok(strErr, "growbuffwriter_init writer_write_str != ERR_OK");

        resData = growbuffwriter_data(&writer);
        assert_ok(resData.error, "growbuffwriter_init data after reset != ERR_OK");

        dataBuff = resData.value;
        assert_true(dataBuff.size == 4, "growbuffwriter_init used.size after reset != 4");
        assert_true(dataBuff.bytes[0] == 'g' && dataBuff.bytes[3] == 'w', "growbuffwriter_init writer_write_str output != \"grow\"");

        growbuffwriter_deinit(&writer);

        ResultWriter resErr = growbuffwriter_init(badAlloc, 8);
        assert_true(resErr.error.code != ERR_OK, "growbuffwriter_init resErr.error.code == ERR_OK");
    }

    io_println("growstrwriter_init");
    {
        ResultWriter res = growstrwriter_init(alloc, 4);
        assert_ok(res.error, "growstrwriter_init res.error.code != ERR_OK");

        Writer writer = res.value;
        const char *text = "hello";
        for (const char *cursor = text; *cursor; ++cursor)
        {
            Error err = writer_write_byte(&writer, *cursor);
            assert_ok(err, "growstrwriter_init writer_write_byte != ERR_OK");
        }

        ResultOwnedStr resData = growstrwriter_data(&writer);
        assert_ok(resData.error, "growstrwriter_init growstrwriter_data != ERR_OK");
        assert_str_eq(resData.value, "hello", "growstrwriter_init str != \"hello\"");

        Error resetErr = growstrwriter_reset(&writer, 16);
        assert_ok(resetErr, "growstrwriter_init growstrwriter_reset != ERR_OK");

        Error strErr = writer_write_str(&writer, "str");
        assert_ok(strErr, "growstrwriter_init writer_write_str != ERR_OK");

        resData  = growstrwriter_data(&writer);
        assert_ok(resData.error, "growstrwriter_init data after reset != ERR_OK");
        assert_str_eq(resData.value, "str", "growstrwriter_init writer_write_str output != \"str\"");

        growstrwriter_deinit(&writer);

        ResultWriter resErr = growstrwriter_init(badAlloc, 4);
        assert_true(resErr.error.code != ERR_OK, "growstrwriter_init resErr.error.code == ERR_OK");
    }

    io_println("writer_write_bytes");
    {
        char storage[6] = {0};
        Buffer buff = (Buffer){.bytes = (i8*)storage, .size = sizeof(storage)};

        ResultWriter res = buffwriter_init(&alloc, &buff);
        assert_ok(res.error, "writer_write_bytes res.error.code != ERR_OK");

        Writer writer = res.value;
        ConstBuff data = {
            .bytes = (i8*)"ABCD",
            .size = 4,
        };
        Error err = writer_write_bytes(&writer, data);
        assert_ok(err, "writer_write_bytes err.code != ERR_OK");

        storage[4] = 0;
        assert_str_eq(storage, "ABCD", "writer_write_bytes storage != \"ABCD\"");

        buffwriter_deinit(&writer);
    }

    io_println("writer_write_str");
    {
        char storage[12] = {0};
        Buffer buff = (Buffer){.bytes = (i8*)storage, .size = sizeof(storage)};
        
        ResultWriter res = buffwriter_init(&alloc, &buff);
        assert_ok(res.error, "writer_write_str res.error.code != ERR_OK");

        Writer writer = res.value;
        Error err = writer_write_str(&writer, "text");
        assert_ok(err, "writer_write_str err.code != ERR_OK");

        storage[4] = 0;
        assert_str_eq(storage, "text", "writer_write_str storage != \"text\"");

        Error resetErr = buffwriter_reset(&writer);
        assert_ok(resetErr, "writer_write_str resetErr.code != ERR_OK");
        storage[0] = 0;

        Error nullErr = writer_write_str(&writer, NULL);
        assert_ok(nullErr, "writer_write_str nullErr.code != ERR_OK");

        storage[6] = 0;
        assert_str_eq(storage, "(null)", "writer_write_str null output != \"(null)\"");

        buffwriter_deinit(&writer);
    }

    io_println("writer_write_int");
    {
        ResultWriter res = growstrwriter_init(alloc, 8);
        assert_ok(res.error, "writer_write_int res.error.code != ERR_OK");

        Writer writer = res.value;
        Error err = writer_write_int(&writer, -12345);
        assert_ok(err, "writer_write_int err.code != ERR_OK");

        ResultOwnedStr resData = growstrwriter_data(&writer);
        assert_ok(resData.error, "writer_write_int growstrwriter_data != ERR_OK");
        assert_str_eq(resData.value, "-12345", "writer_write_int str != \"-12345\"");

        growstrwriter_deinit(&writer);
    }

    io_println("writer_write_uint");
    {
        ResultWriter res = growstrwriter_init(alloc, 8);
        assert_ok(res.error, "writer_write_uint res.error.code != ERR_OK");

        Writer writer = res.value;
        Error err = writer_write_uint(&writer, 0);
        assert_ok(err, "writer_write_uint zero err.code != ERR_OK");

        ResultOwnedStr resData = growstrwriter_data(&writer);
        assert_ok(resData.error, "writer_write_uint growstrwriter_data != ERR_OK");
        assert_str_eq(resData.value, "0", "writer_write_uint zero != \"0\"");

        Error resetErr = growstrwriter_reset(&writer, 16);
        assert_ok(resetErr, "writer_write_uint resetErr.code != ERR_OK");

        err = writer_write_uint(&writer, 9876543210ULL);
        assert_ok(err, "writer_write_uint err.code != ERR_OK");

        resData = growstrwriter_data(&writer);
        assert_ok(resData.error, "writer_write_uint growstrwriter_data second != ERR_OK");
        assert_str_eq(resData.value, "9876543210", "writer_write_uint str != \"9876543210\"");

        growstrwriter_deinit(&writer);
    }

    io_println("writer_write_float");
    {
        ResultWriter res1 = growstrwriter_init(alloc, 16);
        assert_ok(res1.error, "writer_write_float res1.error.code != ERR_OK");
        Writer writer1 = res1.value;
        Error err = writer_write_float(&writer1, -12.5, 1);
        assert_ok(err, "writer_write_float err.code != ERR_OK");
        
        ResultOwnedStr resData = growstrwriter_data(&writer1);
        assert_ok(resData.error, "writer_write_float growstrwriter_data res1 != ERR_OK");
        assert_str_eq(resData.value, "-12.5", "writer_write_float str != \"-12.5\"");
        growstrwriter_deinit(&writer1);

        ResultWriter res2 = growstrwriter_init(alloc, 16);
        assert_ok(res2.error, "writer_write_float res2.error.code != ERR_OK");
        Writer writer2 = res2.value;
        err = writer_write_float(&writer2, 0.005, 2);
        assert_ok(err, "writer_write_float small err.code != ERR_OK");

        resData = growstrwriter_data(&writer2);
        assert_ok(resData.error, "writer_write_float growstrwriter_data res2 != ERR_OK");
        assert_str_eq(resData.value, "0.01", "writer_write_float str != \"0.01\"");
        growstrwriter_deinit(&writer2);

        ResultWriter res3 = growstrwriter_init(alloc, 16);
        assert_ok(res3.error, "writer_write_float res3.error.code != ERR_OK");
        Writer writer3 = res3.value;
        err = writer_write_float(&writer3, 9.9996, 3);
        assert_ok(err, "writer_write_float rounding err.code != ERR_OK");

        resData = growstrwriter_data(&writer3);
        assert_ok(resData.error, "writer_write_float growstrwriter_data res3 != ERR_OK");
        assert_str_eq(resData.value, "10.000", "writer_write_float str != \"10.000\"");
        growstrwriter_deinit(&writer3);
    }
}

static void _xstd_utf8_tests(Allocator alloc)
{
    (void)alloc;
    io_println("utf8_iter_str");
    {
        ConstStr sample = "hé";
        ResultUtf8Iter iterRes = utf8_iter_str(sample);
        assert_ok(iterRes.error, "utf8_iter_str iterRes != ERR_OK");
        assert_true(iterRes.value.ptr == sample, "utf8_iter_str ptr != sample");
        assert_true(iterRes.value.end == NULL, "utf8_iter_str end != NULL");

        ResultUtf8Iter iterNull = utf8_iter_str(NULL);
        assert_true(iterNull.error.code != ERR_OK, "utf8_iter_str iterNull == ERR_OK");
    }
    io_println("utf8_iter_buff");
    {
        const char *emoji = "😀";
        ConstBuff buff = {
            .bytes = (i8*)emoji,
            .size = (u64)(sizeof("😀") - 1),
        };
        ResultUtf8Iter buffRes = utf8_iter_buff(buff);
        assert_ok(buffRes.error, "utf8_iter_buff buffRes != ERR_OK");
        assert_true(buffRes.value.ptr == emoji, "utf8_iter_buff ptr != emoji");
        assert_true(buffRes.value.end == emoji + (sizeof("😀") - 1), "utf8_iter_buff end != emoji + size");

        ConstBuff badBuff = {
            .bytes = NULL,
            .size = 4,
        };
        ResultUtf8Iter buffErr = utf8_iter_buff(badBuff);
        assert_true(buffErr.error.code != ERR_OK, "utf8_iter_buff buffErr == ERR_OK");
    }
    io_println("utf8_iter_has_next");
    {
        ResultUtf8Iter iterRes = utf8_iter_str("A");
        assert_ok(iterRes.error, "utf8_iter_has_next iterRes != ERR_OK");
        Utf8Iter it = iterRes.value;
        assert_true(utf8_iter_has_next(&it), "utf8_iter_has_next first != true");
        utf8_iter_next(&it);
        assert_true(!utf8_iter_has_next(&it), "utf8_iter_has_next end == true");
        assert_true(!utf8_iter_has_next(NULL), "utf8_iter_has_next NULL != false");
    }
    io_println("utf8_iter_peek_next");
    {
        ConstStr sample = "A☕";
        ResultUtf8Iter iterRes = utf8_iter_str(sample);
        assert_ok(iterRes.error, "utf8_iter_peek_next iterRes != ERR_OK");

        Utf8Iter it = iterRes.value;
        ResultUtf8Codepoint peekA = utf8_iter_peek(&it);
        assert_ok(peekA.error, "utf8_iter_peek peekA != ERR_OK");
        assert_true(peekA.value.codepoint == 'A', "utf8_iter_peek peekA codepoint != 'A'");
        assert_true(peekA.value.width == 1, "utf8_iter_peek peekA width != 1");

        ResultUtf8Codepoint nextA = utf8_iter_next(&it);
        assert_ok(nextA.error, "utf8_iter_next nextA != ERR_OK");
        assert_true(nextA.value.codepoint == 'A', "utf8_iter_next nextA codepoint != 'A'");
        assert_true(it.ptr == sample + 1, "utf8_iter_next pointer not advanced to second codepoint");

        ResultUtf8Codepoint peekCoffee = utf8_iter_peek(&it);
        assert_ok(peekCoffee.error, "utf8_iter_peek peekCoffee != ERR_OK");
        assert_true(peekCoffee.value.codepoint == 0x2615, "utf8_iter_peek peekCoffee codepoint != ☕");
        assert_true(peekCoffee.value.width == 3, "utf8_iter_peek peekCoffee width != 3");

        ResultUtf8Codepoint nextCoffee = utf8_iter_next(&it);
        assert_ok(nextCoffee.error, "utf8_iter_next nextCoffee != ERR_OK");
        assert_true(nextCoffee.value.codepoint == 0x2615, "utf8_iter_next nextCoffee codepoint != ☕");
        assert_true(!utf8_iter_has_next(&it), "utf8_iter_peek_next has_next after end");
    }
    io_println("utf8_iter_next_invalid");
    {
        char invalidSeq[] = {(char)0xE2, 0x28, (char)0xA1, 0};
        ResultUtf8Iter iterRes = utf8_iter_str(invalidSeq);
        assert_ok(iterRes.error, "utf8_iter_next_invalid iterRes != ERR_OK");

        Utf8Iter it = iterRes.value;
        ResultUtf8Codepoint invalidCp = utf8_iter_next(&it);
        assert_true(invalidCp.error.code != ERR_OK, "utf8_iter_next_invalid invalidCp == ERR_OK");
    }
    io_println("utf8_iter_next_truncated");
    {
        const char *euro = "€";
        ConstBuff truncatedBuff = {
            .bytes = (i8*)euro,
            .size = 2,
        };
        ResultUtf8Iter iterRes = utf8_iter_buff(truncatedBuff);
        assert_ok(iterRes.error, "utf8_iter_next_truncated iterRes != ERR_OK");

        Utf8Iter it = iterRes.value;
        ResultUtf8Codepoint truncated = utf8_iter_next(&it);
        assert_true(truncated.error.code != ERR_OK, "utf8_iter_next_truncated truncated == ERR_OK");
    }
    io_println("utf8_iter_advance_bytes");
    {
        ConstStr sample = "ab☕";
        ResultUtf8Iter iterRes = utf8_iter_str(sample);
        assert_ok(iterRes.error, "utf8_iter_advance_bytes iterRes != ERR_OK");

        Utf8Iter it = iterRes.value;
        utf8_iter_advance_bytes(&it, 2);
        ResultUtf8Codepoint cp = utf8_iter_peek(&it);
        assert_ok(cp.error, "utf8_iter_advance_bytes cp != ERR_OK");
        assert_true(cp.value.codepoint == 0x2615, "utf8_iter_advance_bytes codepoint != ☕");

        utf8_iter_advance_bytes(&it, 10);
        assert_true(!utf8_iter_has_next(&it), "utf8_iter_advance_bytes still has next");
    }
}

static void _xstd_list_tests(Allocator alloc)
{
    Error err;
    Allocator badAlloc = _xstd_bad_alloc();

    io_println("list_init");
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
    io_println("list_push");
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
    io_println("list_pop");
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
    io_println("list_get");
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
    io_println("list_set");
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
    io_println("list_getref");
    {
        ResultList res = list_init(&alloc, sizeof(u64), 16);
        assert_ok(res.error, "list_getref res.error.code != ERR_OK");

        List l = res.value;

        u64 item = 5;
        list_push(&l, &item);

        u64 *ref = (u64*)list_getref(&l, 0);
        assert_true(ref != NULL, "list_getref ref == NULL");
        assert_true(*ref == 5, "list_getref *ref != 5");

        u64 *ref2 = (u64*)list_getref(&l, 5);
        assert_true(ref2 == NULL, "list_getref ref2 != NULL");

        u64 *ref3 = (u64*)list_getref(NULL, 0);
        assert_true(ref3 == NULL, "list_getref ref3 != NULL");

        list_deinit(&l);
    }
    io_println("list_get_unsafe");
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
    io_println("list_set_unsafe");
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
    io_println("list_getref_unsafe");
    {
        ResultList res = list_init(&alloc, sizeof(u64), 16);
        assert_ok(res.error, "list_getref_unsafe res.error.code != ERR_OK");

        List l = res.value;

        u64 item = 5;
        list_push(&l, &item);

        u64 *ref = (u64*)list_getref_unsafe(&l, 0);
        assert_true(ref != NULL, "list_getref_unsafe ref == NULL");
        assert_true(*ref == 5, "list_getref_unsafe *ref != 5");

        list_deinit(&l);
    }
    io_println("list_free_items");
    {
        ResultList res = ListInitT(HeapStr, &alloc);
        assert_ok(res.error, "list_free_items res.error.code != ERR_OK");

        List l = res.value;

        HeapStr str1 = ConstToHeapStr(&alloc, "Test string 1");
        HeapStr str2 = ConstToHeapStr(&alloc, "Test string 2");

        ListPushT(HeapStr, &l, &str1);
        ListPushT(HeapStr, &l, &str2);

        list_free_items(&alloc, &l);
        list_deinit(&l);
    }
    io_println("list_clear");
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
    io_println("list_clear_nofree");
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
    io_println("list_for_each");
    {
        ResultList res = ListInitT(HeapStr, &alloc);
        assert_ok(res.error, "list_for_each res.error.code != ERR_OK");

        List l = res.value;

        HeapStr str1 = ConstToHeapStr(&alloc, "Test string 1");
        HeapStr str2 = ConstToHeapStr(&alloc, "Test string 2");

        ListPushT(HeapStr, &l, &str1);
        ListPushT(HeapStr, &l, &str2);

        list_for_each(&l, _xstd_foreach_test, NULL);

        String *strRef1 = (String*)list_getref(&l, 0);
        assert_str_eq(*strRef1, " est string 1", "list_for_each *strRef1 != \" est string 1\"");

        String *strRef2 = (String*)list_getref(&l, 1);
        assert_str_eq(*strRef2, " est string 2", "list_for_each *strRef2 != \" est string 2\"");

        list_free_items(&alloc, &l);
        list_deinit(&l);
    }
}

static void _xstd_math_tests(Allocator alloc)
{
    (void)alloc;

    io_println("add");
    {
        u8 addRes1 = math_u8_add(0, 1);
        assert_true(addRes1 == 1, "math_u8_add addRes1 != 1");

        i8 addResI1 = math_i8_add(-2, 3);
        assert_true(addResI1 == 1, "math_i8_add addResI1 != 1");

        u8 addRes2 = math_u8_add(EnumMaxVal.U8, 1);
        assert_true(addRes2 == 0, "math_u8_add addRes2 != 0");

        u16 addRes3 = math_u16_add(0, 1);
        assert_true(addRes3 == 1, "math_u16_add addRes3 != 1");

        u16 addRes4 = math_u16_add(EnumMaxVal.U16, 1);
        assert_true(addRes4 == 0, "math_u16_add addRes4 != 0");

        i16 addResI16 = math_i16_add(-10, 5);
        assert_true(addResI16 == -5, "math_i16_add addResI16 != -5");

        u32 addRes5 = math_u32_add(0, 1);
        assert_true(addRes5 == 1, "math_u32_add addRes5 != 1");

        u32 addRes6 = math_u32_add(EnumMaxVal.U32, 1);
        assert_true(addRes6 == 0, "math_u32_add addRes6 != 0");

        i32 addResI32 = math_i32_add(-3, -7);
        assert_true(addResI32 == -10, "math_i32_add addResI32 != -10");

        u64 addRes7 = math_u64_add(0, 1);
        assert_true(addRes7 == 1, "math_u64_add addRes7 != 1");

        u64 addRes8 = math_u64_add(EnumMaxVal.U64, 1);
        assert_true(addRes8 == 0, "math_u64_add addRes8 != 0");

        i64 addResI64 = math_i64_add(-5, 10);
        assert_true(addResI64 == 5, "math_i64_add addResI64 != 5");

        f32 addResF32 = math_f32_add(1.5f, 2.0f);
        assert_true(addResF32 == 3.5f, "math_f32_add addResF32 != 3.5f");

        f64 addResF64 = math_f64_add(1.0, 2.5);
        assert_true(addResF64 == 3.5, "math_f64_add addResF64 != 3.5");
    }

    io_println("add_nooverflow");
    {
        ResultU8 addResNo1 = math_u8_add_nooverflow(0, 1);
        assert_ok(addResNo1.error, "math_u8_add_nooverflow addResNo1 != OK");
        assert_true(addResNo1.value == 1, "math_u8_add_nooverflow addResNo1 != 1");

        ResultU8 addResNo2 = math_u8_add_nooverflow(EnumMaxVal.U8, 1);
        assert_true(addResNo2.error.code != ERR_OK, "math_u8_add_nooverflow addResNo2 == OK");

        ResultI8 addResNo3 = math_i8_add_nooverflow(-5, 3);
        assert_ok(addResNo3.error, "math_i8_add_nooverflow addResNo3 != OK");
        assert_true(addResNo3.value == -2, "math_i8_add_nooverflow addResNo3 != -2");

        ResultI8 addResNo4 = math_i8_add_nooverflow(EnumMaxVal.I8_MAX, 1);
        assert_true(addResNo4.error.code != ERR_OK, "math_i8_add_nooverflow addResNo4 == OK");

        ResultU16 addResNo5 = math_u16_add_nooverflow(0, 1);
        assert_ok(addResNo5.error, "math_u16_add_nooverflow addResNo5 != OK");
        assert_true(addResNo5.value == 1, "math_u16_add_nooverflow addResNo5 != 1");

        ResultU16 addResNo6 = math_u16_add_nooverflow(EnumMaxVal.U16, 1);
        assert_true(addResNo6.error.code != ERR_OK, "math_u16_add_nooverflow addResNo6 == OK");

        ResultI32 addResNo7 = math_i32_add_nooverflow(-100, 50);
        assert_ok(addResNo7.error, "math_i32_add_nooverflow addResNo7 != OK");
        assert_true(addResNo7.value == -50, "math_i32_add_nooverflow addResNo7 != -50");

        ResultI32 addResNo8 = math_i32_add_nooverflow(EnumMaxVal.I32_MAX, 1);
        assert_true(addResNo8.error.code != ERR_OK, "math_i32_add_nooverflow addResNo8 == OK");

        ResultU64 addResNo9 = math_u64_add_nooverflow(0, 1);
        assert_ok(addResNo9.error, "math_u64_add_nooverflow addResNo9 != OK");
        assert_true(addResNo9.value == 1, "math_u64_add_nooverflow addResNo9 != 1");

        ResultU64 addResNo10 = math_u64_add_nooverflow(EnumMaxVal.U64, 1);
        assert_true(addResNo10.error.code != ERR_OK, "math_u64_add_nooverflow addResNo10 == OK");

        ResultI64 addResNo11 = math_i64_add_nooverflow(-5, 10);
        assert_ok(addResNo11.error, "math_i64_add_nooverflow addResNo11 != OK");
        assert_true(addResNo11.value == 5, "math_i64_add_nooverflow addResNo11 != 5");

        ResultI64 addResNo12 = math_i64_add_nooverflow(EnumMaxVal.I64_MAX, 1);
        assert_true(addResNo12.error.code != ERR_OK, "math_i64_add_nooverflow addResNo12 == OK");

        ResultI64 addResNo13 = math_i64_add_nooverflow(EnumMaxVal.I64_MIN, -1);
        assert_true(addResNo13.error.code != ERR_OK, "math_i64_add_nooverflow addResNo13 == OK");
    }

    io_println("substract");
    {
        u8 subRes1 = math_u8_substract(5, 2);
        assert_true(subRes1 == 3, "math_u8_substract subRes1 != 3");

        u8 subResWrap = math_u8_substract(0, 1);
        assert_true(subResWrap == EnumMaxVal.U8, "math_u8_substract subResWrap != EnumMaxVal.U8");

        i16 subResI = math_i16_substract(-5, 10);
        assert_true(subResI == -15, "math_i16_substract subResI != -15");

        u32 subRes2 = math_u32_substract(100, 100);
        assert_true(subRes2 == 0, "math_u32_substract subRes2 != 0");

        f32 subResF = math_f32_substract(5.5f, 2.0f);
        assert_true(subResF == 3.5f, "math_f32_substract subResF != 3.5f");
    }

    io_println("substract_nooverflow");
    {
        ResultU8 subNo1 = math_u8_substract_nooverflow(5, 3);
        assert_ok(subNo1.error, "math_u8_substract_nooverflow subNo1 != OK");
        assert_true(subNo1.value == 2, "math_u8_substract_nooverflow subNo1 != 2");

        ResultU8 subNo2 = math_u8_substract_nooverflow(0, 1);
        assert_true(subNo2.error.code != ERR_OK, "math_u8_substract_nooverflow subNo2 == OK");

        ResultI16 subNo3 = math_i16_substract_nooverflow(10, -5);
        assert_ok(subNo3.error, "math_i16_substract_nooverflow subNo3 != OK");
        assert_true(subNo3.value == 15, "math_i16_substract_nooverflow subNo3 != 15");

        ResultI16 subNo4 = math_i16_substract_nooverflow(EnumMaxVal.I16_MIN, 1);
        assert_true(subNo4.error.code != ERR_OK, "math_i16_substract_nooverflow subNo4 == OK");

        ResultU64 subNo5 = math_u64_substract_nooverflow(10, 1);
        assert_ok(subNo5.error, "math_u64_substract_nooverflow subNo5 != OK");
        assert_true(subNo5.value == 9, "math_u64_substract_nooverflow subNo5 != 9");

        ResultU64 subNo6 = math_u64_substract_nooverflow(0, 1);
        assert_true(subNo6.error.code != ERR_OK, "math_u64_substract_nooverflow subNo6 == OK");

        ResultI64 subNo7 = math_i64_substract_nooverflow(25, -5);
        assert_ok(subNo7.error, "math_i64_substract_nooverflow subNo7 != OK");
        assert_true(subNo7.value == 30, "math_i64_substract_nooverflow subNo7 != 30");

        ResultI64 subNo8 = math_i64_substract_nooverflow(EnumMaxVal.I64_MIN, 1);
        assert_true(subNo8.error.code != ERR_OK, "math_i64_substract_nooverflow subNo8 == OK");
    }

    io_println("multiply");
    {
        u16 mulRes1 = math_u16_multiply(4, 5);
        assert_true(mulRes1 == 20, "math_u16_multiply mulRes1 != 20");

        i32 mulRes2 = math_i32_multiply(-6, 3);
        assert_true(mulRes2 == -18, "math_i32_multiply mulRes2 != -18");

        f32 mulRes3 = math_f32_multiply(2.5f, 4.0f);
        assert_true(mulRes3 == 10.0f, "math_f32_multiply mulRes3 != 10.0f");

        f64 mulRes4 = math_f64_multiply(1.5, 2.0);
        assert_true(mulRes4 == 3.0, "math_f64_multiply mulRes4 != 3.0");
    }

    io_println("multiply_nooverflow");
    {
        ResultU16 mulNo1 = math_u16_multiply_nooverflow(4, 5);
        assert_ok(mulNo1.error, "math_u16_multiply_nooverflow mulNo1 != OK");
        assert_true(mulNo1.value == 20, "math_u16_multiply_nooverflow mulNo1 != 20");

        ResultU16 mulNo2 = math_u16_multiply_nooverflow(EnumMaxVal.U16, 2);
        assert_true(mulNo2.error.code != ERR_OK, "math_u16_multiply_nooverflow mulNo2 == OK");

        ResultI32 mulNo3 = math_i32_multiply_nooverflow(-10, -4);
        assert_ok(mulNo3.error, "math_i32_multiply_nooverflow mulNo3 != OK");
        assert_true(mulNo3.value == 40, "math_i32_multiply_nooverflow mulNo3 != 40");

        ResultI32 mulNo4 = math_i32_multiply_nooverflow(EnumMaxVal.I32_MAX, 2);
        assert_true(mulNo4.error.code != ERR_OK, "math_i32_multiply_nooverflow mulNo4 == OK");

        ResultU64 mulNo5 = math_u64_multiply_nooverflow(2, EnumMaxVal.U64);
        assert_true(mulNo5.error.code != ERR_OK, "math_u64_multiply_nooverflow mulNo5 == OK");

        ResultI64 mulNo6 = math_i64_multiply_nooverflow(-12, 3);
        assert_ok(mulNo6.error, "math_i64_multiply_nooverflow mulNo6 != OK");
        assert_true(mulNo6.value == -36, "math_i64_multiply_nooverflow mulNo6 != -36");

        ResultI64 mulNo7 = math_i64_multiply_nooverflow(EnumMaxVal.I64_MIN, -1);
        assert_true(mulNo7.error.code != ERR_OK, "math_i64_multiply_nooverflow mulNo7 == OK");
    }

    io_println("divide");
    {
        ResultU8 divRes0 = math_u8_divide(9, 3);
        assert_ok(divRes0.error, "math_u8_divide divRes0 != OK");
        assert_true(divRes0.value == 3, "math_u8_divide divRes0 != 3");

        ResultU8 divRes0Err = math_u8_divide(9, 0);
        assert_true(divRes0Err.error.code == ERR_INVALID_PARAMETER, "math_u8_divide divRes0Err != ERR_INVALID_PARAMETER");

        ResultU32 divRes1 = math_u32_divide(10, 2);
        assert_ok(divRes1.error, "math_u32_divide divRes1 != OK");
        assert_true(divRes1.value == 5, "math_u32_divide divRes1 != 5");

        ResultU32 divRes2 = math_u32_divide(10, 0);
        assert_true(divRes2.error.code == ERR_INVALID_PARAMETER, "math_u32_divide divRes2 != ERR_INVALID_PARAMETER");

        ResultI32 divRes3 = math_i32_divide(-12, 3);
        assert_ok(divRes3.error, "math_i32_divide divRes3 != OK");
        assert_true(divRes3.value == -4, "math_i32_divide divRes3 != -4");

        ResultF32 divRes4 = math_f32_divide(5.0f, 2.0f);
        assert_ok(divRes4.error, "math_f32_divide divRes4 != OK");
        assert_true(divRes4.value == 2.5f, "math_f32_divide divRes4 != 2.5f");

        ResultF32 divRes5 = math_f32_divide(1.0f, 0.0f);
        assert_true(divRes5.error.code == ERR_INVALID_PARAMETER, "math_f32_divide divRes5 != ERR_INVALID_PARAMETER");

        ResultF64 divRes6 = math_f64_divide(6.0, 2.0);
        assert_ok(divRes6.error, "math_f64_divide divRes6 != OK");
        assert_true(divRes6.value == 3.0, "math_f64_divide divRes6 != 3.0");

        ResultF64 divRes7 = math_f64_divide(1.0, 0.0);
        assert_true(divRes7.error.code == ERR_INVALID_PARAMETER, "math_f64_divide divRes7 != ERR_INVALID_PARAMETER");
    }

    io_println("divide_nooverflow");
    {
        ResultU32 divNo1 = math_u32_divide_nooverflow(9, 3);
        assert_ok(divNo1.error, "math_u32_divide_nooverflow divNo1 != OK");
        assert_true(divNo1.value == 3, "math_u32_divide_nooverflow divNo1 != 3");

        ResultU32 divNo2 = math_u32_divide_nooverflow(9, 0);
        assert_true(divNo2.error.code == ERR_INVALID_PARAMETER, "math_u32_divide_nooverflow divNo2 != ERR_INVALID_PARAMETER");

        ResultI32 divNo3 = math_i32_divide_nooverflow(-12, 3);
        assert_ok(divNo3.error, "math_i32_divide_nooverflow divNo3 != OK");
        assert_true(divNo3.value == -4, "math_i32_divide_nooverflow divNo3 != -4");

        ResultI32 divNo4 = math_i32_divide_nooverflow(EnumMaxVal.I32_MIN, -1);
        assert_true(divNo4.error.code != ERR_OK, "math_i32_divide_nooverflow divNo4 == OK");

        ResultU64 divNo5 = math_u64_divide_nooverflow(16, 4);
        assert_ok(divNo5.error, "math_u64_divide_nooverflow divNo5 != OK");
        assert_true(divNo5.value == 4, "math_u64_divide_nooverflow divNo5 != 4");

        ResultU64 divNo6 = math_u64_divide_nooverflow(16, 0);
        assert_true(divNo6.error.code == ERR_INVALID_PARAMETER, "math_u64_divide_nooverflow divNo6 != ERR_INVALID_PARAMETER");

        ResultI64 divNo7 = math_i64_divide_nooverflow(-64, 8);
        assert_ok(divNo7.error, "math_i64_divide_nooverflow divNo7 != OK");
        assert_true(divNo7.value == -8, "math_i64_divide_nooverflow divNo7 != -8");

        ResultI64 divNo8 = math_i64_divide_nooverflow(EnumMaxVal.I64_MIN, -1);
        assert_true(divNo8.error.code != ERR_OK, "math_i64_divide_nooverflow divNo8 == OK");
    }

    io_println("abs");
    {
        i8 absRes1 = math_i8_abs(-5);
        assert_true(absRes1 == 5, "math_i8_abs absRes1 != 5");

        i32 absRes2 = math_i32_abs(-1234);
        assert_true(absRes2 == 1234, "math_i32_abs absRes2 != 1234");

        i64 absRes3 = math_i64_abs(-9876543210LL);
        assert_true(absRes3 == 9876543210LL, "math_i64_abs absRes3 != 9876543210");

        f32 absRes4 = math_f32_abs(-2.5f);
        assert_true(absRes4 == 2.5f, "math_f32_abs absRes4 != 2.5f");

        f64 absRes5 = math_f64_abs(-3.5);
        assert_true(absRes5 == 3.5, "math_f64_abs absRes5 != 3.5");
    }

    io_println("power");
    {
        u8 powRes1 = math_u8_power(2, 4);
        assert_true(powRes1 == 16, "math_u8_power powRes1 != 16");

        i8 powRes2 = math_i8_power(-2, 3);
        assert_true(powRes2 == -8, "math_i8_power powRes2 != -8");

        u64 powRes3 = math_u64_power(2, 10);
        assert_true(powRes3 == 1024, "math_u64_power powRes3 != 1024");

        f32 powRes4 = math_f32_power(2.0f, 3.0f);
        assert_true(powRes4 == 8.0f, "math_f32_power powRes4 != 8.0f");

        f64 powRes5 = math_f64_power(4.0, -1.0);
        assert_true(powRes5 == 0.25, "math_f64_power powRes5 != 0.25");
    }

    io_println("power_nooverflow");
    {
        ResultU8 powNo1 = math_u8_power_nooverflow(2, 4);
        assert_ok(powNo1.error, "math_u8_power_nooverflow powNo1 != OK");
        assert_true(powNo1.value == 16, "math_u8_power_nooverflow powNo1 != 16");

        ResultU8 powNo2 = math_u8_power_nooverflow(4, 5);
        assert_true(powNo2.error.code != ERR_OK, "math_u8_power_nooverflow powNo2 == OK");

        ResultI8 powNo3 = math_i8_power_nooverflow(-2, 3);
        assert_ok(powNo3.error, "math_i8_power_nooverflow powNo3 != OK");
        assert_true(powNo3.value == -8, "math_i8_power_nooverflow powNo3 != -8");

        ResultI8 powNo4 = math_i8_power_nooverflow(4, 4);
        assert_true(powNo4.error.code != ERR_OK, "math_i8_power_nooverflow powNo4 == OK");

        ResultU64 powNo5 = math_u64_power_nooverflow(2, 20);
        assert_ok(powNo5.error, "math_u64_power_nooverflow powNo5 != OK");
        assert_true(powNo5.value == ((u64)1 << 20), "math_u64_power_nooverflow powNo5 != (1 << 20)");

        ResultU64 powNo6 = math_u64_power_nooverflow(2, 64);
        assert_true(powNo6.error.code != ERR_OK, "math_u64_power_nooverflow powNo6 == OK");

        ResultI64 powNo7 = math_i64_power_nooverflow(0, 0);
        assert_ok(powNo7.error, "math_i64_power_nooverflow powNo7 != OK");
        assert_true(powNo7.value == 1, "math_i64_power_nooverflow powNo7 != 1");

        ResultI64 powNo8 = math_i64_power_nooverflow(-2, 3);
        assert_ok(powNo8.error, "math_i64_power_nooverflow powNo8 != OK");

        ResultI64 powNo9 = math_i64_power_nooverflow(2, -1);
        assert_true(powNo9.error.code == ERR_INVALID_PARAMETER, "math_i64_power_nooverflow powNo9 != ERR_INVALID_PARAMETER");

        ResultI64 powNo10 = math_i64_power_nooverflow(2, 63);
        assert_true(powNo10.error.code != ERR_OK, "math_i64_power_nooverflow powNo10 == OK");
    }

    io_println("round");
    {
        f32 roundRes1 = math_f32_round(2.6f);
        assert_true(roundRes1 == 3.0f, "math_f32_round roundRes1 != 3.0f");

        f32 roundRes2 = math_f32_round(-2.4f);
        assert_true(roundRes2 == -2.0f, "math_f32_round roundRes2 != -2.0f");

        f64 roundRes3 = math_f64_round(1.49);
        assert_true(roundRes3 == 1.0, "math_f64_round roundRes3 != 1.0");

        f64 roundRes4 = math_f64_round(1.5);
        assert_true(roundRes4 == 2.0, "math_f64_round roundRes4 != 2.0");

        f64 roundRes5 = math_f64_round(-1.6);
        assert_true(roundRes5 == -2.0, "math_f64_round roundRes5 != -2.0");
    }
}
static void _xstd_mem_tests(void)
{
    io_println("mem_copy zero size");
    {
        u8 src[4] = {1, 2, 3, 4};
        u8 dst[4] = {9, 9, 9, 9};

        mem_copy(dst, src, 0);

        for (u64 i = 0; i < 4; ++i)
            assert_true(dst[i] == 9, "mem_copy zero size modified destination");
    }

    io_println("mem_copy full copy");
    {
        u8 src[32];
        u8 dst[32] = {0};

        for (u64 i = 0; i < sizeof(src); ++i)
            src[i] = (u8)(i * 3);

        mem_copy(dst, src, sizeof(src));

        for (u64 i = 0; i < sizeof(src); ++i)
            assert_true(dst[i] == src[i], "mem_copy full copy mismatch");
    }

    io_println("mem_copy unaligned");
    {
        u8 src[40];
        u8 dst[40];

        for (u64 i = 0; i < 40; ++i)
        {
            src[i] = (u8)(i + 5);
            dst[i] = 0;
        }

        mem_copy(dst + 3, src + 1, 31);

        for (u64 i = 0; i < 31; ++i)
            assert_true(dst[i + 3] == src[i + 1], "mem_copy unaligned mismatch");
    }

    io_println("mem_copy same buffer");
    {
        u8 buff[24];
        for (u64 i = 0; i < sizeof(buff); ++i)
            buff[i] = (u8)(i ^ 0xAA);

        u8 expected[24];
        for (u64 i = 0; i < sizeof(buff); ++i)
            expected[i] = buff[i];

        mem_copy(buff, buff, sizeof(buff));

        for (u64 i = 0; i < sizeof(buff); ++i)
            assert_true(buff[i] == expected[i], "mem_copy same buffer altered data");
    }
}
