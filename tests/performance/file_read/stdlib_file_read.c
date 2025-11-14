#include <errno.h>
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

#define CHUNK_SIZE 65536u
#define RUNS 500u

static int read_once(const char *path, uint64_t *bytesRead)
{
    FILE *file = fopen(path, "rb");
    if (!file)
    {
        fprintf(stderr, "Failed to open file: %s\n", path);
        return 1;
    }

    unsigned char *buffer = (unsigned char *)malloc(CHUNK_SIZE);
    if (!buffer)
    {
        fprintf(stderr, "Failed to allocate buffer.\n");
        fclose(file);
        return 1;
    }

    uint64_t localBytes = 0;

    while (1)
    {
        size_t readCount = fread(buffer, 1, CHUNK_SIZE, file);
        if (readCount > 0)
            localBytes += readCount;

        if (readCount < CHUNK_SIZE)
        {
            if (ferror(file))
            {
                fprintf(stderr, "Read error encountered.\n");
                free(buffer);
                fclose(file);
                return 1;
            }
            break;
        }
    }

    free(buffer);
    fclose(file);

    *bytesRead += localBytes;
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: stdlib_file_read <file_path>\n");
        return 1;
    }

    const uint64_t start = time_unix_ms_std();
    uint64_t totalBytes = 0;

    for (uint32_t run = 0; run < RUNS; ++run)
        if (read_once(argv[1], &totalBytes) != 0)
            return 1;

    const uint64_t duration = time_unix_ms_std() - start;

    printf("bytes_read=%llu\n", (unsigned long long)totalBytes);
    printf("chunk_size=%u\n", CHUNK_SIZE);
    printf("runs=%u\n", RUNS);
    printf("duration_ms=%llu\n", (unsigned long long)duration);
    return 0;
}
