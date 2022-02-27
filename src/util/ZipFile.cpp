#include <util/ZipFile.hpp>
#include <util/Util.hpp>
#include <fstream>

util::ZipFile::ZipFile() : m_password(),
                           m_selectedFilePath(),
                           m_extractionPath(),
                           m_zipPath(),
                           m_filePaths(),
                           m_fileItor(),
                           m_currentFileID(0),
                           m_zf(nullptr),
                           m_uf(nullptr),
                           m_mode(Mode::READ)
{
    memset(&m_zInfo, 0, sizeof(ZipFileInfo));
    memset(&m_zGlobalInfo, 0, sizeof(ZipGlobalInfo));
    memset(&m_zFilePos, 0, sizeof(ZipFilePos));
}
//>---------------------------------------------------------------------------------------

util::ZipFile::ZipFile(std::string_view filePath) : m_password(),
                                                    m_selectedFilePath(),
                                                    m_extractionPath(),
                                                    m_zipPath(),
                                                    m_filePaths(),
                                                    m_fileItor(),
                                                    m_currentFileID(0),
                                                    m_zf(nullptr),
                                                    m_uf(nullptr),
                                                    m_mode(Mode::READ)
{
    memset(&m_zInfo, 0, sizeof(ZipFileInfo));
    memset(&m_zGlobalInfo, 0, sizeof(ZipGlobalInfo));
    memset(&m_zFilePos, 0, sizeof(ZipFilePos));
    setPath(filePath);
}
//>---------------------------------------------------------------------------------------

util::ZipFile::ZipFile(const ZipFile &orig) {}
//>---------------------------------------------------------------------------------------

util::ZipFile::~ZipFile()
{
    m_filePath.clear();
    m_selectedFilePath.clear();
#if 1
    m_selectedFilePath.shrink_to_fit();
    m_filePath.shrink_to_fit();
#endif
    m_filePaths.clear();
}
//>---------------------------------------------------------------------------------------

void util::ZipFile::setMode(FileMode mode)
{
    m_modeFlags = mode;  // File mode
    m_mode = Mode::READ; // ZipFile mode
    if (!!(m_modeFlags & FileMode::READ) && !(m_modeFlags & FileMode::UPDATE))
    {
        m_mode = Mode::READ;
    }
    else if (!!(m_modeFlags & FileMode::WRITE) && !(m_modeFlags & FileMode::UPDATE))
    {
        m_mode = Mode::WRITE;
    }
    else if (m_modeFlags & FileMode::READ_UPDATE ||
             m_modeFlags & FileMode::UPDATE ||
             m_modeFlags & FileMode::APPEND ||
             m_modeFlags & FileMode::APPEND_UPDATE ||
             m_modeFlags & FileMode::WRITE_UPDATE)
    {
        m_mode = Mode::UPDATE;
    }
#if 0
    if(m_mode == Mode::UPDATE) {
        printf("&READ_UPDATE: %u\n", static_cast<unsigned int>(m_modeFlags & FileMode::READ_UPDATE));
        printf("&UPDATE: %u\n", static_cast<unsigned int>(m_modeFlags & FileMode::UPDATE));
        printf("&APPEND: %u\n", static_cast<unsigned int>(m_modeFlags & FileMode::APPEND));
        printf("&APPEND_UPDATE: %u\n", static_cast<unsigned int>(m_modeFlags & FileMode::APPEND_UPDATE));
        printf("&WRITE_UPDATE: %u\n", static_cast<unsigned int>(m_modeFlags & FileMode::WRITE_UPDATE));
        printf("READ: %u\n", static_cast<unsigned int>(FileMode::READ));
        printf("WRITE: %u\n", static_cast<unsigned int>(FileMode::WRITE));
        printf("APPEND: %u\n", static_cast<unsigned int>(FileMode::APPEND));
        printf("UPDATE: %u\n", static_cast<unsigned int>(FileMode::UPDATE));
        printf("BINARY: %u\n", static_cast<unsigned int>(FileMode::BINARY));
        printf("READ_UPDATE: %u\n", static_cast<unsigned int>(FileMode::READ_UPDATE));
        printf("WRITE_UPDATE: %u\n", static_cast<unsigned int>(FileMode::WRITE_UPDATE));
        printf("APPEND_UPDATE: %u\n", static_cast<unsigned int>(FileMode::APPEND_UPDATE));
        printf("READ | BINARY: %u\n", static_cast<unsigned int>(FileMode::READ | FileMode::BINARY));
        printf("ZIP UPDATE: filepath[%s], zippath[%s]\n", m_filePath.c_str(), m_zipPath.c_str());
    }
#endif
}
//>---------------------------------------------------------------------------------------

void util::ZipFile::setPath(std::string_view filePath)
{
    // This will determine whether or not the specified file points to a valid
    // ZipFile (based on the extension) and also set the relative path to inner
    // compressed file (if any is specified). This means that this supports paths
    // like: "data/textures.zip/castle/flag.tga"
    if (filePath.empty())
        return;
    const char *ext = strings::stristr(filePath, ".zip");
    if (!ext)
        ext = strings::stristr(filePath, ".pk3");
    if (!ext)
    {
        // Probably not a valid Zip file
        m_filePath.clear();
        return;
    }
    if (ext)
    {
        auto extlen = strlen(ext);
        if (extlen > 4)
        {
            // This means that there is something more in the path
            // unsigned int fplen = ((uintptr_t)ext)-((uintptr_t)filePath);
            m_zipPath = filePath;
            m_zipPath.resize(m_zipPath.size() - extlen + 4);
#if 1
            m_zipPath.shrink_to_fit();
#endif
            // Full path to the file within the zip file - it cointains also the
            // name of the zip archive
            m_filePath = filePath;
            // Relative path to the selected file in the archive
            // Checking for a valid path is in open functions
            m_selectedFilePath = (ext + 5);
        }
        else
        {
            // the extension fits
            m_zipPath = filePath;
            m_filePath = m_zipPath;
            m_selectedFilePath.clear();
        }
    }
}
//>---------------------------------------------------------------------------------------

bool util::ZipFile::isCurrentFileDir(void)
{
    auto sn = m_selectedFilePath.size();
    if (sn)
    {
        std::string dirname;
        path::dirName(m_selectedFilePath, dirname);
        auto dn = dirname.size();
        char dc = dirname[dn - 1];
        char sc = m_selectedFilePath[sn - 1];
        if (dn == sn && (dc == '/' || dc == '\\') && (sc == '/' || sc == '\\'))
            return true;
    }
    return false;
}
//>---------------------------------------------------------------------------------------

bool util::ZipFile::goToNextFile(void)
{
    if (!isOpen())
        return false;
    m_currentFileID++;
    if (m_currentFileID >= (int)m_filePaths.size())
    {
        return false;
    }
    return private_selectFile(m_currentFileID);
}
//>---------------------------------------------------------------------------------------

bool util::ZipFile::goToPreviousFile(void)
{
    if (!isOpen())
        return false;
    m_currentFileID--;
    if (m_currentFileID < 0)
    {
        return false;
    }
    return private_selectFile(m_currentFileID);
}
//>---------------------------------------------------------------------------------------

bool util::ZipFile::goToFirstFile(void)
{
    if (m_filePath.empty())
        return false;
    return selectFile(m_filePath.c_str());
}
//>---------------------------------------------------------------------------------------

bool util::ZipFile::private_updateCurrentFileInfo(void)
{
    bool status = true;
    if (!isOpen())
        status = false;
    if (status && m_mode == Mode::READ && m_uf)
    {
        char fileInZip[256];
        int err = unzGetCurrentFileInfo64(m_uf, &m_zInfo.unz64, fileInZip, 256, nullptr, 0, nullptr, 0);
        if (err != UNZ_OK)
        {
            // Something went wrong while getting info
            status = false;
        }
        else
        {
            if (m_password.empty())
            {
                err = unzOpenCurrentFile(m_uf);
            }
            else
            {
                err = unzOpenCurrentFilePassword(m_uf, m_password.c_str());
            }
            if (err != UNZ_OK)
            {
                // Could not open the file
                m_selectedFilePath.clear();
                m_currentFileID = 0;
                status = false;
            }
            else
            {
                m_selectedFilePath = fileInZip;
            }
        }
    }
    if (status)
    {
        // Join the path to the file
        path::join(m_filePath, m_zipPath, m_selectedFilePath);
    }
    return status;
}
//>---------------------------------------------------------------------------------------

bool util::ZipFile::private_selectFile(const int id)
{
    if (id < 0 || id >= (int)m_filePaths.size())
        return false;
    if (m_filePath.empty())
    {
        return false;
    }
    if (!m_uf)
    {
        // The Zip file needs to be opened before selecting the file
        return false;
    }
    int err = UNZ_OK;
    err = unzCloseCurrentFile(m_uf);
    // Probably path points to the file inside of the Zip
    err = unzLocateFile(m_uf, m_filePaths[id].c_str(), nullptr);
    if (err != UNZ_OK)
    {
        m_selectedFilePath.clear();
        m_currentFileID = 0;
        return false;
    }
    m_currentFileID = id;
    // Properly selected path
    // m_selectedFilePath = m_filePaths[id];
    return private_updateCurrentFileInfo();
    // return true;
}
//>---------------------------------------------------------------------------------------

bool util::ZipFile::selectFile(std::string_view filePath)
{
    if (filePath.empty())
        return false;
    if (m_filePath.empty())
        return false;
    if (!m_uf)
    {
        // The Zip file needs to be opened before selecting the file
        return false;
    }
    int err = UNZ_OK;
    const char *ext = strings::stristr(filePath, ".zip");
    if (!ext)
        ext = strings::stristr(filePath, ".pk3");
    if (!ext)
    {
        err = unzCloseCurrentFile(m_uf);
        // Probably path points to the file inside of the Zip
        err = unzLocateFile(m_uf, filePath.data(), nullptr);
        if (err != UNZ_OK)
        {
            m_selectedFilePath.clear();
            m_currentFileID = 0;
            return false;
        }
        m_currentFileID = m_filePaths.find(std::string(filePath));
        // Properly selected path
        m_selectedFilePath = filePath;
    }
    else
    {
        // Check the path (the same as Zip) -> select the first file
        if (strings::equals(m_zipPath.c_str(), filePath.data(), false))
        {
            err = unzCloseCurrentFile(m_uf);
            err = unzGoToFirstFile(m_uf);
            m_currentFileID = 0;
            if (err != UNZ_OK)
            {
                m_selectedFilePath.clear();
                return false;
            }
        }
    }
    private_updateCurrentFileInfo();
    return true;
}
//>---------------------------------------------------------------------------------------

bool util::ZipFile::open(void)
{
    if (m_mode == Mode::NONE || m_zipPath.empty())
        return false;
    // MAIN OPEN FUNCTION FOR ZIP
    // Here the magic happens
    int err = UNZ_OK;
    if (m_mode == Mode::READ || m_mode == Mode::EXTRACT)
    {
        if (m_uf)
        {
            unzCloseCurrentFile(m_uf);
            unzClose(m_uf);
        }
        m_uf = unzOpen64((void *)m_zipPath.c_str());
        if (m_uf == nullptr)
        {
            // Failed to open file
            return false;
        }
        // Successfully opened file
        // Update global info
        err = unzGetGlobalInfo64(m_uf, &m_zGlobalInfo.unz64);
        if (err != UNZ_OK)
        {
            err = unzGetGlobalInfo(m_uf, &m_zGlobalInfo.unz);
            if (err != UNZ_OK)
            {
            }
        }

        {
            err = unzGoToFirstFile(m_uf);
            if (err != UNZ_OK)
                return false;
            // Clear current file list
            m_filePaths.clear();
            // Update the inner list of files
            unz_file_info64 file_info = {0};
            char fileInZip[256];
            do
            {
                err = unzGetCurrentFileInfo64(m_uf, &file_info, fileInZip, sizeof(fileInZip), nullptr, 0, nullptr, 0);
                if (err != UNZ_OK)
                    break;
                /* Display a '*' if the file is encrypted */
                // if((file_info.flag & 1) != 0)
                //     charCrypt = '*';
                auto len = strlen(fileInZip);
                if (len)
                {
                    m_filePaths.push_back(std::string(fileInZip));
                }
                err = unzGoToNextFile(m_uf);
            } while (err == UNZ_OK);
        }
    }
    if (m_selectedFilePath.empty())
    {
        // When passing the path to the main ZipFile (root)
        // The first file will be selected automatically.
        // It's important that after a successful call to open(), functions
        // for reading/writing can be called normally, without additional parameters.
        selectFile(m_filePath);
    }
    else
    {
        // If for now the selected file name is not empty
        // it means that it was set with (probably) setPath function
        selectFile(m_selectedFilePath);
    }
    return true;
}
//>---------------------------------------------------------------------------------------

bool util::ZipFile::close(void)
{
    if (!isOpen())
        return true;
    if (m_uf)
    {
        unzCloseCurrentFile(m_uf);
        unzClose(m_uf);
        m_uf = nullptr;
    }
    if (m_zf)
    {
        zipCloseFileInZip(m_zf);
        zipClose(m_zf, nullptr);
        m_zf = nullptr;
    }
    memset(&m_zInfo, 0, sizeof(ZipFileInfo));
    memset(&m_zGlobalInfo, 0, sizeof(ZipGlobalInfo));
    m_selectedFilePath.clear();
    m_filePaths.clear();
    return true;
}
//>---------------------------------------------------------------------------------------

bool util::ZipFile::isOpen(void) const
{
    return (bool)(m_zf != nullptr || m_uf != nullptr);
}
//>---------------------------------------------------------------------------------------

bool util::ZipFile::exists(void)
{
    std::ifstream fileCheck(m_filePath);
    return (bool)fileCheck.good();
}
//>---------------------------------------------------------------------------------------

char *util::ZipFile::load(void)
{
    char *data = nullptr;
    bool status = true;
    unsigned int fileSize = 0;
    if (!isOpen() || m_selectedFilePath.empty())
        status = false;
    if (!m_uf)
        status = false;
    if (status)
    {
        fileSize = (unsigned int)m_zInfo.unz64.uncompressed_size;
        if (!fileSize)
            status = false;
    }
    if (status)
    {
        data = new char[fileSize + 1];
        if (!data)
            status = false;
        else
            memset(data, 0, fileSize + 1);
    }
    if (status)
    {
        // Reselect the current file - this will rewind the file to the beginning
        if (getPosition() != 0L)
            status = selectFile(m_selectedFilePath.c_str());
    }
    if (status)
    {
        auto bytesRead = read((void *)data, 1, fileSize);
        if (bytesRead <= 0)
        {
            status = false;
            // fgFree((void *)data, fileSize, true);
            delete[] data;
            data = nullptr;
        }
    }
    return data;
}
//>---------------------------------------------------------------------------------------

int64_t util::ZipFile::read(void *buffer, unsigned int elemsize, unsigned int elemcount)
{
    if (!buffer || !elemsize || !elemcount || m_filePath.empty())
        return 0;
    if (m_mode == Mode::READ && !m_uf)
        return 0;
    if (m_selectedFilePath.empty())
    {
        // Selected file path must be not empty
        // Empty path for selected file means that no file (in zip) is currently open
        return 0;
    }
    int err = UNZ_OK;
    unsigned int bytesToRead = elemsize * elemcount;
    int elemRead = unzReadCurrentFile(m_uf, buffer, bytesToRead);
    if (elemRead < 0)
    {
        // Error
        return -1;
    }
    return elemRead;
}
//>---------------------------------------------------------------------------------------

char *util::ZipFile::readString(char *buffer, unsigned int maxlen)
{
    if (!buffer || !maxlen)
        return nullptr;
    if (!isOpen())
        return nullptr;
    if (m_selectedFilePath.empty())
        return nullptr;
    if (isEOF())
        return nullptr;
    char c = 0;
    unsigned int pos = 0;
    int64_t status = 0;
    if (m_uf)
    {
        do
        {
            status = read(&c, 1, 1);
            if (status > 0)
            {
                buffer[pos] = c;
                pos++;
                if (c == '\n')
                {
                    buffer[pos] = 0;
                    pos++;
                    status = -1;
                }
            }
            else if (!pos)
            {
                buffer[0] = 0;
                buffer = nullptr;
            }
        } while (status > 0 && pos < maxlen);
    }
    return buffer;
}
//>---------------------------------------------------------------------------------------

int64_t util::ZipFile::write(const void *buffer, unsigned int elemsize, unsigned int elemcount)
{
    if (!buffer || !elemsize || !elemcount || m_filePath.empty())
        return 0;
    return 0;
}
//>---------------------------------------------------------------------------------------

int util::ZipFile::print(const char *fmt, ...)
{
    if (!fmt || !isOpen())
        return 0;
    if (m_mode != Mode::WRITE && m_mode != Mode::UPDATE)
        return 0;
    if (m_selectedFilePath.empty())
        return 0;
    return 0;
}
//>---------------------------------------------------------------------------------------

bool util::ZipFile::puts(std::string_view str)
{
    if (str.empty() || !isOpen())
        return false;
    if (m_mode != Mode::WRITE && m_mode != Mode::UPDATE)
        return false;
    if (m_selectedFilePath.empty())
        return false;
    return false;
}
//>---------------------------------------------------------------------------------------

bool util::ZipFile::isEOF(void)
{
    bool status = false;
    if (m_uf && !m_selectedFilePath.empty())
    {
        if (unzeof(m_uf) == 1)
            status = true;
    }
    return status;
}
//>---------------------------------------------------------------------------------------

bool util::ZipFile::flush(void)
{
    return false;
}
//>---------------------------------------------------------------------------------------

int util::ZipFile::getChar(void)
{
    int c = 0;
    if (m_mode == Mode::READ && m_uf && !m_selectedFilePath.empty())
    {
        if (this->read((void *)&c, 1, 1) <= 0)
            c = 0;
    }
    return c;
}
//>---------------------------------------------------------------------------------------

bool util::ZipFile::putChar(char c)
{
    return false;
}
//>---------------------------------------------------------------------------------------

uint64_t util::ZipFile::getSize(void)
{
    uint64_t size = 0;
    // if(m_selectedFilePath.empty()) {
    //  There is no current file selected
    //  File info structure is invalid
    // return 0;
    // }
    if (m_uf && !m_selectedFilePath.empty())
        size = m_zInfo.unz64.uncompressed_size;
    return size;
}
//>---------------------------------------------------------------------------------------

int64_t util::ZipFile::getPosition(void)
{
    int64_t pos = 0LL;
    if (m_uf)
        pos = unztell(m_uf);
    // long int offset = (long int)unzGetOffset(m_uf);
    // long int tell = unztell(m_uf);
    // if(unzGetFilePos(m_uf, &m_zFilePos.unz) == UNZ_OK) {
    // printf("offset: %ld tell:%ld numoffile:%ld pos_in_zip_dir:%ld\n", offset, tell, m_zFilePos.unz.num_of_file, m_zFilePos.unz.pos_in_zip_directory);
    // }
    return pos;
}
//>---------------------------------------------------------------------------------------

int64_t util::ZipFile::setPosition(int64_t offset, int whence)
{
    return 0;
}
//>---------------------------------------------------------------------------------------
