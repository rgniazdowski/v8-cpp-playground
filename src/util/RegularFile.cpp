#include <util/RegularFile.hpp>

#include <filesystem>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>

std::ios_base::openmode util::RegularFile::iosFileMode(Mode mode)
{
    //(at end) Set the stream's position indicator to the end of the stream on opening.
    //??
    // int ate = static_cast<int>(0x04);

    std::ios_base::openmode openmode = 0;
    if ((bool)(mode & Mode::READ))
        openmode = std::ios_base::in;
    else if ((bool)(mode & Mode::WRITE))
        openmode = std::ios_base::out;
    if ((bool)(mode & Mode::BINARY))
        openmode |= std::ios_base::binary;
    if ((bool)(mode & Mode::APPEND))
        openmode |= std::ios_base::app;
    if ((bool)(mode & Mode::UPDATE))
        openmode |= std::ios_base::trunc;
    return openmode;
}

bool util::RegularFile::exists(std::string_view filePath)
{
    std::ifstream fileCheck(filePath.data());
    return fileCheck.good();
}
//>---------------------------------------------------------------------------------------

util::RegularFile::RegularFile()
{
    m_modeFlags = Mode::READ | Mode::BINARY;
}
//>---------------------------------------------------------------------------------------

util::RegularFile::RegularFile(std::string_view filePath)
{
    m_modeFlags = Mode::READ | Mode::BINARY;
    m_filePath = filePath;
}
//>---------------------------------------------------------------------------------------

util::RegularFile::~RegularFile()
{
    close();
    m_filePath.clear();
}
//>---------------------------------------------------------------------------------------

const char *util::RegularFile::modeStr(Mode mode)
{
    if (mode == Mode::NONE)
        return "";
    if ((bool)!!(mode & Mode::READ))
    {
        if ((mode & Mode::UPDATE) && (mode & Mode::BINARY))
            return "r+b";
        if ((bool)!!(mode & Mode::UPDATE))
            return "r+";
        if ((bool)!!(mode & Mode::BINARY))
            return "rb";
        return "r";
    }
    if ((bool)!!(mode & Mode::WRITE))
    {
        if (mode & Mode::UPDATE && mode & Mode::BINARY)
            return "w+b";
        if ((bool)!!(mode & Mode::UPDATE))
            return "w+";
        if ((bool)!!(mode & Mode::BINARY))
            return "wb";
        return "w";
    }
    if ((bool)!!(mode & Mode::APPEND))
    {
        if (mode & Mode::UPDATE && mode & Mode::BINARY)
            return "a+b";
        if ((bool)!!(mode & Mode::UPDATE))
            return "a+";
        if ((bool)!!(mode & Mode::BINARY))
            return "ab";
        return "a";
    }
    return "";
}
//>---------------------------------------------------------------------------------------

bool util::RegularFile::open(void)
{
    if (m_filePath.empty())
    {
        return false;
    }
    if (isRead())
    {
        m_ifs.open(m_filePath, self_type::iosFileMode(m_modeFlags));
        return m_ifs.good();
    }
    if (isWrite())
    {
        m_ofs.open(m_filePath, self_type::iosFileMode(m_modeFlags));
        return m_ofs.good();
    }
    return false;
}
//>---------------------------------------------------------------------------------------

bool util::RegularFile::close(void)
{
    if (!m_ifs.is_open() && !m_ofs.is_open())
        return false;
    if (m_ifs.is_open())
        m_ifs.close();
    if (m_ofs.is_open())
        m_ofs.close();
    m_ifs.clear(0, false);
    m_ofs.clear(0, false);
    return true;
}
//>---------------------------------------------------------------------------------------

bool util::RegularFile::isOpen(void) const
{
    return (isRead() && m_ifs.is_open()) || ((isWrite() || isAppend()) && m_ofs.is_open());
}
//>---------------------------------------------------------------------------------------

char *util::RegularFile::load(void)
{
    // force different mode when explicitly loading a file
    setMode(Mode::READ | Mode::BINARY);
    if (!isOpen() && !open())
    {
        // FG_MessageSubsystem->reportWarning(tag_type::name(), FG_ERRNO_FILE_ALREADY_OPEN, "%s", filePath);
        return nullptr;
    }
    auto fileSize = getSize();
    if (fileSize <= 0)
    {
        // FG_MessageSubsystem->reportWarning(tag_type::name(), FG_ERRNO_FILE_ERROR_SIZE, "%s", filePath);
        close();
        return nullptr;
    }
    char *fileBuffer = new char[fileSize + 1];
    if (fileBuffer == nullptr)
    {
        // FG_MessageSubsystem->reportError(tag_type::name(), FG_ERRNO, "%s", filePath); // #FIXME - memory error codes
        close();
        return nullptr;
    }
#if defined(FG_USING_SDL2)
    int bytesRead = m_file->read(m_file, fileBuffer, 1, fileSize);
#if defined(FG_USING_PLATFORM_ANDROID)
    FG_LOG_DEBUG("Loading file '%s' into buffer[%d], contents: '%s'", filePath, bytesRead, fileBuffer);
#endif
#else
    int64_t bytesRead = read(fileBuffer, 1, (unsigned int)fileSize);
#endif
    fileBuffer[fileSize] = '\0';
    if (bytesRead != (int)fileSize)
    {
        // FG_MessageSubsystem->reportWarning(tag_type::name(), FG_ERRNO_FILE_READ_COUNT, "%s", filePath);
        // fgFree(fileBuffer);
        delete fileBuffer;
        fileBuffer = nullptr;
        fileSize = 0;
        close();
        return nullptr;
    }

    return fileBuffer;
}
//>---------------------------------------------------------------------------------------

int64_t util::RegularFile::read(void *buffer, unsigned int elemsize, unsigned int elemcount)
{
    if (m_ifs.bad() || !m_ifs.is_open())
        return -1;
    auto bytesRead = m_ifs.readsome(static_cast<char *>(buffer), elemsize * elemcount);
    return bytesRead;
}
//>---------------------------------------------------------------------------------------

char *util::RegularFile::readString(char *buffer, unsigned int maxlen)
{
    if (m_ifs.bad() || !m_ifs.is_open())
        return nullptr;
    if (m_ifs.getline(buffer, maxlen).good())
        return buffer;
    return nullptr;
}
//>---------------------------------------------------------------------------------------

int util::RegularFile::print(const char *fmt, ...)
{
    if (fmt == nullptr)
    {
        return -1;
    }

    char buf[FG_MAX_BUFFER];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, FG_MAX_BUFFER, fmt, args);
    va_end(args);

    //#if !defined(FG_USING_SDL2)
    // FG_ERRNO_CLEAR();
    // clearerr(m_file);
    //#endif

    // int charsCount = fprintf(m_file, "%s", buf);
    // if (ferror(m_file))
    //{
    //  if (FG_ERRNO)
    //      FG_MessageSubsystem->reportError(tag_type::name(), FG_ERRNO);
    //  else
    //      FG_MessageSubsystem->reportError(tag_type::name(), FG_ERRNO_FILE_ERROR_WRITE);
    //}
    // return charsCount;
    return 0;
}
//>---------------------------------------------------------------------------------------

int64_t util::RegularFile::write(const void *buffer, unsigned int elemsize, unsigned int elemcount)
{
    if (!m_ofs.good() || !m_ofs.is_open())
        return -1;
    auto start = getPosition();
    int64_t written = -1;
    m_ofs.write(static_cast<const char *>(buffer), elemsize * elemcount);
    if (m_ofs.good())
        written = getPosition() - start;
    return written;
}
//>---------------------------------------------------------------------------------------

bool util::RegularFile::puts(std::string_view str)
{
    if (!m_ofs.good() || !m_ofs.is_open())
        return false;
    return m_ofs.write(str.data(), str.length()).good();
}
//>---------------------------------------------------------------------------------------

bool util::RegularFile::isEOF(void)
{
    if (m_ifs.is_open() && m_ifs.good())
        return m_ifs.eof();
    if (m_ofs.is_open() && m_ofs.good())
        return m_ofs.eof();
    return false;
}
//>---------------------------------------------------------------------------------------

bool util::RegularFile::flush(void)
{
    if (m_ofs.bad() || !m_ofs.is_open())
        return false;
    if (m_ofs.good())
    {
        m_ofs.flush();
        return m_ofs.good();
    }
    return false;
}
//>---------------------------------------------------------------------------------------

int util::RegularFile::getChar(void)
{
    if (!m_ifs.good() || !m_ifs.is_open())
        return -1;
    auto c = m_ifs.get();
    return c;
}
//>---------------------------------------------------------------------------------------

bool util::RegularFile::putChar(char c)
{
    if (!m_ofs.good() || !m_ofs.is_open())
        return false;
    m_ofs.put(c);
    return m_ofs.eof();
}
//>---------------------------------------------------------------------------------------

uint64_t util::RegularFile::getSize(void)
{
    std::error_code err;
    auto size = std::filesystem::file_size(std::filesystem::path(m_filePath), err);
    if (err)
    {
        return 0;
    }
    return size;
}
//>---------------------------------------------------------------------------------------

int64_t util::RegularFile::getPosition(void)
{
    int64_t pos = 0;
    if (m_ifs.good())
        pos = m_ifs.tellg();
    else if (m_ofs.good())
        pos = m_ofs.tellp();
    return pos;
}
//>---------------------------------------------------------------------------------------

int64_t util::RegularFile::setPosition(int64_t offset, int whence)
{
    int64_t pos = 0;
    if (m_ifs.good())
    {
        m_ifs.seekg(offset, whence);
        pos = m_ifs.tellg();
    }
    else if (m_ofs.good())
    {
        m_ofs.seekp(offset, whence);
        pos = m_ofs.tellp();
    }
    return pos;
}
//>---------------------------------------------------------------------------------------