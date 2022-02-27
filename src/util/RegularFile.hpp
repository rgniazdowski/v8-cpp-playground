#pragma once
#ifndef FG_INC_REGULAR_FILE
#define FG_INC_REGULAR_FILE

#include <BuildConfig.hpp>
#include <util/Tag.hpp>
#include <util/FileBase.hpp>

#include <vector>
#include <string>
#include <fstream>

#define FG_MAX_BUFFER 8192

namespace util
{
    class RegularFile;

    using RegularFileTag = Tag<RegularFile>;

    class RegularFile : public util::base::File
    {
    public:
        using self_type = RegularFile;
        using base_type = util::base::File;
        using tag_type = RegularFileTag;
        using Mode = util::base::File::Mode;

    protected:
        /// C standard or SDL_RWops file handle
        // FileHandleType *m_file;
        std::ifstream m_ifs;
        std::ofstream m_ofs;
        std::vector<std::byte> m_buffer;

    public:
        RegularFile();
        RegularFile(std::string_view filePath);
        RegularFile(const RegularFile &orig) = delete;

        virtual ~RegularFile();

        static const char *modeStr(Mode mode);

        //>-------------------------------------------------------------------------------

        using ::util::base::File::open;
        virtual bool open(void) override;

        //>-------------------------------------------------------------------------------

        virtual bool close(void) override;
        virtual bool isOpen(void) const override;
        virtual bool exists(void) override { return RegularFile::exists(m_filePath.c_str()); }

        static bool exists(std::string_view filePath);

        //>-------------------------------------------------------------------------------

        using ::util::base::File::load;

        virtual char *load(void) override;

        virtual int64_t read(void *buffer, unsigned int elemsize, unsigned int elemcount) override;
        virtual char *readString(char *buffer, unsigned int maxlen) override;

        virtual int64_t write(const void *buffer, unsigned int elemsize, unsigned int elemcount) override;
        virtual int print(const char *fmt, ...) override;
        virtual bool puts(std::string_view str) override;

        //>-------------------------------------------------------------------------------

        virtual bool isEOF(void) override;
        virtual bool flush(void) override;

        virtual int getChar(void) override;
        virtual bool putChar(char c) override;

        virtual uint64_t getSize(void) override;
        virtual int64_t getPosition(void) override;
        virtual int64_t setPosition(int64_t offset, int whence) override;

        static std::ios_base::openmode iosFileMode(Mode mode);

        // FileHandleType *getFilePtr(void) const
        //{
        //     return m_file;
        // }
    }; //# class RegularFile

} //> namespace util

#endif //> FG_INC_REGULAR_FILE