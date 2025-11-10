#include "../xstd.h"
#include "xstd_tests.h"

int main(void)
{
    DebugAllocatorState dbgState;
    ResultAllocator dbgAllocRes = debug_allocator(&dbgState, 128, default_allocator());
    assert_ok(dbgAllocRes.error, "Failed to create debug allocator.");

    Allocator dbgAlloc = dbgAllocRes.value;

    io_println("[Testing math]:");
    _xstd_math_tests(dbgAlloc);

    io_println("\n[Testing writer]:");
    _xstd_writer_tests(dbgAlloc);

    io_println("\n[Testing list]:");
    _xstd_list_tests(dbgAlloc);

    io_println("\n[Testing utf8]:");
    _xstd_utf8_tests(dbgAlloc);

    io_println("\n[Testing string]:");
    _xstd_string_tests(dbgAlloc);

    io_println("\n[Testing file]:");
    _xstd_file_tests(dbgAlloc);

    io_println("\n[Passed all tests]");

    io_print("Active allocations after test: ");
    io_print_uint(dbgState.activeAllocCount);
    io_print("\n");

    io_print("Active allocated bytes after test: ");
    io_print_uint(dbgState.activeUserBytes);
    io_print("\n");
    return 0;
}
