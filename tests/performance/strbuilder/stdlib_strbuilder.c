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

#define RUNS 100u
#define BUILDS_PER_RUN 100u
#define APPENDS_PER_BUILD 200u
#define BUFFER_CAPACITY 65536u

static int build_once(void)
{
    char *buffer = (char *)malloc(BUFFER_CAPACITY);
    if (!buffer)
        return 1;

    size_t offset = 0;
    int status = 0;

    for (uint64_t i = 0; i < APPENDS_PER_BUILD; ++i)
    {
        if (offset >= BUFFER_CAPACITY)
        {
            status = 1;
            break;
        }

        int written = snprintf(
            buffer + offset,
            BUFFER_CAPACITY - offset,
            "value-%llu|payload\n",
            (unsigned long long)i);

        if (written < 0)
        {
            status = 1;
            break;
        }

        const size_t consumed = (size_t)written;
        if (consumed >= BUFFER_CAPACITY - offset)
        {
            status = 1;
            break;
        }

        offset += consumed;
    }

    free(buffer);
    return status;
}

int main(void)
{
    const uint64_t totalBuilds = (uint64_t)RUNS * (uint64_t)BUILDS_PER_RUN;

    printf("stdlib snprintf benchmark\n");
    printf("runs=%u\n", RUNS);
    printf("builds_per_run=%u\n", BUILDS_PER_RUN);
    printf("appends_per_build=%u\n", APPENDS_PER_BUILD);

    const uint64_t start = time_unix_ms_std();
    for (uint32_t run = 0; run < RUNS; ++run)
    {
        for (uint32_t build = 0; build < BUILDS_PER_RUN; ++build)
        {
            if (build_once() != 0)
            {
                fprintf(stderr, "string build failed\n");
                return 1;
            }
        }
    }
    const uint64_t duration = time_unix_ms_std() - start;

    printf("total_ms=%llu\n", (unsigned long long)duration);
    printf("avg_ms_per_build=%llu\n", (unsigned long long)(duration / totalBuilds));
    return 0;
}
