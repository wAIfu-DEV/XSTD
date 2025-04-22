#include "../../xstd.h"

int main(void)
{
    io_println("Echo started, type \"quit\" to exit.");

    while (true)
    {
        // Read line from stdin
        ResultOwnedStr inputRes = io_read_line(&c_allocator);
        x_assertErr(inputRes.error, "Could not read from stdin.");

        // Handle exit
        if (string_equals(inputRes.value, "quit"))
            return 0;

        io_println(inputRes.value);

        // Free owned memory
        c_allocator.free(&c_allocator, inputRes.value);
    }
}
