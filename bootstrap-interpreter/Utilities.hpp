#ifndef SYSMEL_UTILITIES_HPP
#define SYSMEL_UTILITIES_HPP

#pragma once

#include <string>
#include <utility>
#include <fstream>

namespace Sysmel
{
    std::pair<std::string, std::string> splitPath(const std::string &path)
    {
        auto unixPathSeparator = path.rfind('/');
        auto windowsPathSeparator = path.rfind('\\');
        auto pathSeparator = 0;

        if (unixPathSeparator != std::string::npos && windowsPathSeparator != std::string::npos)
            pathSeparator = std::max(unixPathSeparator, windowsPathSeparator);
        else if (unixPathSeparator != std::string::npos)
            pathSeparator = unixPathSeparator;
        else if (windowsPathSeparator != std::string::npos)
            pathSeparator = windowsPathSeparator;

        return std::make_pair(path.substr(0, pathSeparator), path.substr(pathSeparator + 1));
    }

    std::string joinPath(const std::string &directory, const std::string &basename)
    {
        return directory + "/" + basename;
    }

    std::string readWholeTextFile(const std::string &filename)
    {
        std::ifstream in(filename);
        if(!in.good())
        {
            fprintf(stderr, "Failed to open input file '%s'\n", filename.c_str());
            return std::string();
        }

        return std::string { std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>() };
    }
}
#endif //SYSMEL_UTILITIES_HPP