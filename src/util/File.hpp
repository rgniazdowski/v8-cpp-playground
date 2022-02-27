#pragma once
#ifndef FG_INC_FILE
#define FG_INC_FILE

#include <util/FileBase.hpp>
#include <util/RegularFile.hpp>
#include <util/ZipFile.hpp>

namespace util
{
    /**
     * This is special wrapper for file operations
     * This class contains two objects - RegularFile & ZipFile
     * Selection depends on the selected path
     */
    class File : public util::base::File
    {
    public:
        using self_type = util::File;
        using base_type = util::base::File;
        using tag_type = util::RegularFile::tag_type;
        using Mode = util::base::File::Mode;
        using FileMode = util::base::File::Mode;
        using ZipMode = util::ZipFile::Mode;

    private:
        enum DataMode
        {
            ZIP,
            REGULAR
        }; // enum DataMode

    private:
        /// Object for managing Zip files and files within the Zip
        ZipFile m_zip;
        /// Object for managing regular files (plain, on disk/ROM)
        RegularFile m_regular;
        /// Pointer to the base type, is never NULL
        base_type *m_file;
        /// Current mode in which this object operates on - ZIP or REGULAR
        /// Depends on the selected path - path can point to ZIP or REGULAR
        /// or a REGULAR file inside of a ZIP file.
        DataMode m_mode;

    public:
        File();

        File(std::string_view filePath);

        File(const File &orig);

        virtual ~File();

        //>-------------------------------------------------------------------------------

        static inline const char *modeStr(Mode mode)
        {
            return RegularFile::modeStr(mode);
        }

        //>-------------------------------------------------------------------------------

        virtual std::string const &getPath(void) const override
        {
            return m_file->getPath();
        }

        virtual void setMode(Mode mode) override
        {
            m_file->setMode(mode);
            m_modeFlags = mode;
        }

        virtual void setPath(std::string_view filePath) override;

        //>-------------------------------------------------------------------------------

        using util::base::File::open;

        virtual inline bool open(void) override { return m_file->open(); }

        using util::base::File::close;

        virtual inline bool close(void) override { return m_file->close(); }

        virtual inline bool isOpen(void) const override { return m_file->isOpen(); }

        virtual inline bool exists(void) { return m_file->exists(); }

        //>-------------------------------------------------------------------------------

        using util::base::File::load;

        virtual inline char *load(void) override { return m_file->load(); }

        virtual inline int64_t read(void *buffer, unsigned int elemsize, unsigned int elemcount)
        {
            return m_file->read(buffer, elemsize, elemcount);
        }

        virtual inline char *readString(char *buffer, unsigned int maxlen)
        {
            return m_file->readString(buffer, maxlen);
        }

        virtual inline int64_t write(const void *buffer, unsigned int elemsize, unsigned int elemcount) override
        {
            return m_file->write(buffer, elemsize, elemcount);
        }

        virtual int print(const char *fmt, ...) override;

        virtual inline bool puts(std::string_view str) override { return m_file->puts(str); }

        //>-------------------------------------------------------------------------------

        virtual inline bool isEOF(void) override { return m_file->isEOF(); }

        virtual inline bool flush(void) override { return m_file->flush(); }

        virtual inline int getChar(void) override { return m_file->getChar(); }

        virtual inline bool putChar(char c) override { return m_file->putChar(c); }

        virtual inline uint64_t getSize(void) override { return m_file->getSize(); }

        virtual inline int64_t getPosition(void) override { return m_file->getPosition(); }

        virtual inline int64_t setPosition(int64_t offset, int whence) override { return m_file->setPosition(offset, whence); }
    }; //# class File
    using DataFile = File;
} //> namespace util

#endif /* FG_INC_FILE */