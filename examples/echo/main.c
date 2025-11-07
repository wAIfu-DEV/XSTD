#include "../../xstd.h"

int main(void)
{
    Allocator *a = default_allocator();
    io_println("Echo started, type \"quit\" to exit.");

    while (true)
    {
        // Read line from stdin
        ResultOwnedStr inputRes = io_read_line(a);
        assert_ok(inputRes.error, "Could not read from stdin.");

        // Handle exit
        if (string_equals(inputRes.value, "quit"))
            return 0;

        io_println(inputRes.value);

        // Free owned memory
        a->free(a, inputRes.value);
    }
}
