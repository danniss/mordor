// Copyright (c) 2009 - Decho Corp.

#include "mordor/pch.h"

#include "std.h"

#include "mordor/exception.h"

namespace Mordor {

StdinStream::StdinStream()
{
#ifdef WINDOWS
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdIn == INVALID_HANDLE_VALUE) {
        MORDOR_THROW_EXCEPTION_FROM_LAST_ERROR_API("GetStdHandle");
    }
    if (hStdIn == NULL) {
        MORDOR_THROW_EXCEPTION_FROM_ERROR_API(ERROR_FILE_NOT_FOUND, "GetStdHandle");
    }
    init(hStdIn, false);
#else
    init(STDIN_FILENO, false);
#endif
}

StdinStream::StdinStream(IOManager &ioManager)
{
#ifdef WINDOWS
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdIn == INVALID_HANDLE_VALUE) {
        MORDOR_THROW_EXCEPTION_FROM_LAST_ERROR_API("GetStdHandle");
    }
    if (hStdIn == NULL) {
        MORDOR_THROW_EXCEPTION_FROM_ERROR_API(ERROR_FILE_NOT_FOUND, "GetStdHandle");
    }
    init(&ioManager, hStdIn, false);
#else
    init(&ioManager, STDIN_FILENO, false);
#endif
}

StdoutStream::StdoutStream()
{
#ifdef WINDOWS
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdOut == INVALID_HANDLE_VALUE) {
        MORDOR_THROW_EXCEPTION_FROM_LAST_ERROR_API("GetStdHandle");
    }
    if (hStdOut == NULL) {
        MORDOR_THROW_EXCEPTION_FROM_ERROR_API(ERROR_FILE_NOT_FOUND, "GetStdHandle");
    }
    init(hStdOut, false);
#else
    init(STDOUT_FILENO, false);
#endif
}

StdoutStream::StdoutStream(IOManager &ioManager)
{
#ifdef WINDOWS
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdOut == INVALID_HANDLE_VALUE) {
        MORDOR_THROW_EXCEPTION_FROM_LAST_ERROR_API("GetStdHandle");
    }
    if (hStdOut == NULL) {
        MORDOR_THROW_EXCEPTION_FROM_ERROR_API(ERROR_FILE_NOT_FOUND, "GetStdHandle");
    }
    init(&ioManager, hStdOut, false);
#else
    init(&ioManager, STDOUT_FILENO, false);
#endif
}

StderrStream::StderrStream()
{
#ifdef WINDOWS
    HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
    if (hStdErr == INVALID_HANDLE_VALUE) {
        MORDOR_THROW_EXCEPTION_FROM_LAST_ERROR_API("GetStdHandle");
    }
    if (hStdErr == NULL) {
        MORDOR_THROW_EXCEPTION_FROM_ERROR_API(ERROR_FILE_NOT_FOUND, "GetStdHandle");
    }
    init(hStdErr, false);
#else
    init(STDERR_FILENO, false);
#endif
}

StderrStream::StderrStream(IOManager &ioManager)
{
#ifdef WINDOWS
    HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
    if (hStdErr == INVALID_HANDLE_VALUE) {
        MORDOR_THROW_EXCEPTION_FROM_LAST_ERROR_API("GetStdHandle");
    }
    if (hStdErr == NULL) {
        MORDOR_THROW_EXCEPTION_FROM_ERROR_API(ERROR_FILE_NOT_FOUND, "GetStdHandle");
    }
    init(&ioManager, hStdErr, false);
#else
    init(&ioManager, STDERR_FILENO, false);
#endif
}

}