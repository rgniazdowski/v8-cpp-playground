#pragma once
#ifndef FG_INC_FILE_BASE
#define FG_INC_FILE_BASE

#include <util/EnumFlags.hpp>
#include <string>

namespace util
{
    namespace base
    {

        class File
        {
        public:
            /**
             * Enumeration for standard file manipulation modes
             */
            enum class Mode : unsigned int
            {
                /// No mode specified (invalid).
                NONE = 0,
                // Open file for input operations. The file must exist.
                READ = 1 << 0,
                // Create an empty file for output operations.
                WRITE = 1 << 1,
                // Open file for output at the end of a file. The file is created if it does not exist.
                APPEND = 1 << 2,
                // Additional update mode (both input/output).
                UPDATE = 1 << 3,
                // Open file as a binary file.
                BINARY = 1 << 4,
                // Open a file for update (both for input and output). The file must exist.
                READ_UPDATE = Mode::READ + Mode::UPDATE,
                // Create an empty file and open it for update (both for input and output).
                // If a file with the  same name already exists its  contents are discarded
                // and the file is treated as a new empty file.
                WRITE_UPDATE = Mode::WRITE + Mode::UPDATE,
                // Open a file for update (both for input and  output) with all output operations
                // writing data at the end of the file. Repositioning operations (fseek, fsetpos,
                // rewind)  affects the  next input  operations,  but  output operations move the
                // position back to the end of file.  The file is  created if  it does not exist.
                APPEND_UPDATE = Mode::APPEND + Mode::UPDATE
            }; // enum class Mode

        protected:
            std::string m_filePath;
            Mode m_modeFlags;

        public:
            File() : m_filePath(), m_modeFlags(Mode::READ) {}
            File(std::string_view filePath) : m_filePath(filePath), m_modeFlags(Mode::READ) {}
            virtual ~File() {}

        public:
            virtual void setPath(std::string_view filePath) { m_filePath = filePath; }
            virtual const std::string &getPath(void) const { return m_filePath; }
            virtual void setMode(Mode mode) { m_modeFlags = mode; }
            virtual Mode getMode(void) const { return m_modeFlags; }

            virtual bool isRead(void) const;
            virtual bool isWrite(void) const;
            virtual bool isUpdate(void) const;
            virtual bool isAppend(void) const;

            virtual bool open(void) = 0;
            bool open(std::string_view filePath)
            {
                if (m_modeFlags == Mode::NONE)
                    this->setMode(Mode::READ); // fallback
                if (filePath.empty())
                    return false;
                this->setPath(filePath);
                return open();
            }

            virtual bool close(void) = 0;
            virtual bool isOpen(void) const = 0;
            virtual bool exists(void) = 0;

            virtual char *load(void) = 0;
            char *load(std::string_view filePath)
            {
                if (filePath.empty())
                    return nullptr;
                if (isOpen())
                    close();
                setPath(filePath);
                if (m_filePath.empty())
                    return nullptr;
                if (!open())
                    return nullptr;
                return load();
            }

            virtual int64_t read(void *buffer, unsigned int elemsize, unsigned int elemcount) = 0;
            virtual char *readString(char *buffer, unsigned int maxlen) = 0;

            virtual int64_t write(const void *buffer, unsigned int elemsize, unsigned int elemcount) = 0;
            virtual int print(const char *fmt, ...) = 0;
            virtual bool puts(std::string_view str) = 0;

            virtual bool isEOF(void) = 0;
            virtual bool flush(void) = 0;

            virtual int getChar(void) = 0;
            virtual bool putChar(char c) = 0;

            virtual uint64_t getSize(void) = 0;
            virtual int64_t getPosition(void) = 0;
            virtual int64_t setPosition(int64_t offset, int whence) = 0;
        }; //# class File

        // Overload standard bitwise operator for enum type
        ENUM_FLAGS(File::Mode);

        bool File::isRead(void) const { return (bool)(m_modeFlags & Mode::READ); }
        bool File::isWrite(void) const { return (bool)(m_modeFlags & Mode::WRITE); }
        bool File::isUpdate(void) const { return (bool)(m_modeFlags & Mode::UPDATE); }
        bool File::isAppend(void) const { return (bool)(m_modeFlags & Mode::APPEND); }
    } //> namespace base
} //> namespace util

#endif //> FG_INC_FILE_BASE