#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
static uint64_t time_unix_ms_std(void)
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    const uint64_t ticks = ((uint64_t)ft.dwHighDateTime << 32) | (uint64_t)ft.dwLowDateTime;
    const uint64_t epochDiff = 116444736000000000ULL;
    return (ticks - epochDiff) / 10000ULL;
}
#else
#include <sys/time.h>
static uint64_t time_unix_ms_std(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000ULL + (uint64_t)(tv.tv_usec / 1000ULL);
}
#endif

#define RUNS 5u
#define OPERATIONS_PER_RUN 200000u
#define BLOCK_SIZE 64u

static volatile void *g_alloc_sink = NULL;

static uint64_t run_allocator_cycle(uint64_t *failures)
{
    const uint64_t start = time_unix_ms_std();

    for (uint64_t i = 0; i < OPERATIONS_PER_RUN; ++i)
    {
        void *ptr = malloc(BLOCK_SIZE);
        if (!ptr)
        {
            if (failures)
                ++(*failures);
            continue;
        }
        g_alloc_sink = ptr;
        free(ptr);
    }

    const uint64_t end = time_unix_ms_std();
    return end - start;
}

int main(void)
{
    printf("stdlib allocator benchmark\n");
    printf("operations_per_run=%u\n", OPERATIONS_PER_RUN);
    printf("block_size=%u bytes\n", BLOCK_SIZE);

    uint64_t totalMs = 0;
    uint64_t totalFailures = 0;

    for (uint32_t run = 0; run < RUNS; ++run)
    {
        uint64_t failures = 0;
        const uint64_t elapsed = run_allocator_cycle(&failures);
        totalMs += elapsed;
        totalFailures += failures;
        printf("run %u: %llu ms\n", run + 1, (unsigned long long)elapsed);
    }

    printf("average_ms=%llu\n", (unsigned long long)(totalMs / RUNS));
    printf("total_failures=%llu\n", (unsigned long long)totalFailures);
    return totalFailures ? 1 : 0;
}
