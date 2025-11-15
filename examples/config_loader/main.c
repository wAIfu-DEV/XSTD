#include "../../xstd.h"
#include "../../xstd/xstd_hashmap.h"

typedef struct _config_entry
{
    OwnedStr value;
} ConfigEntry;

static void config_print(Buffer key, void *valuePtr, void *userArg)
{
    (void)userArg;
    Allocator *a = default_allocator();

    ConfigEntry *entry = (ConfigEntry *)valuePtr;
    OwnedStr keyStr = NULL;

    if (key.size)
    {
        keyStr = a->alloc(a, key.size + 1);
        if (keyStr)
        {
            for (u64 i = 0; i < key.size; ++i)
                keyStr[i] = key.bytes[i];
            keyStr[key.size] = 0;
        }
    }

    io_print("  ");
    io_print(keyStr ? keyStr : "(invalid-key)");
    io_print(" = ");
    io_println(entry->value ? entry->value : "(null)");

    if (keyStr)
        a->free(a, keyStr);
}

static void config_release(Buffer key, void *valuePtr, void *userArg)
{
    (void)key;

    Allocator *alloc = (Allocator *)userArg;
    ConfigEntry *entry = (ConfigEntry *)valuePtr;

    if (entry->value)
    {
        alloc->free(alloc, entry->value);
        entry->value = NULL;
    }
}

static Bool config_set(HashMap *map, Allocator *storeAlloc, ConstStr key, ConstStr value)
{
    ConfigEntry existing;
    Error getErr = hashmap_get_str(map, key, &existing);
    if (getErr.code == ERR_OK && existing.value)
        storeAlloc->free(storeAlloc, existing.value);

    ResultOwnedStr valueCopy = string_dupe(storeAlloc, value);
    if (valueCopy.error.code)
    {
        io_printerrln("[config] failed to copy value.");
        return false;
    }

    ConfigEntry entry = {
        .value = valueCopy.value,
    };

    Error setErr = hashmap_set_str(map, key, &entry);
    if (setErr.code != ERR_OK)
    {
        storeAlloc->free(storeAlloc, valueCopy.value);
        io_printerrln("[config] failed to insert entry.");
        return false;
    }
    return true;
}

static Bool slice_key_value(Allocator *scratch, ConstStr line, ConstStr *outKey, ConstStr *outValue)
{
    ResultOwnedStr trimmed = string_trim_whitespace(scratch, line, true, true);
    if (trimmed.error.code)
        return false;

    if (!trimmed.value || trimmed.value[0] == 0 || trimmed.value[0] == '#')
        return false;

    i64 commentIdx = string_find_char(trimmed.value, '#');
    if (commentIdx >= 0)
    {
        ResultOwnedStr withoutComment = string_substr(scratch, trimmed.value, 0, (u64)commentIdx);
        if (withoutComment.error.code)
            return false;

        trimmed = string_trim_whitespace(scratch, withoutComment.value, true, true);
        if (trimmed.error.code)
            return false;

        if (!trimmed.value || trimmed.value[0] == 0)
            return false;
    }

    i64 eqIdx = string_find_char(trimmed.value, '=');
    if (eqIdx < 0)
        return false;

    ResultOwnedStr keySlice = string_substr(scratch, trimmed.value, 0, (u64)eqIdx);
    if (keySlice.error.code)
        return false;

    ResultOwnedStr valueSlice = string_substr(scratch, trimmed.value, (u64)eqIdx + 1, string_size(trimmed.value));
    if (valueSlice.error.code)
        return false;

    ResultOwnedStr keyTrim = string_trim_whitespace(scratch, keySlice.value, true, true);
    if (keyTrim.error.code || !keyTrim.value || keyTrim.value[0] == 0)
        return false;

    ResultOwnedStr valueTrim = string_trim_whitespace(scratch, valueSlice.value, true, true);
    if (valueTrim.error.code)
        return false;

    *outKey = keyTrim.value;
    *outValue = valueTrim.value ? valueTrim.value : "";
    return true;
}

int main(void)
{
    Allocator *storeAlloc = default_allocator();

    ResultHashMap mapRes = HashMapInitT(ConfigEntry, storeAlloc);
    assert_ok(mapRes.error, "[config] failed to create hashmap.");
    HashMap config = mapRes.value;

    i8 scratchBytes[4096];
    Buffer scratchBuffer = {
        .bytes = scratchBytes,
        .size = sizeof(scratchBytes),
    };

    ResultAllocator scratchRes = arena_allocator(scratchBuffer, false);
    assert_ok(scratchRes.error, "[config] failed to init scratch arena.");
    Allocator scratch = scratchRes.value;

    ResultFile fileRes = file_open("config.ini", EnumFileOpenMode.READ);
    assert_ok(fileRes.error, "[config] failed to open config.ini");
    File cfgFile = fileRes.value;

    ResultList linesRes = file_read_lines(&scratch, &cfgFile);
    assert_ok(linesRes.error, "[config] failed to read config file.");
    List lines = linesRes.value;

    io_println("Loading config.ini ...");

    u64 lineCount = list_size(&lines);
    for (u64 i = 0; i < lineCount; ++i)
    {
        String rawLine = list_get_as_ptr(&lines, i);

        ConstStr key = NULL;
        ConstStr value = NULL;
        if (!slice_key_value(&scratch, rawLine, &key, &value))
            continue;

        if (!config_set(&config, storeAlloc, key, value))
        {
            io_printerrln("[config] skipping entry due to errors.");
            continue;
        }
    }

    io_println("Loaded configuration:");
    hashmap_for_each(&config, config_print, NULL);

    ConfigEntry welcome = {0};
    Error lookupErr = hashmap_get_str(&config, "welcome_message", &welcome);
    if (lookupErr.code == ERR_OK && welcome.value)
    {
        io_print("\nwelcome_message -> ");
        io_println(welcome.value);
    }
    else
    {
        io_println("\nwelcome_message not configured.");
    }

    list_deinit(&lines);
    hashmap_for_each(&config, config_release, storeAlloc);
    hashmap_deinit(&config);
    file_close(&cfgFile);
    return 0;
}
