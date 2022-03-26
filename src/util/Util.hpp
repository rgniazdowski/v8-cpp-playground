#pragma once
#ifndef FG_INC_UTIL_GENERIC
#define FG_INC_UTIL_GENERIC

#include <BuildConfig.hpp>
#include <string>
#include <filesystem>
#include <vector>

namespace util
{
    using DirEntriesVector = std::vector<std::filesystem::directory_entry>;
    using FileNamesVector = std::vector<std::string>;

    struct entry_to_path
    {
        std::string operator()(const std::filesystem::directory_entry &entry) const
        {
            return entry.path().string();
        }
    };

    void transform_entries_to_paths(const DirEntriesVector &input, FileNamesVector &vec);

    void read_directory(std::string_view name, FileNamesVector &vec);

    void read_directory(std::string_view name, DirEntriesVector &vec);

    template <class T, class Alloc = std::allocator<T>>
    bool contains(std::vector<T, Alloc> const &vec, T const &value)
    {
        using base_type = std::vector<T, Alloc>;
        for (typename base_type::const_iterator it = vec.begin(); it != vec.end(); it++)
        {
            if ((*it) == value)
                return true;
        }
        return false;
    }

    template <class T, class Alloc = std::allocator<T>>
    int find(std::vector<T, Alloc> const &vec, T const &value)
    {
        using vec_type = std::vector<T, Alloc>;
        int idx = 0;
        for (typename vec_type::const_iterator it = vec.begin(); it != vec.end(); it++, idx++)
        {
            if ((*it) == value)
                return idx;
        }
        return -1;
    }

    template <class T, class Alloc = std::allocator<T>>
    bool remove(std::vector<T, Alloc> &vec, typename std::vector<T, Alloc>::size_type index)
    {
        auto size = vec.size();
        if (index >= size)
            return false;
        vec.operator[](index) = vec.operator[](size - 1);
        vec.resize(size - 1);
        return true;
    }

    template <class T, class Alloc = std::allocator<T>>
    std::vector<T, Alloc> reverse(std::vector<T, Alloc> const &vec)
    {
        using vec_type = std::vector<T, Alloc>;
        vec_type out;
        typename self_type::reverse_iterator b = vec.rbegin();
        typename self_type::reverse_iterator e = vec.rend();
        for (; b != e; b++)
            out.push_back(*b);
        return out;
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

    void replaceAll(std::string &source, std::string_view needle, std::string_view replacer);
    void replaceAll(std::string &source, const std::vector<std::string_view> &needlePairs);

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