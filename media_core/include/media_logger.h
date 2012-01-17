/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _MEDIA_LOGGER_H
#define _MEDIA_LOGGER_H

#define TRACE_FILE_PATH "test.log"
#define MAX_CHARS_LOG_FILE_PATH 1024

class Media_logger
{
public:
    enum Level {console, file};
    ~Media_logger() {};

    static Media_logger& instance();
    void log(Level _level, const char *fmt, ...);

private:
    Media_logger();
    Media_logger(const Media_logger& logger) {};
    Media_logger operator=(const Media_logger& logger) { return *this; };
    void set_log_file_path(const char* _path);

private:
    char path[MAX_CHARS_LOG_FILE_PATH];
};

#ifdef MEDIA_TRACE_CONSOLE
#define SET_LOG_FILE_PATH(path) ((void)0)
#define MEDIA_LOG(fmt, ...) Media_logger::instance().log(Media_logger::console, fmt, __VA_ARGS__)
#elif MEDIA_TRACE_FILE
#define SET_LOG_FILE_PATH(path) Media_logger::instance().set_log_file_path(path)
#define MEDIA_LOG(fmt, ...) Media_logger::instance().log(Media_logger::file, fmt, __VA_ARGS__)
#else
#define SET_LOG_FILE_PATH(path) ((void)0)
#define MEDIA_LOG(fmt, ...) ((void)0)
#endif

#endif
