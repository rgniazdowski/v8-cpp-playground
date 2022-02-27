#pragma once
#ifndef FG_INC_DIRENT
#define FG_INC_DIRENT

#include <BuildConfig.hpp>

#include <vector>
#include <string>
#include <cstdio>
#include <sys/stat.h>

namespace util
{
    class Dirent
    {
    public:
        Dirent();
        Dirent(std::string_view dirPath);
        virtual ~Dirent();

    private:
        bool internal_readZipFile(const std::string &fileName,
                                  const std::string &filePath,
                                  bool recursive = false);

    public:
        bool readDir(bool recursive = false, bool listZipFiles = false);

        bool readDir(std::string_view dirPath, bool recursive = false, bool listZipFiles = false);

        const char *getNextFile(void);

        std::string &getNextFilePath(std::string &path);
        std::string &searchForFile(std::string &output, const std::string &basePath,
                                   const std::string &pattern, const bool deep = false);

        const std::vector<std::string> &getFiles(void) const { return m_filePaths; }
        bool rewind(void);
        void clearList(void);

    protected:
        std::string m_dirPath;
        std::vector<std::string> m_filePaths;
        std::vector<std::string>::iterator m_fileIt;

        bool m_isRecursive;
    }; //# class Dirent

} //> namespace util

#endif //> FG_INC_DIRENT