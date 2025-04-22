#include "../xstd.h"
#include "xstd_tests.h"

int main(void)
{
    process_setup_default_crash_handler();

    DebugAllocatorState dbgState;
    ResultAllocator dbgAllocRes = debug_allocator(&dbgState, &c_allocator);
    assert_ok(dbgAllocRes.error, "Failed to create debug allocator.");

    Allocator dbgAlloc = dbgAllocRes.value;

    io_print("Tests: ");

    io_print("lists, ");
    _xstd_list_tests(dbgAlloc);

    io_print("strings, ");
    _xstd_string_tests(dbgAlloc);

    io_print("maths");
    _xstd_math_tests(dbgAlloc);

    io_println(" - Passed.");

    debug_allocator_logstats(&dbgState);
    return 0;
}
