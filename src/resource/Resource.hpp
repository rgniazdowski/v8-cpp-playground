#ifndef FG_INC_RESOURCE
#define FG_INC_RESOURCE

#include <string>
#include <ctime>

#include <Quality.hpp>
#include <Manager.hpp>
#include <resource/ManagedDataFile.hpp>
#include <util/Handle.hpp>
#include <util/Tag.hpp>

namespace resource
{
    class ResourceManager;
    class ResourceGroup;
    class Resource;
}

namespace resource
{
    using ResourceTag = ::util::Tag<Resource>;
    using ResourceHandle = ::util::Handle<ResourceTag>;

    enum class ResourcePriority : char
    {
        INVALID = ((char)(-1)),

        LOW = 0,
        MEDIUM = 1,
        HIGH = 2,

        RESERVED1 = 3,
        RESERVED2 = 4,
        RESERVED3 = 5
    };

    /// Enum type holding all possible resource types in the game engine
    using ResourceType = unsigned int;

    const ResourceType AUTO = 0x0000;
    const ResourceType INVALID = 0x0000;

    class Resource : public ManagedDataFile<ResourceHandle, Quality>
    {
        friend class ResourceManager;
        friend class ResourceGroup;

    public:
        using base_type = ManagedDataFile<ResourceHandle, Quality>;
        using tag_type = ResourceTag;
        using handle_type = ResourceHandle;
        using quality_type = Quality;

    public:
        Resource() : m_priority(ResourcePriority::LOW),
                     m_quality(Quality::UNIVERSAL),
                     m_resType(resource::INVALID),
                     m_lastAccess(0),
                     m_size(0),
                     m_isReady(false)
        {
            setDefaultID(Quality::UNIVERSAL);
        }

        Resource(std::string_view path) : m_priority(ResourcePriority::LOW),
                                          m_quality(Quality::UNIVERSAL),
                                          m_resType(resource::INVALID),
                                          m_lastAccess(0),
                                          m_size(0),
                                          m_isReady(false)
        {
            setDefaultID(Quality::UNIVERSAL);
            setFilePath(path);
        }

        virtual ~Resource() {}

    protected:
        virtual void clear(void)
        {
            m_resType = resource::INVALID;
            m_priority = ResourcePriority::LOW;
            m_quality = Quality::UNIVERSAL;
            m_lastAccess = 0;
            m_isReady = false;
            m_size = 0;
            m_fileMapping.clear();
            m_filePath.clear();
            // m_pManager = NULL;
        }

    public:
        virtual bool create(void) = 0;
        virtual void destroy(void) { Resource::clear(); };

        virtual bool recreate(void) = 0;
        virtual void dispose(void) = 0;

        virtual size_t getSize(void) const { return m_size; }
        virtual bool isDisposed(void) const = 0;

        inline virtual void setFlags(std::string_view flags) {}

        inline void setPriority(ResourcePriority priority) { m_priority = priority; }
        inline ResourcePriority getPriority(void) const { return m_priority; }

        inline void setQuality(Quality quality) { m_quality = quality; }
        inline Quality getQuality(void) const { return m_quality; }
        inline ResourceType getResourceType(void) const { return m_resType; }
        inline std::string const &getCurrentFilePath(void) const { return base_type::getFilePath(this->m_quality); }

    public:
        inline void setLastAccess(time_t lastAccess) { m_lastAccess = lastAccess; }
        inline time_t getLastAccess(void) const { return m_lastAccess; }

        virtual bool operator<(Resource &container)
        {
            if (getPriority() < container.getPriority())
                return true;
            else if (getPriority() > container.getPriority())
                return false;
            else
            {
                if (m_lastAccess < container.getLastAccess())
                    return true;
                else if (m_lastAccess > container.getLastAccess())
                    return false;
                else
                {
                    if (getSize() < container.getSize())
                        return true;
                    else
                        return false;
                }
            }
            return false;
        }

        virtual bool operator>(Resource &container)
        {
            if (getPriority() < container.getPriority())
                return false;
            else if (getPriority() > container.getPriority())
                return true;
            else
            {
                if (m_lastAccess < container.getLastAccess())
                    return false;
                else if (m_lastAccess > container.getLastAccess())
                    return true;
                else
                {
                    if (getSize() < container.getSize())
                        return false;
                    else
                        return true;
                }
            }
            return false;
        }

    protected:
        ResourcePriority m_priority;
        Quality m_quality;
        ResourceType m_resType;
        time_t m_lastAccess;
        size_t m_size;
        bool m_isReady;
    }; //# class Resource

} //> namespace resource

#endif //> FG_INC_RESOURCE