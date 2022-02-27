#include <util/File.hpp>
#include <cstdarg>

util::File::File() : m_zip(),
                     m_regular(),
                     m_file(nullptr),
                     m_mode(REGULAR)
{
    m_file = &m_regular;
    m_modeFlags = Mode::READ;
}
//>---------------------------------------------------------------------------------------

util::File::File(std::string_view filePath) : m_zip(),
                                              m_regular(),
                                              m_file(nullptr),
                                              m_mode(REGULAR)
{
    m_file = &m_regular;
    m_modeFlags = Mode::READ;
    this->setPath(filePath);
}
//>---------------------------------------------------------------------------------------

util::File::File(const File &orig)
{
    this->m_filePath = orig.m_filePath;
    this->m_mode = orig.m_mode;
    this->m_modeFlags = orig.m_modeFlags;
    this->m_zip = orig.m_zip;
    this->m_regular.close();
    this->m_regular.setMode(orig.m_regular.getMode());
    this->m_regular.setPath(orig.m_regular.getPath());
    if (this->m_mode == ZIP)
        this->m_file = &m_zip;
    else
        this->m_file = &m_regular;
}
//>---------------------------------------------------------------------------------------

util::File::~File()
{
    m_zip.close();
    m_regular.close();
    m_file = nullptr;
}
//>---------------------------------------------------------------------------------------

void util::File::setPath(std::string_view filePath)
{
    if (filePath.empty())
        return;
    base_type::setPath(filePath);
    // Use the m_zip setPath to determine if this points to a ZipFile
    m_zip.setPath(filePath);
    auto path = m_zip.getPath();
    if (path.empty())
    {
        // This means that path points to a regular file
        // Close the zip file just in case
        if (m_zip.isOpen())
            m_zip.close();
        m_mode = REGULAR;
        m_file = &m_regular;
        m_file->setPath(filePath);
    }
    else
    {
        // This means that path points to a valid ZipFile or a file inside a Zip
        if (m_regular.isOpen())
            m_regular.close();
        m_mode = ZIP;
        m_file = &m_zip;
    }
}
//>---------------------------------------------------------------------------------------

int util::File::print(const char *fmt, ...)
{
    if (fmt == nullptr || m_file == nullptr)
    {
        // FG_MessageSubsystem->reportWarning(tag_type::name(), FG_ERRNO_FILE_WRONG_PARAMETERS);
        return -1;
    }

    char buf[8192];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, 8192, fmt, args);
    va_end(args);

    return m_file->puts(buf);
}
//>---------------------------------------------------------------------------------------