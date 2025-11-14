#include "xstd.h"

#define RUNS 100u
#define BUILDS_PER_RUN 100u
#define APPENDS_PER_BUILD 200u
#define INITIAL_CAPACITY 1024u

static Error build_once(Allocator *alloc)
{
    ResultWriter writerRes = growstrwriter_init(*alloc, INITIAL_CAPACITY);
    if (writerRes.error.code)
        return writerRes.error;

    Writer writer = writerRes.value;
    Error err = X_ERR_OK;

    for (u64 i = 0; i < APPENDS_PER_BUILD; ++i)
    {
        err = writer_write_str(&writer, "value-");
        if (err.code != ERR_OK)
            break;

        err = writer_write_uint(&writer, i);
        if (err.code != ERR_OK)
            break;

        err = writer_write_str(&writer, "|payload\n");
        if (err.code != ERR_OK)
            break;
    }

    growstrwriter_deinit(&writer);
    return err;
}

int main(void)
{
    Allocator *alloc = default_allocator();
    if (!alloc)
        return 1;

    const u64 totalBuilds = (u64)RUNS * (u64)BUILDS_PER_RUN;

    io_println("xstd growstrwriter benchmark");
    io_print("runs=");
    io_print_uint(RUNS);
    io_println("");
    io_print("builds_per_run=");
    io_print_uint(BUILDS_PER_RUN);
    io_println("");
    io_print("appends_per_build=");
    io_print_uint(APPENDS_PER_BUILD);
    io_println("");

    const u64 start = time_unix_ms();
    for (u32 run = 0; run < RUNS; ++run)
    {
        for (u32 build = 0; build < BUILDS_PER_RUN; ++build)
        {
            Error err = build_once(alloc);
            if (err.code != ERR_OK)
            {
                io_printerrln(err.msg ? err.msg : "string builder error");
                return 1;
            }
        }
    }
    const u64 duration = time_unix_ms() - start;

    io_print("total_ms=");
    io_print_uint(duration);
    io_println("");
    io_print("avg_ms_per_build=");
    io_print_uint(duration / totalBuilds);
    io_println("");

    return 0;
}
