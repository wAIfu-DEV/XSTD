# XSTD - A modern C standard library

# xstd – Foundation Library for C

`xstd` is a lightweight, modular, and extensible standard library implementation for the C language, designed to be used as a drop-in replacement for portions of glibc (aiming for full replacement) and a robust foundation for modern C software development. It aims to provide simpler, safer, and more consistent APIs for everything anyone would want to do using C.

---

## ✨ Features

- 🔐 Safer APIs with built-in result types and error checking
- 🧹 Explicit memory management with pluggable allocators
- 📜 Better string and buffer manipulation primitives
- 📦 Dynamic arrays (lists) with type-safe macros
- 📁 File IO utilities with helpful abstractions
- 📐 Type definitions for fixed-width integers, strings, and buffers
- 👀 More to come...

---

## 🔧 Getting Started

Include only what you need.

```c
#include "xstd_coretypes.h"
#include "xstd_str.h"
```

Or everything.

```c
#include "xstd.h"
```

No external dependencies are required, just copy the files to your project and you are good to go.

---

## 🚀 Example Usage

### Strings

```c
#include "xstd_str.h"

ConstStr a = "Hello";
ConstStr b = ", World!";

ResultOwnedStr result = string_concat(&c_allocator, a, b);

if (result.error != ERR_OK)
{
    io_printerrln("Failed to concat strings.");
    return;
}

io_println(result.value);
c_allocator.free(&c_allocator, result.value);

```

### Dynamic Lists

```c
#include "xstd_list.h"

// Create list
ResultList res = ListInitT(HeapStr, &c_allocator);

if (res.error != ERR_OK)
{
    io_printerrln("Failed to initialize list.");
    return;
}

List list = res.value;

ResultHeapStr dupeRes = string_dupe(&c_allocator, "Hello");

if (dupeRes.error != ERR_OK)
{
    io_printerrln("Failed to duplicate string.");
    return;
}

ListPushT(String, &list, &dupeRes.value);

list_free_items(&c_allocator, &list); // If items are heap pointers, frees them.
list_deinit(&list);
}
```

### File IO

```c
#include "xstd_file.h"

ResultFile fileRes = file_open("hello.txt", FileOpenModes.READ);

if (fileRes.error != ERR_OK)
{
    io_printerrln("Failed to open file.");
    return;
}

ResultOwnedStr contentRes = file_readall_str(&c_allocator, &fileRes.value);

if (contentRes.error != ERR_OK)
{
    io_printerrln("Failed to read file.");
    return;
}

HeapStr fileContents = contentRes.value;

io_println(fileContents);

c_allocator.free(&c_allocator, fileContents);
file_close(&fileRes.value);
```

---

## 📦 Current Modules

| Module | Description |
|--------|-------------|
| `xstd_coretypes.h` | Foundational types: `u8`, `i32`, `String`, `Buffer`, etc. |
| `xstd_alloc.h` | Allocation abstraction (`Allocator` interface) |
| `xstd_debugalloc.h` | Debug allocator wrapper (tracks allocs, logs usage) |
| `xstd_err.h` | Typed error codes with descriptions |
| `xstd_result.h` | `Result<T>` wrappers for error-safe operations |
| `xstd_str.h` | Safer and simple string manipulation |
| `xstd_buffer.h` | Generic binary buffer manipulation |
| `xstd_list.h` | Relatively type-safe dynamic lists (vectors) |
| `xstd_io.h` | IO abstraction and assertion/log helpers |
| `xstd_file.h` | File manipulation with `File` abstraction |

---

## 📚 Philosophy and Goals

- Replace ad-hoc usage of all stdlib features with extensible and safer APIs.
- Verbose but explicit error handling.
- Clear and explicit allocations and memory ownership.
- Avoid undefined behavior in favor of robust error codes.
- Modular and embeddable (no runtime, no external deps)
- Portable C99 up to current-year C.
- Extensible by user: plug in your own allocators, types, and error handling.

---

## 👀 Inspirations

| From | What |
|--------|--------|
| Zig | Allocators, explicit allocations |
| Go, Rust | Explicit error handling |
| GDScript | General purpose Error type |
| JavaScript | For emotional support |

---

## 🔮 Roadmap

| Feature | Status |
|--------|--------|
| Memory allocator abstraction | ✅ |
| Dynamic strings and buffers | ✅ |
| Debug allocator with tracking | ✅ |
| Custom error enums with messages | ✅ |
| Full list/vector API | ✅ |
| File abstraction wrapper | ✅ |
| String builder | ✅ |
| String searching | ✅ |
| Hashmaps / Dictionaries | 🔜 |
| Filesystem API | 🔜 |
| utf-8 support | 🔜 |
| JSON/CSV parsing | 🔜 |

---

## 🔩 Integration Tips

- Compile only modules you use.
- Replace `malloc`/`free`/`realloc` with `c_allocator` or your own custom `Allocator`.
- Include `xstd_err.h` and `xstd_result.h` for strong error handling.
- Use `list_*` and `string_*` instead of raw array manipulation.
- Use `x_assert` during development/debug builds to catch errors early.

---

## 🔗 License

MIT License

---

## 💬 Contribution

This is a work-in-progress foundation. Suggestions or pull requests are welcome. Help us build a modern, safe standard library for C.

---

## Compiler Compatibility

(Tested with std=c99 and std=c17 w. optimizations -O0, 1, 2, 3, fast, s)

| Compiler | Status |
|--------|--------|
| GCC | ✅ |
| CLANG | ✅ |
| ZIG CC | ✅ |
| MSVC | 🔜 |
| Others | 🔜 |

---

## 📢 Author

Made with ❤️ and C by a dev who believe stdlib can be better (respectfully, no ill intent meant)

---

Special thanks for ChatGPT for making part of this README
