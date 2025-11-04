#include "../../xstd.h"

int main(void)
{
    // OPEN FILE ===============================================================
    ResultFile resFile = file_open("sample.csv", FileOpenModes.READ);
    assert_ok(resFile.error, "Failed to open sample file");

    File file = resFile.value;

    // INIT ARENA ==============================================================

    { // Stack allocated arena will be freed as it falls out of this scope

        // Allocate 1MB of stack mem
        i8 arenaBufferBytes[1024];
        // Create buffer pointing to mem
        Buffer arenaBuffer = (Buffer){
            .bytes = arenaBufferBytes,
            .size = sizeof(arenaBufferBytes)};

        // Initialize alloc allocator
        ResultAllocator resArena = arena_allocator(arenaBuffer, false);
        assert_ok(resArena.error, "Failed to initialize alloc.");

        Allocator alloc = resArena.value;

        // ^^^
        // This is if you want to use stack memory
        // For heap memory you can use:
        // Allocator alloc = c_allocator;

        // READ FILE ===========================================================

        // Read file as list of lines
        ResultList resLines = file_read_lines(&alloc, &file);
        assert_ok(resLines.error, "Failed to read file as lines");

        List lines = resLines.value;

        // PARSE FILE ==========================================================

        typedef struct _csv_line
        {
            u64 id;
            OwnedStr name;
            u64 age;
        } CsvLine;

        // Initialize result list
        ResultList resultListRes = ListInitT(CsvLine, &alloc);
        assert_ok(resultListRes.error, "Failed to initialize result list.");

        List resultList = resultListRes.value;

        // Iterate lines and parse to CsvLine
        u64 linesNb = list_size(&lines);
        for (u64 i = 1; i < linesNb; ++i)
        {
            // If list items are pointers (here String which is an alias of i8*)
            // list_get_as_ptr allows direct retrieval of the item as a pointer
            String line = list_get_as_ptr(&lines, i);

            // Split line on ',' character, returns list of strings
            ResultList resSplit = string_split_char(&alloc, line, ',');
            assert_ok(resSplit.error, "Failed to split line.");

            List split = resSplit.value;

            String idStr = list_get_as_ptr(&split, 0);
            String nameStr = list_get_as_ptr(&split, 1);
            String ageStr = list_get_as_ptr(&split, 2);

            if (!idStr || !nameStr || !ageStr)
            {
                io_printerrln("Failed to get part of split line.");
                continue;
            }

            // Parse ID and age strings as numbers
            ResultU64 idRes = string_parse_uint(idStr);
            assert_ok(idRes.error, "Failed to parse ID.");

            ResultU64 ageRes = string_parse_uint(ageStr);
            assert_ok(ageRes.error, "Failed to parse age.");

            // Add parsed line to result list
            CsvLine parsed;
            parsed.id = idRes.value;
            parsed.age = ageRes.value;
            parsed.name = nameStr;

            ListPushT(CsvLine, &resultList, &parsed);
        }

        assert_true(list_size(&resultList) > 0, "Could not parse any lines.");

        // PRINT RESULTS =======================================================

        u64 resultNb = list_size(&resultList);
        for (u64 i = 0; i < resultNb; ++i)
        {
            CsvLine *item = list_getref(&resultList, i);
            if (!item)
                continue;

            // prints: "0: id=0 name=Bob age=35"
            io_print_uint(i);
            io_print(": id=");
            io_print_uint(item->id);
            io_print(" name=");
            io_print(item->name);
            io_print(" age=");
            io_print_uint(item->age);
            io_println("");
        }
    } // Arena deallocation
}
