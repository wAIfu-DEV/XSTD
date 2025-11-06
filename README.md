# XSTD – A C Standard Library for the 21st Century 🚀

📦 A modern, modular, and powerful C standard library replacement — inspired by modern languages but built for C.

---

## 🛠 What's this?

**xstd** is a lightweight, flexible, and expressive C standard library focused on developer productivity, safety, and ergonomics. It reimagines the C runtime and standard tooling for 2024 and beyond, introducing idioms and patterns inspired by languages like Rust, Go, and Zig — while remaining fully C-compatible and zero-dependency.

✳️ **Highlights**:
- Result types (`ResultOwnedStr`, `ResultOwnedBuff`, `Result<T>`) — no more `NULL` checks or magic number errors.
- Explicit memory ownership though typedefs (`OwnedStr`, `ConstStr`, `String`, `OwnedBuff`, `ConstBuff`) — ownership is never implicit.
- Modular allocators: arena allocators, buffer allocators, tracking/debug allocators, and fallbacks to `malloc`.
- Modern tooling: string builders, hashmaps, IO abstractions, file APIs, and more.
- Safer APIs: bounds-checked string functions, typed list operations, and precise error propagation.
- Full introspection support: allocation metrics, file metadata, and crash diagnostics.
- Cross-platform compatibility: completely minimal interface built on top of stdlib.

---

## 🤯 Why something other than C standard library?

The C standard library (stdlib) has served faithfully for decades, but:

❌ Memory management is unsafe and sometimes invisible — allocations are opaque, leaks happen silently.  
❌ Error handling is often unintuitive — `fopen()` returns NULL and you’re already segfaulting.  
❌ Working with strings is error-prone — `strcat`, `strlen`, global buffers...  
❌ Arrays are unsafe — you never know their size.  
❌ Unsafe and deprecated functions are still widely usable.  

We think: **C deserves better tooling**, and we've built **xstd** to fill that void — a modern, fully C99-compatible library with minimal dependencies and huge ergonomics improvements.

---

## ✨ Features

✅ Error-Driven Design
• `Result<T>` for allocation, file IO, string manipulation, parsing, hashing, etc.
• Clearly defined `ErrorCode` enum with human-readable messages.

✅ Modern Allocator System
• Arena allocator — blazing fast, rolling block allocator
• Buffer allocator — stack-like allocator with free support
• Debug allocator — tracks leaks, counts allocations, checks double frees
• Default fallback stdlib allocator (`c_allocator`)
• Clean, pluggable design with full introspection

✅ Strings & Buffers Made Safe
• Heap strings (owned), const strings, stack strings
• Builders for appending strings without unsafe strcat
• Fully bounds-checked copies, resizes, and concatenation
• File-safe string/bytes APIs
• UTF-8 compatible (ASCII-safe) character operations

✅ Containers (with Types!)
• `List<T>` — realloc-style resizable vector with type checked push/pop
• `HashMap<Str, T>` — safe, dynamic key:value store with string key support
• Safe access macros: `ListPushT`, `HashMapSetStrT`, etc.

✅ File IO You've Always Wanted
• `file_readall_str()` — read the whole file as a string
• `file_read_bytes(n)` — read n bytes safely
• `file_write_uint(file, 1234)`
• Line iterators, flushers, position readers, error checks, etc.

✅ Writers = No More snprintf()
• Writers to buffered memory, dynamically growing buffers, strings
• `Writer w; writer_write_uint(&w, 1234);`

✅ Crash Handling
• Setup crash handler with `process_setup_crash_handler()`
• Intercepts SIGSEGV, SIGABRT, SIGFPE, etc.

✅ Math & String Parsing
• Overflow-checked math operations
• Safe `string_parse_int`, `string_from_float`, etc.
• Standard math ops: power, abs, mul, div, add with overflow detection

✅ Fully Modular
• Include only what you need: `xstd_io.h`, `xstd_file.h`, `xstd_list.h`, etc.
• No global state, allocation decoupled from modules
• Platform compatibility via small, replaceable interfaces (`xstd_os_int.h`)

---

## 📐 Project Philosophy

🔒 Safety:
All operations favor **bounds-checked**, **well-defined behavior**. Unsafe operations are explicitly marked.

🧠 Predictability:
Every function either returns a `Result<T>` or sets up predictable invariants. No silent NULLs.

💡 Composability:
Allocators plug into lists, writers use buffers, hash maps use allocators — everything connects seamlessly.

💨 Performance:
Header-only, inlined functions. Arena allocators for batch allocations. Avoids unnecessary syscalls.

💥 Simplicity:
Simple, portable, and transparent. Absolutely no macros unless for type safety. No hacks. Always readable.

---

## 🔨 Example: Using Lists and Strings

```c
#include "xstd_core.h"
#include "xstd_string.h"

i32 main() {
    Allocator *a = &c_allocator;

    // Most XSTD functions will either return void, Error, or ResultT
    ResultStrBuilder builderRes = strbuilder_init(a);
    
    // Check for errors by comparing error code against 0
    // Error code of 0 (ERR_OK) means success and a defined value,
    // But error code != 0 means failure, and a very likely undefined value.
    if (r.error.code != ERR_OK) {
        io_printerrln(ErrorToString(r.error.code)) // All standard error codes have string representations
        io_printerrln(r.error.msg); // All xstd library errors will also have a descriptive error message!
        return 1;
    }

    StringBuilder builder = r.value;

    // Push strings to the builder
    strbuilder_push_copy(&builder, "Yes! ");
    strbuilder_push_copy(&builder, "We ");
    strbuilder_push_copy(&builder, "Can!");

    // Strings the user need to free will be returned through OwnedStr or ResultOwnedStr.
    // This way, the intent is clear and explicitly declared through the code.
    ResultOwnedStr builtRes = strbuilder_get_string(&builder);

    // You do not have to compare the code to ERR_OK for it to achieve the same thing
    if (builtRes.error.code) {
        io_printerrln(r.error.msg);
        return 1;
    }

    OwnedStr built = builtRes.value;
    io_println(built);

    // We free the result string, since OwnedStr implies the consumer owns the
    // string, it is our responsibility to free it.
    // Strings that do not require freeing are passed as either ConstStr or String
    a->free(a, built);

    // Any call to x_init() should be followed by a call to x_deinit(),
    // in order to free the memory.
    strbuilder_deinit(&builder);
}
```

---

## 🧩 Something for everyone

- 🔍 Want to track leaks? Use the `DebugAllocator` to get peak memory stats and track unfreed blocks.
- 🥷 Building a DSL? Use `Writer` to generate source-like text output.
- 🚥 Parsing input? Use `string_split_lines`, `string_parse_float`, `string_trim_whitespace`.
- 💾 Need simple caching? Use `HashMapSetStrT` and free memory easily with `hashmap_deinit`.

---

## 🚫 What xstd isn't

- A bloated runtime
- A garbage collector
- A pthread-heavy concurrency abstraction
- A C++ STL polyfill

It’s **just** modern utilities — nothing intrusive.

---

## 🧪 Want to get started?

```c
#include "xstd_io.h"
#include "xstd_string.h"

int main(void)
{
    io_println("Hello from xstd!");

    ResultOwnedStr intStr = string_from_int(&c_allocator, -42);
    if (intStr.error.code) {
        io_printerrln(intStr.error.msg);
        return 1;
    }
    
    io_println(intStr.value);
    c_allocator.free(&c_allocator, intStr.value);
    return 0;
}
```

---

## 🧬 File Structure

| File | Description |
|------|-------------|
| `xstd_core.h` | Core types like `u8`, `i64`, `ibool`, `Buffer`, etc. |
| `xstd_alloc.h` | Allocator interface (`Allocator`, `alloc`) |
| `xstd_alloc_arena.h` | Arena (stack-like) allocator |
| `xstd_alloc_buffer.h` | Free-able buffer allocator |
| `xstd_alloc_debug.h` | Allocation-tracking wrapper |
| `xstd_io.h` | Terminal IO / assertions / prints |
| `xstd_file.h` | Cross-platform file reading & writing |
| `xstd_error.h` | Rich error handling |
| `xstd_string.h` | Safe strings & builders |
| `xstd_list.h` | Type-safe dynamic arrays |
| `xstd_hashmap.h` | Type-safe string-keyed hash maps |
| `xstd_writer.h` | Writers to buffer & string APIs |
| `xstd_math.h` | Overflow-safe math utilities |
| `xstd_result.h` | `Result<T>` structs |
| `xstd_vectors.h` | Vec2<T>, Vec3<T> structs |
| `xstd_process.h` | Crash signal handlers |

---

## 👷 Coming Soon

- JSON & text parsing
- Cross-platform filesystem APIs
- More allocator types (slab, fixed-pool)
- Tighter `xstd_string` codegen/writer integration
- Unit tests via `xstd_test.h`

---

## ❤️ Contributing

Want to help improve C ergonomics? Fork this project and improve one of the modules!

We love:
- Better documentation
- New allocator types
- Performance improvements
- Bug fixes
- Language bindings

Open an issue or PR and join us!

---

## 🧑‍💻 Who’s it for?

- C developers who love modern patterns
- Embedded engineers who need safety without runtime
- Compiler authors needing tooling
- Systems programmers who want better ergonomics without C++
- You. If you're reading this.

---

## 🔚 TL;DR

✅ Safer
✅ Faster
✅ Type-safe
✅ Debuggable
✅ Ergonomic
✅ Just C

> Goodbye `malloc()`, `fgets()`, `strcat()`.
> Hello `arena_allocator()`, `ResultOwnedStr`, `string_split_lines()` and `Writer`.

🧠 It's time to bring C to the modern age.
Try **xstd**.

---
👉 MIT licensed | 100% portable | C99+
Contributions welcome.

---
Projects making use of xstd:
- [Wade32 OS](https://github.com/wAIfu-DEV/Wade32)

---
🛠 Built with love for C ❤️

---
I'd like to thank ChatGPT for writing this README, really cool.
