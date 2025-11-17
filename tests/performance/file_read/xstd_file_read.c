#include "xstd.h"

#define CHUNK_SIZE 65536u
#define RUNS 500u

static i32 read_once(Allocator *alloc, ConstStr path, u64 *bytesRead)
{
    ResFile fileRes = file_open(path, EnumFileOpenMode.READ);
    if (fileRes.isErr)
    {
        io_printerrln(fileRes.err.msg ? fileRes.err.msg : "Failed to open file.");
        return 1;
    }

    File file = fileRes.value;
    u64 localBytes = 0;

    while (true)
    {
        ResOwnedBuff chunkRes = file_read_bytes(alloc, &file, CHUNK_SIZE);
        if (chunkRes.isErr)
        {
            io_printerrln(chunkRes.err.msg ? chunkRes.err.msg : "file_read_bytes failed.");
            if (chunkRes.value.bytes)
                alloc->free(alloc, chunkRes.value.bytes);
            file_close(&file);
            return 1;
        }

        if (!chunkRes.value.bytes || chunkRes.value.size == 0)
        {
            if (chunkRes.value.bytes)
                alloc->free(alloc, chunkRes.value.bytes);
            break;
        }

        localBytes += chunkRes.value.size;
        alloc->free(alloc, chunkRes.value.bytes);

        if (chunkRes.value.size < CHUNK_SIZE || file_is_eof(&file))
            break;
    }

    file_close(&file);
    *bytesRead += localBytes;
    return 0;
}

i32 main(i32 argc, String *argv)
{
    String *args = io_args_utf8(argc, argv);
    if (argc != 2)
    {
        io_printerrln("Usage: xstd_file_read <file_path>");
        return 1;
    }

    Allocator *alloc = default_allocator();
    if (!alloc)
        return 1;

    const u64 start = time_unix_ms();
    u64 totalBytes = 0;

    for (u32 run = 0; run < RUNS; ++run)
    {
        if (read_once(alloc, args[1], &totalBytes) != 0)
            return 1;
    }

    const u64 duration = time_unix_ms() - start;

    io_print("bytes_read=");
    io_print_uint(totalBytes);
    io_println("");
    io_print("chunk_size=");
    io_print_uint(CHUNK_SIZE);
    io_println("");
    io_print("runs=");
    io_print_uint(RUNS);
    io_println("");
    io_print("duration_ms=");
    io_print_uint(duration);
    io_println("");
    return 0;
}
