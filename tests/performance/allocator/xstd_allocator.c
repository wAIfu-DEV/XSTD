#include "xstd.h"

#define RUNS 5u
#define OPERATIONS_PER_RUN 200000u
#define BLOCK_SIZE 64u

static volatile void *g_alloc_sink = NULL;

static u64 run_allocator_cycle(Allocator *alloc, u64 *failures)
{
    const u64 start = time_unix_ms();

    for (u64 i = 0; i < OPERATIONS_PER_RUN; ++i)
    {
        void *ptr = alloc->alloc(alloc, BLOCK_SIZE);
        if (!ptr)
        {
            if (failures)
                ++(*failures);
            continue;
        }
        g_alloc_sink = ptr;
        alloc->free(alloc, ptr);
    }

    const u64 end = time_unix_ms();
    return end - start;
}

int main(void)
{
    Allocator *alloc = default_allocator();
    if (!alloc)
        return 1;

    io_println("xstd allocator benchmark");
    io_print("operations_per_run=");
    io_print_uint(OPERATIONS_PER_RUN);
    io_println("");
    io_print("block_size=");
    io_print_uint(BLOCK_SIZE);
    io_println(" bytes");

    u64 totalMs = 0;
    u64 totalFailures = 0;

    for (u32 run = 0; run < RUNS; ++run)
    {
        u64 failures = 0;
        const u64 elapsed = run_allocator_cycle(alloc, &failures);
        totalMs += elapsed;
        totalFailures += failures;

        io_print("run ");
        io_print_uint(run + 1);
        io_print(": ");
        io_print_uint(elapsed);
        io_println(" ms");
    }

    io_print("average_ms=");
    io_print_uint(totalMs / RUNS);
    io_println("");
    io_print("total_failures=");
    io_print_uint(totalFailures);
    io_println("");

    return totalFailures ? 1 : 0;
}
