#include <util/Util.hpp>
#include <util/Dirent.hpp>
#include <util/ZipFile.hpp>

util::Dirent::Dirent() : m_dirPath(),
                         m_filePaths(),
                         m_fileIt(),
                         m_isRecursive(false)
{
    m_fileIt = m_filePaths.end();
}

util::Dirent::Dirent(std::string_view dirPath)
{
    m_isRecursive = false;
    readDir(dirPath, false);
}

util::Dirent::~Dirent()
{
    m_dirPath.clear();
    m_filePaths.clear();
}

bool util::Dirent::internal_readZipFile(const std::string &fileName, const std::string &filePath, bool recursive)
{
    // If the current file is a zip file and the listing trigger is
    // active - append full zip listing into file names/paths vector
    util::ZipFile zip(filePath);
    zip.open();
    auto &zipFileList = zip.getFileList();
    auto nZipFiles = zipFileList.size();
    for (auto i = 0; i < nZipFiles; i++)
    {
        // Need to check here if the path inside of the zip does not
        // point to a directory - such path has a ending delimeter
        std::string filePathInZip;
        // Join the paths
        if (recursive)
            path::join(filePathInZip, filePath, zipFileList[i]);
        else
            path::join(filePathInZip, fileName, zipFileList[i]);
        std::string dirInZipName;
        // If the path has an ending delimeter the dirName function
        // will return the same string - no changes
        path::dirName(filePathInZip, dirInZipName);
        auto fn = filePathInZip.size();
        auto dn = dirInZipName.size();
        char dc = dirInZipName[dn - 1];
        char fc = filePathInZip[fn - 1];
        // Check whether path has the ending delimeter - it's not so accurate
        if (!(dn == fn && (dc == '/' || dc == '\\') && (fc == '/' || fc == '\\')))
            m_filePaths.push_back(filePathInZip);
    }
    zip.close();
    return (bool)(!!nZipFiles);
}
//>---------------------------------------------------------------------------------------

bool util::Dirent::readDir(bool recursive, bool listZipFiles)
{
    m_filePaths.clear();
    FileNamesVector dirStack;
    FileNamesVector dirPaths;
    DirEntriesVector dirEntries;
    strings::split(m_dirPath, ';', dirPaths);
    for (auto &dirPath : dirPaths)
    {
        const char *fileExt = strings::stristr(dirPath, ".zip");
        if (!fileExt)
            fileExt = strings::stristr(dirPath, ".pk3");
        if (fileExt)
            internal_readZipFile(dirPath, dirPath, false); // dir path points to a zip file
        else
            dirStack.push_back(dirPath);
        // Go through directory stack
        while (!dirStack.empty())
        {
            std::string curDir = dirStack.back();
            dirStack.pop_back();
            // FG_LOG_DEBUG("util::dirent: Opening directory for reading: '%s'", curDir.c_str());
            dirEntries.clear();
            read_directory(curDir, dirEntries);
            if (dirEntries.empty())
                continue; // empty folder
            for (auto &dirEntry : dirEntries)
            {
                bool isDir = dirEntry.is_directory();
                bool isZip = false;
                auto &filePath = dirEntry.path().string();
                auto &fileName = dirEntry.path().filename().string();
                if (fileName[0] == '.' || strings::startsWith(fileName.c_str(), "private", false))
                    continue; // skip private files - these are never loaded
                if (!isDir)
                {
                    const char *fileExt = strings::stristr(filePath, ".zip");
                    if (!fileExt)
                        fileExt = strings::stristr(filePath, ".pk3");
                    if (fileExt)
                        isZip = true;
                }
                if (recursive && isDir)
                    dirStack.push_back(filePath); // throw the path to a directory onto the stack for next read
                else if (!isDir)
                    m_filePaths.push_back(filePath); // It's not a directory so push it into the list
                if (isZip && listZipFiles)
                    internal_readZipFile(fileName, filePath, recursive);
            }
        }
    } // for(...) -> split dir path by ';'
    rewind();
    m_isRecursive = recursive;
    if (!m_filePaths.empty())
        return true;
    return false;
}
//>---------------------------------------------------------------------------------------

bool util::Dirent::readDir(std::string_view dirPath, bool recursive, bool listZipFiles)
{
    if (dirPath.empty())
        m_dirPath = path::getAssetsPath();
    else
        m_dirPath = dirPath;
    return readDir(recursive, listZipFiles);
}
//>---------------------------------------------------------------------------------------

const char *util::Dirent::getNextFile(void)
{
    if (m_fileIt == m_filePaths.end())
    {
        m_fileIt = m_filePaths.begin();
        if (m_fileIt != m_filePaths.end())
            return (*m_fileIt).c_str();
        return NULL;
    }
    m_fileIt++;
    if (m_fileIt == m_filePaths.end())
        return NULL;

    return (*m_fileIt).c_str();
}
//>---------------------------------------------------------------------------------------

std::string &util::Dirent::getNextFilePath(std::string &path)
{
    const char *filename = util::Dirent::getNextFile();
    path.clear();
#if 0
    if(filename && !m_isRecursive) {
        // #FIXME - this will cause error if fgDirent was not recursive and
        // did not store file paths by default - if fgDirent was called with
        // many directories to list -> FUBAR :(
        path::join(path, m_dirPath, std::string(filename));
    } else if(filename && m_isRecursive) {
        // with the recursive mode, this array always stores paths (relative)
        path = filename;
    }
#endif
    if (filename)
        path = filename;
    return path;
}

std::string &util::Dirent::searchForFile(std::string &output,
                                         const std::string &basePath,
                                         const std::string &patterns,
                                         const bool deep)
{
    std::string searchPath;
    if (!basePath.empty())
    {
        searchPath = basePath;
    }
    else
    {
        // The default search path
        searchPath = path::getAssetsPath();
    }
#if 0
    if(searchPath[0] != '.') {
        if(searchPath[0] == '/' || searchPath[0] == '\\')
            searchPath.insert(searchPath.begin(), 1, '.');
        else
            searchPath.insert(0, "./");
    }
#endif
#if defined(FG_USING_PLATFORM_ANDROID)
    /* Need to fix this path for android - cannot begin with ./
     * also it would be good to move such things to some fg::path::* functions
     * The below part and others are used quite a lot #FIXME #CODEREPEAT
     */
    if (searchPath[0] == '.')
    {
        int skip = 1;
        if (searchPath[1] == FG_PATH_DELIMC || searchPath[1] == FG_PATH_DELIM2C)
            skip = 2;
        searchPath = searchPath.substr(skip);
    }
#endif /* FG_USING_PLATFORM_ANDROID */
    if (searchPath.length() && searchPath[searchPath.length() - 1] != '/' && searchPath[searchPath.length() - 1] != '\\')
        searchPath.append(path::DELIMITER);
    output.clear();
    bool stop = false;
    std::string foundPath;
    const char *subPath;
    std::vector<std::string> patternVec;
    strings::split(patterns, ';', patternVec);
    do
    {
        if (getNextFilePath(foundPath).empty())
            stop = true;
        if (!stop && (strings::startsWith(foundPath, searchPath) || searchPath.empty()))
        {
            subPath = foundPath.c_str() + searchPath.length();
            // If the found subpath contains delimeters - skip if deep trigger is not active
            if (!deep && strings::containsChars(subPath, "/\\"))
                continue;
            if (patternVec.empty())
                output = foundPath;
            for (int i = 0; i < (int)patternVec.size(); i++)
            {
                std::string &pattern = patternVec[i];
                if (pattern.length())
                {
                    const char *filename = path::fileName(subPath);
                    if (pattern[0] == '*')
                    {
                        if (strings::endsWith(filename, (pattern.c_str() + 1), false))
                        {
                            output = foundPath;
                        }
                    }
                    else if (pattern[pattern.length() - 1] == '*')
                    {
                        std::string _tmp = pattern.substr(0, pattern.length() - 1);
                        if (strings::startsWith(filename, _tmp.c_str(), false))
                        {
                            output = foundPath;
                        }
                    }
                    else
                    {
                        if (pattern.compare(filename) == 0)
                        {
                            output = foundPath;
                        }
                    }
                }
                else
                {
                    output = foundPath;
                }
            }
        }
    } while (!stop && output.empty());
    return output;
}
//>---------------------------------------------------------------------------------------

bool util::Dirent::rewind(void)
{
    m_fileIt = m_filePaths.end();
    if (!m_filePaths.size())
        return false;
    return true;
}
//>---------------------------------------------------------------------------------------

void util::Dirent::clearList(void)
{
    m_isRecursive = false;
    m_filePaths.clear();
    m_fileIt = m_filePaths.end();
}
//>---------------------------------------------------------------------------------------
