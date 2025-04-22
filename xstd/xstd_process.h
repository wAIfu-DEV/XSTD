#pragma once

#include "xstd_os_int.h"
#include "xstd_os_int_stdlib.h"
#include "xstd_io.h"

const char *SignalStrings[23] = {
    [SIGABRT] = "SIGABRT - Termination after abort() call.",
    [SIGFPE] = "SIGFPE - Termination after floating point arithmetic exception.",
    [SIGILL] = "SIGILL - Termination after call to illegal instruction.",
    [SIGINT] = "SIGINT - Termination after user interrupt.",
    [SIGSEGV] = "SIGSEGV - Termination after segmentation fault.",
    [SIGTERM] = "SIGTERM - Termination after request from OS.",
};

#define SignalToString(err) (((err) < 23) ? SignalStrings[(err)] : "UNKNOWN")

void process_setup_crash_handler(void (*handler)(i32 sigNum))
{
    __os_int.signal(SIGABRT, handler);
    __os_int.signal(SIGFPE, handler);
    __os_int.signal(SIGILL, handler);
    __os_int.signal(SIGINT, handler);
    __os_int.signal(SIGSEGV, handler);
    __os_int.signal(SIGTERM, handler);
}

void __process_default_sig_handler(i32 sigNum)
{
    crash_print(SignalToString(sigNum), 1);
}

void process_setup_default_crash_handler(void)
{
    __os_int.signal(SIGABRT, __process_default_sig_handler);
    __os_int.signal(SIGFPE, __process_default_sig_handler);
    __os_int.signal(SIGILL, __process_default_sig_handler);
    __os_int.signal(SIGINT, __process_default_sig_handler);
    __os_int.signal(SIGSEGV, __process_default_sig_handler);
    __os_int.signal(SIGTERM, __process_default_sig_handler);
}
