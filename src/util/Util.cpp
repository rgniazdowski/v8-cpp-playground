#include <util/Util.hpp>
#include <Unistd.hpp>

#include <sstream>

void util::transform_entries_to_paths(const DirEntriesVector &input, FileNamesVector &vec)
{
    std::transform(input.begin(), input.end(), std::back_inserter(vec), entry_to_path());
}
//>---------------------------------------------------------------------------------------

void util::read_directory(std::string_view name, FileNamesVector &vec)
{
    std::filesystem::path path(name);
    std::filesystem::directory_iterator start(path);
    std::filesystem::directory_iterator end;
    std::transform(start, end, std::back_inserter(vec), entry_to_path());
}
//>---------------------------------------------------------------------------------------

void util::read_directory(std::string_view name, DirEntriesVector &vec)
{
    std::filesystem::path path(name);
    std::filesystem::directory_iterator start(path);
    std::filesystem::directory_iterator end;
    for (; start != end; start++)
        vec.push_back(*start);
}
//>---------------------------------------------------------------------------------------

void strings::toLower(std::string &output, const std::string &input)
{
    if (input.empty())
        return;
    output.resize(input.length(), '\0');
    auto n = input.length();
    for (unsigned int i = 0; i < n; i++)
    {
        output[i] = tolower(input[i]);
    }
}
//>---------------------------------------------------------------------------------------

std::string strings::toLower(const std::string &input)
{
    std::string output;
    if (input.empty())
        return output;
    output.resize(input.length(), '\0');
    auto n = input.length();
    for (unsigned int i = 0; i < n; i++)
    {
        output[i] = tolower(input[i]);
    }
    return output;
}
//>---------------------------------------------------------------------------------------

void strings::toLower(char *output, const char *input)
{
    if (!output || !input)
        return;
    auto n = strlen(input);
    if (!n)
        return;
    for (unsigned int i = 0; i < n; i++)
    {
        output[i] = tolower(input[i]);
    }
}
//>---------------------------------------------------------------------------------------

char *strings::toLower(const char *input)
{
    if (!input)
        return nullptr;
    auto n = strlen(input);
    char *output = _strdup(input);
    if (!output)
        return nullptr;
    for (unsigned int i = 0; i < n; i++)
    {
        output[i] = tolower(input[i]);
    }
    return output;
}
//>---------------------------------------------------------------------------------------

void strings::toUpper(std::string &output, const std::string &input)
{
    if (input.empty())
        return;
    output.resize(input.length(), '\0');
    auto n = input.length();
    for (unsigned int i = 0; i < n; i++)
    {
        output[i] = toupper(input[i]);
    }
}
//>---------------------------------------------------------------------------------------

std::string strings::toUpper(const std::string &input)
{
    std::string output;
    if (input.empty())
        return output;
    output.resize(input.length(), '\0');
    auto n = input.length();
    for (unsigned int i = 0; i < n; i++)
    {
        output[i] = toupper(input[i]);
    }
    return output;
}
//>---------------------------------------------------------------------------------------

void strings::toUpper(char *output, const char *input)
{
    if (!output || !input)
        return;
    auto n = strlen(input);
    if (!n)
        return;
    for (unsigned int i = 0; i < n; i++)
    {
        output[i] = toupper(input[i]);
    }
}
//>---------------------------------------------------------------------------------------

char *strings::toUpper(const char *input)
{
    if (!input)
        return nullptr;
    auto n = strlen(input);
    char *output = _strdup(input);
    if (!output)
        return nullptr;
    for (unsigned int i = 0; i < n; i++)
    {
        output[i] = toupper(input[i]);
    }
    return output;
}
//>---------------------------------------------------------------------------------------

std::string strings::trim(const std::string &str,
                          const std::string &whitespace)
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos || strBegin > str.length())
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}
//>---------------------------------------------------------------------------------------

std::string strings::trim(const std::string &str,
                          const char *whitespace)
{
    std::string whitespaceStr;
    if (!whitespace)
        whitespaceStr = " \t";
    else
        whitespaceStr = whitespace;
    return strings::trim(str, whitespaceStr);
}
//>---------------------------------------------------------------------------------------

std::string strings::trim(const char *str, const char *whitespace)
{
    if (!str)
        return std::string();
    std::string whitespaceStr;
    if (!whitespace)
        whitespaceStr = " \t";
    else
        whitespaceStr = whitespace;
    return strings::trim(std::string(str), whitespaceStr);
}
//>---------------------------------------------------------------------------------------

std::string strings::reduce(const std::string &str, const std::string &fill, const std::string &whitespace)
{
    // trim first
    std::string result = strings::trim(str, whitespace);
    // replace sub ranges
    auto beginSpace = result.find_first_of(whitespace);
    while (beginSpace != std::string::npos)
    {
        const auto endSpace = result.find_first_not_of(whitespace, beginSpace);
        const auto range = endSpace - beginSpace;

        result.replace(beginSpace, range, fill);

        const auto newStart = beginSpace + fill.length();
        beginSpace = result.find_first_of(whitespace, newStart);
    }
    return result;
}
//>---------------------------------------------------------------------------------------

std::vector<std::string> &strings::split(const std::string &input, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(input);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        if (!item.empty())
            elems.push_back(item);
    }
    return elems;
}
//>---------------------------------------------------------------------------------------

std::vector<std::string> strings::split(const std::string &input, char delim)
{
    std::vector<std::string> elems;
    strings::split(input, delim, elems);
    return elems;
}
//>---------------------------------------------------------------------------------------

bool strings::isFloat(const std::string &string)
{
    std::string::const_iterator it = string.begin();
    bool decimalPoint = false;
    int minSize = 0;
    if (string.size() > 0 && (string[0] == '-' || string[0] == '+'))
    {
        it++;
        minSize++;
    }
    //
    // If you don't want to recognize floating point numbers in
    // the format X.XXf, just remove the condition:
    // && ((*it!='f') || it+1 != string.end() || !decimalPoint)
    //
    // If you don't want to recognize numbers without '.' as
    // float (i.e. not '1', only '1.', '1.0', '1.0f'...)
    // then change the last line to:
    // return string.size()>minSize && it == string.end() && decimalPoint;
    //
    while (it != string.end())
    {
        if (*it == '.')
        {
            if (!decimalPoint)
                decimalPoint = true;
            else
                break;
        }
        else if (!std::isdigit(*it) && (it + 1 != string.end() || !decimalPoint))
            break;
        ++it;
    }
    return (((int)string.size() > minSize) && it == string.end() && decimalPoint);
}
//>---------------------------------------------------------------------------------------

bool strings::isNumber(const std::string &string)
{
    std::string::const_iterator it = string.begin();
    bool decimalPoint = false;
    int minSize = 0;
    if (string.size() > 0 && (string[0] == '-' || string[0] == '+'))
    {
        it++;
        minSize++;
    }
    while (it != string.end())
    {
        if (*it == '.')
        {
            if (!decimalPoint)
                decimalPoint = true;
            else
                break;
        }
        else if (!std::isdigit(*it))
            break;
        ++it;
    }
    return (((int)string.size() > minSize) && it == string.end());
}
//>---------------------------------------------------------------------------------------

bool strings::equals(const std::string &input, const std::string &pattern, const bool caseSensitive)
{
    if (pattern.length() != input.length())
        return false;
    return strings::startsWith(input, pattern, caseSensitive);
}
//>---------------------------------------------------------------------------------------

bool strings::equals(const char *input, const char *pattern, const bool caseSensitive)
{
    if (strlen(pattern) != strlen(input))
        return false;
    return strings::startsWith(input, pattern, caseSensitive);
}
//>---------------------------------------------------------------------------------------

bool strings::startsWith(const std::string &input, const std::string &pattern, const bool caseSensitive)
{
    if (input.length() < pattern.length() || pattern.empty() || input.empty())
        return false;
    auto plen = pattern.length();
    for (int i = 0; i < plen; i++)
    {
        if ((caseSensitive && input[i] != pattern[i]) ||
            (!caseSensitive && tolower(input[i]) != tolower(pattern[i])))
            return false;
    }
    return true;
}
//>---------------------------------------------------------------------------------------

bool strings::startsWith(const char *input, const char *pattern, bool caseSensitive)
{
    if (!input || !pattern)
        return false;
    auto plen = strlen(pattern);
    auto ilen = strlen(input);
    if (ilen < plen || !ilen || !plen)
        return false;
    for (int i = 0; i < plen; i++)
    {
        if ((caseSensitive && input[i] != pattern[i]) ||
            (!caseSensitive && tolower(input[i]) != tolower(pattern[i])))
            return false;
    }
    return true;
}
//>---------------------------------------------------------------------------------------

bool strings::endsWith(const std::string &input, const std::string &pattern, bool caseSensitive)
{
    if (input.length() < pattern.length() || pattern.empty() || input.empty())
        return false;
    auto plen = pattern.length();
    auto ilen = input.length();
    for (auto i = ilen - 1, p = plen - 1; i >= 0 && p >= 0; i--, p--)
    {
        if ((caseSensitive && input[i] != pattern[p]) ||
            (!caseSensitive && tolower(input[i]) != tolower(pattern[p])))
            return false;
    }
    return true;
}
//>---------------------------------------------------------------------------------------

bool strings::endsWith(const char *input, const char *pattern, bool caseSensitive)
{
    if (!input || !pattern)
        return false;
    auto plen = strlen(pattern);
    auto ilen = strlen(input);
    if (ilen < plen || !plen || !ilen)
        return false;
    for (auto i = ilen - 1, p = plen - 1; i >= 0 && p >= 0; i--, p--)
    {
        if ((caseSensitive && input[i] != pattern[p]) ||
            (!caseSensitive && tolower(input[i]) != tolower(pattern[p])))
        {
            return false;
        }
    }
    return true;
}
//>---------------------------------------------------------------------------------------

bool strings::containsChars(const std::string &input, const std::string &chars)
{
    return (input.find_first_of(chars) != std::string::npos);
}
//>---------------------------------------------------------------------------------------

bool strings::containsChars(std::string_view input, std::string_view chars)
{
    return containsChars(input.data(), chars.data());
}
//>---------------------------------------------------------------------------------------

bool strings::containsChars(const char *input, const char *chars, const bool caseSensitive)
{
    if (!input || !chars)
        return false;
    auto i = 0, j = 0;
    auto ilen = strlen(input), clen = strlen(chars);
    if (!ilen || !clen)
        return false;
    for (; i < ilen; i++)
    {
        for (j = 0; j < clen; j++)
        {
            if ((caseSensitive && input[i] == chars[j]) ||
                (!caseSensitive && tolower(input[i]) == tolower(chars[j])))
            {
                return true;
            }
        }
    }
    return false;
}
//>---------------------------------------------------------------------------------------

const char *strings::strstr(const char *str, const char *needle)
{
    if (!needle || !*needle || !str)
        return str;
    char *p1 = (char *)str, *p2 = (char *)needle;
    char *p1Adv = (char *)str;
    while (*++p2)
        p1Adv++;
    while (*p1Adv)
    {
        char *p1Begin = p1;
        p2 = (char *)needle;
        while (*p1 && *p2 && *p1 == *p2)
        {
            p1++;
            p2++;
        }
        if (!*p2)
            return p1Begin;
        p1 = p1Begin + 1;
        p1Adv++;
    }
    return nullptr;
}
//>---------------------------------------------------------------------------------------

const char *strings::strstr(std::string_view str, std::string_view needle)
{
    if (str.empty() || needle.empty())
        return nullptr;
    return strings::strstr(str.data(), needle.data());
}
//>---------------------------------------------------------------------------------------

const char *strings::stristr(const char *str, const char *needle)
{
    if (!needle || !*needle || !str)
        return str;
    char *p1 = (char *)str, *p2 = (char *)needle;
    char *p1Adv = (char *)str;
    while (*++p2)
        p1Adv++;
    while (*p1Adv)
    {
        char *p1Begin = p1;
        p2 = (char *)needle;
        while (*p1 && *p2 && (tolower(*p1) == tolower(*p2)))
        {
            p1++;
            p2++;
        }
        if (!*p2)
            return p1Begin;
        p1 = p1Begin + 1;
        p1Adv++;
    }
    return nullptr;
}
//>---------------------------------------------------------------------------------------

const char *strings::stristr(std::string_view str, std::string_view needle)
{
    if (str.empty() || needle.empty())
        return nullptr;
    return strings::stristr(str.data(), needle.data());
}
//>---------------------------------------------------------------------------------------

const char *path::getAssetsPath(void)
{
#if defined(FG_USING_PLATFORM_ANDROID)
    return "\0\0";
#elif defined(FG_USING_PLATFORM_WINDOWS)
    return ".\\";
#elif defined(FG_USING_PLATFORM_LINUX)
    return "./";
#endif
}
//>---------------------------------------------------------------------------------------

bool path::changeCurrentWorkingDir(const char *newPath)
{
    if (!newPath)
        return false;
    return chdir(newPath) < 0 ? false : true;
}
//>---------------------------------------------------------------------------------------

bool path::changeCurrentWorkingDir(const std::string &newPath)
{
    if (newPath.empty())
        return false;
    return chdir(newPath.c_str()) < 0 ? false : true;
}
//>---------------------------------------------------------------------------------------

char *path::getCurrentWorkingPath(char *buffer, size_t maxlen)
{
    if (!buffer)
        return nullptr;
    if (maxlen < 8)
        return nullptr;
    getcwd(buffer, static_cast<int>(maxlen));
    return buffer;
}
//>---------------------------------------------------------------------------------------

std::string path::getCurrentWorkingPath(void)
{
    std::string curdir;
    getCurrentWorkingPath(curdir);
    return curdir;
}
//>---------------------------------------------------------------------------------------

void path::getCurrentWorkingPath(std::string &output_path)
{
    output_path.clear();
    char buf[2048];
    if (getcwd(buf, 2048))
    {
        output_path.append(buf);
    }
}
//>---------------------------------------------------------------------------------------

const char *path::fileExt(const char *path, bool fullExt)
{
    if (!path)
        return nullptr;
    path = path::fileName(path);
    const char *dot = nullptr;
    if (fullExt == true)
        dot = strchr(path, '.');
    else
        dot = strrchr(path, '.');
    if (!dot || dot == path)
        return nullptr;
    return dot + 1;
}
//>---------------------------------------------------------------------------------------

const char *path::fileName(const char *path)
{
    if (!path)
        return nullptr;
    return (strrchr(path, '/') ? strrchr(path, '/') + 1 : strrchr(path, '\\') ? strrchr(path, '\\') + 1
                                                                              : path);
}
//>---------------------------------------------------------------------------------------

void path::dirName(char *path)
{
    const char *filename = path::fileName(path);
    if (!filename)
        return;
    auto npath = strlen(path);
    auto nfile = strlen(filename);
    auto pos = npath - nfile;
    for (auto i = pos; i < npath; i++)
        path[i] = 0;
}
//>---------------------------------------------------------------------------------------

char *path::dirName(const char *path)
{
    const char *filename = path::fileName(path);
    if (!filename)
        return nullptr;
    auto npath = strlen(path);
    auto nfile = strlen(filename);
    auto newlen = npath - nfile;
    // char *buf = fgMalloc<char>(newlen + 1);
    char *buf = new char[newlen + 1];
    memset(buf, 0, newlen + 1);
    strncpy_s(buf, newlen, path, newlen);
    buf[newlen] = 0;
    return buf;
}
//>---------------------------------------------------------------------------------------

std::string path::dirName(std::string &path)
{
    const char *filename = path::fileName(path.c_str());
    if (!filename)
        return path;
    return path.substr(0, path.length() - strlen(filename));
}
//>---------------------------------------------------------------------------------------

std::string &path::dirName(std::string &path, std::string &dirpath)
{
    dirpath.clear();
    dirpath.append(dirName(path));
    return dirpath;
}
//>---------------------------------------------------------------------------------------

void path::split(std::string &path, std::string &dirpath, std::string &filename)
{
    dirpath = dirName(path);
    const char *filename_c = path::fileName(path.c_str());
    if (!filename_c)
        filename.clear();
    else
        filename = filename_c;
}
//>---------------------------------------------------------------------------------------

std::string &path::join(std::string &targetpath,
                        const std::string &dirpath,
                        const std::string &filename)
{
    // path - here is the result stored

    auto dirlen = dirpath.length();
    auto filelen = filename.length();
    std::string tmp;
    tmp.reserve(dirlen + filelen + 2);
    tmp.append(dirpath);
    if (dirpath[dirlen - 1] == FG_PATH_DELIMC || tmp[dirlen - 1] == FG_PATH_DELIM2C)
        tmp[dirlen - 1] = FG_PATH_DELIMC;
    else if (dirlen)
        tmp.append(path::DELIMITER);
    tmp.append(filename);
    targetpath.clear();
    targetpath.append(tmp);
    return targetpath;
}
//>---------------------------------------------------------------------------------------

std::string path::join(const std::string &dirpath, const std::string &filename)
{
    std::string path;
    path::join(path, dirpath, filename);
    return path;
}
//>---------------------------------------------------------------------------------------

void path::join(std::string &path, std::vector<std::string> &parts)
{
    path.clear();
    size_t len = 0;
    int cnt = 0;
    for (auto &it : parts)
    {
        len += it.length() + 1;
    }
    path.reserve(len + 1);
    for (auto &it : parts)
    {
        path.append(it);
        cnt++;
        if (cnt != parts.size())
            path.append(path::DELIMITER);
    }
}
//>---------------------------------------------------------------------------------------
