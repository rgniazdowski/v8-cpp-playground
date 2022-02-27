#pragma once
#ifndef FG_INC_MANAGED_DATA_FILE
#define FG_INC_MANAGED_DATA_FILE

#include <resource/ManagedObject.hpp>
#include <map>
#include <string>

namespace resource
{
    template <typename THandleType, typename TMapKeyType>
    class ManagedDataFile : public ManagedObject<THandleType>
    {
    public:
        using handle_type = THandleType;
        using map_key_type = TMapKeyType;
        using tag_type = typename handle_type::tag_type;
        using user_type = typename tag_type::user_type;
        using base_type = ManagedObject<handle_type>;

        using FileMapping = std::map<TMapKeyType, std::string>;
        using FileMappingItor = typename FileMapping::iterator;
        using FileMappingConstItor = typename FileMapping::const_iterator;

    public:
        ManagedDataFile() : m_defaultID((TMapKeyType)-1)
        {
            m_filePath.clear();
            m_fileMapping.clear();
        }
        virtual ~ManagedDataFile()
        {
            m_filePath.clear();
            m_fileMapping.clear();
        }

    public:
        virtual void setFilePath(std::string_view path)
        {
            m_filePath = path;
            m_fileMapping[m_defaultID] = path;
        }

        virtual void setFilePath(std::string_view path, TMapKeyType id)
        {
            m_fileMapping[id] = path;
            if (id == m_defaultID)
                m_filePath = path;
            else if (m_filePath.empty())
                m_filePath = path;
        }

        inline FileMapping &getFileMapping(void) { return m_fileMapping; }
        inline unsigned int getFilesCount(void) const { return m_fileMapping.size(); }

        inline std::string const &getFilePath(void) const { return m_filePath; }

        inline std::string const &getFilePath(TMapKeyType id) const
        {
            FileMappingConstItor c_itor = m_fileMapping.find(id);
            if (c_itor == m_fileMapping.end())
                return m_filePath;
            return c_itor->second;
        }

        inline void setDefaultID(TMapKeyType id) { m_defaultID = id; }

    protected:
        FileMapping m_fileMapping;
        std::string m_filePath;
        TMapKeyType m_defaultID;
    }; //# class ManagedDataFile<THandleType, TMapKeyType>

} //> namespace resource

#endif //> FG_INC_MANAGED_DATA_FILE