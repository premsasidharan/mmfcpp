/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <media_logger.h>

Media_logger::Media_logger()
{
#if defined(MEDIA_TRACE_FILE) || defined(MEDIA_TRACE_CONSOLE)
    set_log_file_path(TRACE_FILE_PATH);
#endif
}

Media_logger& Media_logger::instance()
{
    static Media_logger logger;
    return logger;
}

void Media_logger::set_log_file_path(const char* _path)
{
    if (0 != _path)
    {
        int length = strlen(_path);
        if (length < MAX_CHARS_LOG_FILE_PATH)
        {
            strncpy(path, _path, length);
            FILE* file = fopen(path, "w");
            if (0 == file)
            {
                fprintf(stderr, "Failed to create log file: %s", path);
            }
            else
            {
                fclose(file);
            }
        }
        else
        {
            fprintf(stderr, "Log file path size: %d exceeds max size: %d", length, MAX_CHARS_LOG_FILE_PATH);
        }
    }
    else
    {
        fprintf(stderr, "Invalid Log file path: NULL");
    }
}

void Media_logger::log(Media_logger::Level level, const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);

    switch (level)
    {
    case Media_logger::console:
        vfprintf(stderr, fmt, argp);
        fprintf(stderr, "\n");
        fflush(stderr);
        break;

    case Media_logger::file:
    {
        FILE* file = fopen(path, "a+");
        if (0 == file)
        {
            fprintf(stderr, "File logging failed at %s", path);
        }
        else
        {
            vfprintf(file, fmt, argp);
            fprintf(file, "\n");
            fclose(file);
        }
    }
    break;
    }
    va_end(argp);
}
