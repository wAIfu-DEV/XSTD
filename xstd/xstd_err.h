#pragma once

#include "xstd_coretypes.h"

/**
 * @brief Represents the state of an operation. If err == ERR_OK then no issues
 * have been reported, however if err != ERR_OK then the operation has failed.
 *
 * You can get a string representation of the error by using `ErrorToString(err)`
 */
typedef u8 Error;

enum ErrorEnum
{
    ERR_OK,
    ERR_FAILED,
    ERR_UNAVAILABLE,
    ERR_UNAUTHORIZED,
    ERR_RANGE_ERROR,
    ERR_OUT_OF_MEMORY,
    ERR_FILE_NOT_FOUND,
    ERR_FILE_CANT_OPEN,
    ERR_FILE_CANT_WRITE,
    ERR_FILE_CANT_READ,
    ERR_TIMEOUT,
    ERR_CANT_CONNECT,
    ERR_CANT_RESOLVE,
    ERR_CONNECTION_ERROR,
    ERR_INVALID_PARAMETER,
    ERR_SKIP,
    ERR_WOULD_OVERFLOW,
    ERR_UNEXPECTED_BYTE,
    ERR_UNEXPECTED_ITEM,
    _ERR_MAX_SENTINEL,
};

const char *ErrorStrings[_ERR_MAX_SENTINEL] = {
    [ERR_OK] = "OK",
    [ERR_FAILED] = "FAILED",
    [ERR_UNAVAILABLE] = "UNAVAILABLE",
    [ERR_UNAUTHORIZED] = "UNAUTHORIZED",
    [ERR_RANGE_ERROR] = "RANGE ERROR",
    [ERR_OUT_OF_MEMORY] = "OUT OF MEMORY",
    [ERR_FILE_NOT_FOUND] = "FILE NOT FOUND",
    [ERR_FILE_CANT_OPEN] = "FILE CANT OPEN",
    [ERR_FILE_CANT_WRITE] = "FILE CANT WRITE",
    [ERR_FILE_CANT_READ] = "FILE CANT READ",
    [ERR_TIMEOUT] = "TIMEOUT",
    [ERR_CANT_CONNECT] = "CANT CONNECT",
    [ERR_CANT_RESOLVE] = "CANT RESOLVE",
    [ERR_CONNECTION_ERROR] = "CONNECTION ERROR",
    [ERR_INVALID_PARAMETER] = "INVALID PARAMETER",
    [ERR_WOULD_OVERFLOW] = "WOULD OVERFLOW",
    [ERR_UNEXPECTED_BYTE] = "UNEXPECTED BYTE",
    [ERR_UNEXPECTED_ITEM] = "UNEXPECTED ITEM",
};

#define ErrorToString(err) (((err) < _ERR_MAX_SENTINEL) ? ErrorStrings[(err)] : "UNKNOWN")
