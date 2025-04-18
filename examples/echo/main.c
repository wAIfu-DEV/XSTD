#include "../../xstd.h"

int main(void)
{
    io_println("Echo started, type \"quit\" to exit.");

    while (true)
    {
        // Read line from stdin
        ResultOwnedStr inputRes = io_read_line(&c_allocator);

        // Handle possible errors
        switch (inputRes.error)
        {
        case ERR_FILE_CANT_READ:
            continue;
        default:
            x_assertErr(inputRes.error, "Could not read from stdin.");
        }

        HeapStr input = inputRes.value;

        // Handle exit
        if (string_equals(input, "quit"))
            return 0;

        io_println(input);

        // Free owned memory
        c_allocator.free(&c_allocator, input);
    }
}
