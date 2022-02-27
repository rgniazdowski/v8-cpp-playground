#pragma once
#ifndef FG_INC_ZIP_FILE
#define FG_INC_ZIP_FILE

#include <BuildConfig.hpp>
#include <util/Vector.hpp>
#include <util/FileBase.hpp>

#include "unzip.h"
#include "zip.h"

namespace util
{
    /**
     * This is a special class  wrapper  for zip/unzip operations based on MiniZip library
     * (Zlib contrib). Various operations will be available,  the most  important ones are
     * reading the  archive, listing the files and opening the files inside the archive in
     * similar (almost seamless) way as  opening regular files  on a drive.  This  will be
     * achieved by using special pathnames (eg. data/pack.zip/textures/flag.png).
     *
     * This class will be using custom build MiniZip static library - latest version from
     * git repository.  For now (XII.14) the MiniZip library supports AES encryption, I/O
     * buffering, PKWARE disk spanning.
     */
    class ZipFile : public ::util::base::File
    {
    public:
        using FileMode = ::util::base::File::Mode;

    public:
        enum class Mode
        {
            /* No mode has been specified, illegal (initial) state */
            NONE,
            /* This zip file is opened only for reading - manipulation will
             * be similar to a plain file. Any writing operations will fail
             */
            READ,
            /* Zip file is opened for extraction, files will be written to
             * selected location */
            EXTRACT,
            /* Zip file is opened for writing (adding at the end), new files
             * will be added */
            WRITE,
            /* Zip file is opened for update (similar to writing), new files
             * can be added, some can be updated */
            UPDATE,
            /* Special mode for creating a new, empty zip file container */
            CREATE
        };

    private:
        /// Used password for decoding the encrypted Zip file
        std::string m_password;
        /// Currently selected file inside of the Zip - relative path
        std::string m_selectedFilePath;
        /// Currently selected output (extraction) path
        std::string m_extractionPath;
        /// This path will point to the zip file location - without the internal
        /// path to the compressed file within the zip.
        std::string m_zipPath;
        /// List/vector with the file paths (relative) in the specified Zip
        StringVector m_filePaths;
        /// Iterator to the element in the string vector (file paths)
        StringVector::iterator m_fileItor;
        /// Number (ID) of the currently selected file (in Zip)
        int m_currentFileID;

        union ZipFileInfo
        {
            unz_file_info64 unz64;
            unz_file_info unz;
            zip_fileinfo zip;
        } m_zInfo;

        union ZipGlobalInfo
        {
            unz_global_info64 unz64;
            unz_global_info unz;
        } m_zGlobalInfo;

        union ZipFilePos
        {
            unz64_file_pos unz64;
            unz_file_pos unz;
        } m_zFilePos;
        /// Special zip handle used when creating the new archive (MiniZip)
        zipFile m_zf;
        /// Special zip handle used for unzipping (reading) the existing Zip
        unzFile m_uf;
        /// Enumeration for currently selected ZipFile handling mode
        Mode m_mode;

    private:
        bool private_selectFile(const int id);
        bool private_updateCurrentFileInfo(void);

    public:
        ZipFile();
        ZipFile(std::string_view filePath);
        ZipFile(const ZipFile &orig);

        virtual ~ZipFile();

        //>-------------------------------------------------------------------------------

        void setPassword(std::string_view password) { m_password = password; }
        std::string const &getPassword(void) const { return m_password; }

        virtual void setPath(std::string_view filePath) override;
        virtual void setMode(FileMode mode) override;
        virtual void setMode(Mode mode) { m_mode = mode; }

        std::string const &getCurrentFile(void) const { return m_selectedFilePath; }
        std::string const &getZipPath(void) const { return m_selectedFilePath; }
        FileMode getFileMode(void) const { return m_modeFlags; }
        Mode getMode(void) const { return m_mode; }

        StringVector const &getFileList(void) const { return m_filePaths; }

        //>-------------------------------------------------------------------------------

        bool isCurrentFileDir(void);
        bool goToNextFile(void);
        bool goToPreviousFile(void);
        bool goToFirstFile(void);

        virtual bool selectFile(std::string_view filePath);

        //>-------------------------------------------------------------------------------

        using ::util::base::File::open;
        virtual bool open(void) override;

        //>-------------------------------------------------------------------------------

        virtual bool close(void) override;
        virtual bool isOpen(void) const override;
        virtual bool exists(void) override;

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
    }; //# class ZipFile

} //> namespace util

#endif /* FG_INC_ZIP_FILE */
