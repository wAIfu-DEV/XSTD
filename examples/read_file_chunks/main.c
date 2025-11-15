#include "../../xstd.h"

i32 main(i32 argc, String* argv)
{
    // CLI args, specifically on windows get ANSI encoded, which is retarded,
    // and break file paths which may have utf8 chars in them.
    // This functions ensures the args are UTF-8 encoded regardless of platform.
    String* args = io_args_utf8(argc, argv);

    if (argc != 2)
    {
        crash_print("More or less than 1 argument.", 1);
        return 1;
    }

    Allocator* a = default_allocator();

    String filePath = args[1];
    
    if (!file_exists(filePath))
    {
        ResultOwnedStr res = string_concat(a, "File does not exist: ", filePath);
        crash_print(res.error.code ? "File does not exist." : res.value, 1);
        return 1;
    }

    ResultFile fRes = file_open(filePath, EnumFileOpenMode.READ);
    if (fRes.error.code)
    {
        crash_print_error(fRes.error, "Cannot open file.", 1);
        return 1;
    }

    File* f = &fRes.value;

    while(true)
    {
        ResultOwnedStr chunkRes = file_read_str(a, f, 8);
        if (chunkRes.error.code)
        {
            crash_print_error(chunkRes.error, "Read error.", 1);
            return 1;
        }

        OwnedStr chunk = chunkRes.value;
        io_print(chunk);

        a->free(a, chunk);

        if (file_is_eof(f))
            break;
    }
    
    file_close(f);
    return 0;
}
