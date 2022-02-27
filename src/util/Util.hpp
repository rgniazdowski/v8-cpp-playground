#pragma once
#ifndef FG_INC_UTIL_GENERIC
#define FG_INC_UTIL_GENERIC

#include <BuildConfig.hpp>
#include <string>
#include <filesystem>
#include <vector>

namespace util
{
    struct path_string
    {
        std::string operator()(const std::filesystem::directory_entry &entry) const
        {
            return entry.path().string();
        }
    };

    void read_directory(std::string_view name, std::vector<std::string> &vec)
    {
        std::filesystem::path path(name);
        std::filesystem::directory_iterator start(path);
        std::filesystem::directory_iterator end;
        std::transform(start, end, std::back_inserter(vec), path_string());
    }
} //> namespace util

namespace strings
{
    void toLower(std::string &output, const std::string &input);
    std::string toLower(const std::string &input);
    void toLower(char *output, const char *input);
    char *toLower(const char *input);

    void toUpper(std::string &output, const std::string &input);
    std::string toUpper(const std::string &input);
    void toUpper(char *output, const char *input);
    char *toUpper(const char *input);

    std::string trim(const std::string &str, const std::string &whitespace = " \t\r");

    std::string trim(const std::string &str, const char *whitespace);

    std::string trim(const char *str, const char *whitespace);

    std::string reduce(const std::string &str,
                       const std::string &fill = " ",
                       const std::string &whitespace = " \t\r");

    std::vector<std::string> &split(const std::string &input, char delim,
                                    std::vector<std::string> &elems);

    std::vector<std::string> split(const std::string &input, char delim);

    bool isFloat(const std::string &string);
    bool isNumber(const std::string &string);

    bool equals(const std::string &input, const std::string &pattern, const bool caseSensitive = true);

    bool equals(const char *input, const char *pattern, const bool caseSensitive = true);

    bool startsWith(const std::string &input, const std::string &pattern, const bool caseSensitive = true);

    bool startsWith(const char *input, const char *pattern, bool caseSensitive = true);

    bool endsWith(const std::string &input, const std::string &pattern, bool caseSensitive = true);

    bool endsWith(const char *input, const char *pattern, bool caseSensitive = true);

    bool containsChars(const std::string &input, const std::string &chars);
    bool containsChars(std::string_view input, std::string_view chars);
    bool containsChars(const char *input, const char *chars, const bool caseSensitive = true);

    const char *strstr(const char *str, const char *needle);
    const char *strstr(std::string_view str, std::string_view needle);

    const char *stristr(const char *str, const char *needle);
    const char *stristr(std::string_view str, std::string_view needle);
} //> namespace strings

#if defined(FG_USING_PLATFORM_WINDOWS)
#define FG_PATH_DELIM "\\"
#define FG_PATH_DELIMC '\\'
#define FG_PATH_DELIM2 "/"
#define FG_PATH_DELIM2C '/'
#else
#define FG_PATH_DELIM "/"
#define FG_PATH_DELIMC '/'
#define FG_PATH_DELIM2 "\\"
#define FG_PATH_DELIM2C '\\'
#endif

namespace path
{
    const char *const DELIMITER = FG_PATH_DELIM;
    const char DELIMITER_CHAR = FG_PATH_DELIMC;

    const char *getAssetsPath(void);

    bool changeCurrentWorkingDir(const char *newPath);
    bool changeCurrentWorkingDir(const std::string &newPath);

    char *getCurrentWorkingPath(char *buffer, size_t maxlen);

    std::string getCurrentWorkingPath(void);

    void getCurrentWorkingPath(std::string &output_path);

    const char *fileExt(const char *path, bool fullExt = false);
    const char *fileName(const char *path);

    void dirName(char *path);
    char *dirName(const char *path);
    std::string dirName(std::string &path);
    std::string &dirName(std::string &path, std::string &dir);

    void split(std::string &path, std::string &dirpath, std::string &filename);

    std::string &join(std::string &path, const std::string &dirpath, const std::string &filename);
    std::string join(const std::string &dirpath, const std::string &filename);
    void join(std::string &path, std::vector<std::string> &parts);
} //> namespace path

namespace util
{
    template <class T>
    struct PtrLess
    {
        using TT = std::remove_pointer_t<T>;
        inline bool operator()(TT *left, TT *right) { return ((*left) < (*right)); }
    };

    template <class T>
    struct PtrGreater
    {
        using TT = std::remove_pointer_t<T>;
        inline bool operator()(TT *left, TT *right) { return !((*left) < (*right)); }
    };

    template <class T>
    struct PtrLessEq
    {
        using TT = std::remove_pointer_t<T>;
        inline bool operator()(TT *left, TT *right) { return ((*left) <= (*right)); }
    };

    template <class T>
    struct PtrGreaterEq
    {
        using TT = std::remove_pointer_t<T>;
        inline bool operator()(TT *left, TT *right) { return !((*left) <= (*right)); }
    };

    template <class T>
    struct Less
    {
        inline bool operator()(const T &left, const T &right) { return ((left) < (right)); }
    };

    template <class T>
    struct Greater
    {
        inline bool operator()(const T &left, const T &right) { return !((left) < (right)); }
    };

    template <class T>
    struct LessEq
    {
        inline bool operator()(const T &left, const T &right) { return ((left) <= (right)); }
    };

    template <class T>
    struct GreaterEq
    {
        inline bool operator()(const T &left, const T &right) { return !((left) <= (right)); }
    };

}; //> namespace util

#endif //> FG_INC_UTIL_GENERIC