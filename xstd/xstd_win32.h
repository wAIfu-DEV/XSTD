#pragma once

#ifdef _X_PLAT_WIN

#if _MSC_VER
    #pragma comment(lib, "Shell32.lib")
#endif

typedef int _w32_bool;
typedef unsigned long _w32_dword;
typedef unsigned int _w32_uint;
typedef unsigned long long _w32_size_t;
typedef unsigned long long _w32_ulong64;

typedef void* _w32_handle;
typedef void* _w32_lpvoid;
typedef char* _w32_lpstr;
typedef const char* _w32_lpcstr;
typedef short* _w32_lpwstr;
typedef const short* _w32_lpcwstr;

typedef struct _w32_filetime
{
    _w32_dword dwLowDateTime;
    _w32_dword dwHighDateTime;
} _w32_filetime;

typedef _w32_filetime* _w32_lpfiletime;

#define _WIN_32_GENERIC_READ  (0x80000000L)
#define _WIN_32_GENERIC_WRITE (0x40000000L)
#define _WIN_32_OPEN_EXISTING 3
#define _WIN_32_CREATE_ALWAYS 2

#define _WIN_32_FILE_BEGIN 0
#define _WIN_32_FILE_CURRENT 1
#define _WIN_32_FILE_END 2

#define _WIN_32_STD_INPUT_HANDLE  ((_w32_dword)-10)
#define _WIN_32_STD_OUTPUT_HANDLE ((_w32_dword)-11)
#define _WIN_32_STD_ERROR_HANDLE  ((_w32_dword)-12)

#define _WIN_32_INVALID_HANDLE_VALUE ((_w32_handle)(long long)-1)

// windows API dll imports
// better than importing the entire windows.h lib
// at least for the sake of global scope pollution

__declspec(dllimport) _w32_handle  __stdcall GetProcessHeap(void);
__declspec(dllimport) _w32_lpvoid  __stdcall HeapAlloc(_w32_handle, _w32_dword, _w32_size_t);
__declspec(dllimport) _w32_lpvoid  __stdcall HeapReAlloc(_w32_handle, _w32_dword, _w32_lpvoid, _w32_size_t);
__declspec(dllimport) _w32_bool    __stdcall HeapFree(_w32_handle, _w32_dword, _w32_lpvoid);

__declspec(dllimport) _w32_handle  __stdcall GetStdHandle(_w32_dword);
__declspec(dllimport) _w32_bool    __stdcall FlushFileBuffers(_w32_handle);
__declspec(dllimport) _w32_handle  __stdcall CreateFileW(short*, _w32_dword, _w32_dword, void*, _w32_dword, _w32_dword, _w32_handle);
__declspec(dllimport) _w32_bool    __stdcall CloseHandle(_w32_handle);
__declspec(dllimport) _w32_bool    __stdcall ReadFile(_w32_handle, void*, _w32_dword, _w32_dword*, void*);
__declspec(dllimport) _w32_bool    __stdcall WriteFile(_w32_handle, const void*, _w32_dword, _w32_dword*, void*);
__declspec(dllimport) _w32_bool    __stdcall SetFilePointerEx(_w32_handle, long long, long long*, _w32_dword);
__declspec(dllimport) _w32_bool    __stdcall SetConsoleOutputCP(_w32_uint);
__declspec(dllimport) _w32_lpwstr  __stdcall GetCommandLineW(void);
__declspec(dllimport) _w32_lpwstr* __stdcall CommandLineToArgvW(_w32_lpcwstr, int*);

__declspec(dllimport) void        __stdcall GetSystemTimeAsFileTime(_w32_lpfiletime);

__declspec(dllimport) void        __stdcall ExitProcess(_w32_uint);

#endif
