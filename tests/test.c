#include "../xstd.h"
#include "xstd_tests.h"

int main(void)
{
    DebugAllocatorState dbgState;
    Allocator dbgAlloc = debug_allocator(&dbgState, &c_allocator);

    io_print("Tests: ");

    io_print("lists, ");
    _xstd_list_tests(dbgAlloc);

    io_print("strings");
    _xstd_stringing_tests(dbgAlloc);

    io_println(" - Passed.");

    debug_allocator_logstats(&dbgState);
    return 0;
}
