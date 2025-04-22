#pragma once

#include "xstd_os_int.h"

#include "signal.h"

const _OsInterface __os_int = {
    ._internalState = NULL,
    .signal = signal,
};
