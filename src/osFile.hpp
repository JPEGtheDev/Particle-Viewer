/*
 * osFile.hpp
 *
 * Sets up OS Specific path data.
 *
 */

#ifndef osFile_HPP
#define osFile_HPP

#include <string>
#include <iostream>

#ifdef _WIN32
    #include <windows.h>
#elif __APPLE__
    #include <mach-o/dyld.h>
#elif __linux__
    #include <limits.h>
    #include <unistd.h>
#endif
static std::string ExePath()
{
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos);
#elif __APPLE__
    char path[1024];
    uint32_t size = sizeof(path);
    _NSGetExecutablePath(path, &size);
    std::string::size_type pos = std::string(path).find_last_of(".") - 1;
    return std::string(path).substr(0, pos);
#elif __linux__
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    std::string::size_type pos = std::string(result).find_last_of("\\/");
    return std::string(result).substr(0, pos);
        // return std::string( result, (count > 0) ? count : 0 );
#endif
}
#endif