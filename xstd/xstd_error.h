#pragma once

#include "xstd_core.h"
#include "xstd_errcode.h"

typedef struct _xstd_err
{
    ConstStr msg;
    ErrorCode code;
} Error;

typedef struct _xstd_err_owned
{
    OwnedStr msgOwned;
    ErrorCode code;
} ErrorOwnedStr;

#define X_ERR_OK \
    (Error){ .msg = NULL, .code = ERR_OK }

#define X_ERR_FROM(codeLiteral, messageLiteral) \
    (Error){ .msg = (messageLiteral), .code = (codeLiteral) }

#define _X_ERR_MSG(moduleName, nameSpace, messageLiteral) \
    moduleName "." nameSpace ": " messageLiteral

#define X_ERR_EXT(moduleName, nameSpace, codeLiteral, messageLiteral) \
    (Error){ .msg = _X_ERR_MSG(moduleName, nameSpace, messageLiteral), .code = (codeLiteral) }
