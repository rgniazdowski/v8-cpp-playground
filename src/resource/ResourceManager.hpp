#pragma once
#ifndef FG_INC_RESOURCE_MANAGER
#define FG_INC_RESOURCE_MANAGER

#include <util/Vector.hpp>
#include <Manager.hpp>
#include <resource/DataManager.hpp>
#include <resource/Resource.hpp>

#include <util/Dirent.hpp>
#include <util/Tag.hpp>
#include <util/HandleManager.hpp>
#include <util/AbstractFactory.hpp>

#include <iostream>

namespace resource
{

    class ResourceFactory : public util::AbstractFactory<ResourceType, Resource>
    {
    public:
        typedef util::AbstractFactory<ResourceType, Resource> base_type;
        typedef ResourceFactory self_type;

    public:
        ResourceFactory() : base_type() {}
        virtual ~ResourceFactory() { destroy(); }

    }; //> class ResourceFactory

    using ResourceManagerTag = ::util::Tag<ResourceManager>;

    class ResourceManager : public DataManagerBase<ResourceHandle>
    {
    public:
        using base_type = DataManagerBase<ResourceHandle>;
        using self_type = ResourceManager;
        using handle_type = ResourceHandle;
        using tag_type = ResourceManagerTag;

    protected:
        using HandleVec = ::util::Vector<ResourceHandle>;
        using HandleVecItor = HandleVec::iterator;

    public:
        ResourceManager(base::ManagerBase *pEventMgr = nullptr);
        virtual ~ResourceManager();

    public:
        virtual bool destroy(void) override;
        virtual bool initialize(void) override;

        void setEventManager(base::ManagerBase *pEventMgr) { m_pEventMgr = pEventMgr; }
        base::ManagerBase *getEventManager(void) const { return m_pEventMgr; }
        ResourceFactory *getResourceFactory(void) const { return m_resourceFactory.get(); }
        bool setMaximumMemory(size_t nMaxSize);
        size_t getMaximumMemory(void) const { return m_nMaximumMemory; }
        bool reserveMemory(size_t nMem);

        void goToBegin(void) { m_currentResource = getDataVector().begin(); }

        Resource *getCurrentResource(void) { return (!isValid() ? NULL : const_cast<Resource *>((*m_currentResource).data)); }

        bool isValid(void) { return (m_currentResource != getDataVector().end()); }

        bool goToNext(void)
        {
            m_currentResource++;
            return isValid();
        }
        bool goToNext(ResourceType resType);
        bool goToNext(const ResourceType *resTypes, int n);
        bool goToNext(ResourceType resType, Quality quality);

        bool getResourceNames(util::StringVector &strVec,
                              ResourceType resType = resource::INVALID);
        bool getResourceNames(util::StringVector &strVec,
                              const ResourceType *resTypes, unsigned int n);

        bool insertResource(Resource *pResource);

    protected:
        // bool insertResourceGroup(const ResourceHandle &rhUniqueID, Resource *pResource);
        Resource *refreshResource(Resource *pResource);

    public:
        virtual Resource *request(std::string_view info, const ResourceType forcedType = resource::AUTO);

        using base_type::remove;
        virtual bool remove(Resource *pResource) override;

        virtual bool dispose(Resource *pResource);
        inline bool dispose(const ResourceHandle &rhUniqueID) { return dispose(base_type::get(rhUniqueID)); }
        inline bool dispose(const std::string &nameTag) { return dispose(base_type::get(nameTag)); }
        inline bool dispose(util::NamedHandle &nameTag) { return dispose(base_type::get(nameTag)); }

        virtual inline Resource *get(const ResourceHandle &rhUniqueID) override { return refreshResource(base_type::get(rhUniqueID)); }
        virtual inline Resource *get(const std::string &nameTag) override { return refreshResource(base_type::get(nameTag)); }
        virtual inline Resource *get(util::NamedHandle &nameTag) override { return refreshResource(base_type::get(nameTag)); }

        // Resource *lockResource(const ResourceHandle &rhUniqueID);
        // bool lockResource(Resource *pResource);
        // Resource *unlockResource(const ResourceHandle &rhUniqueID);
        // bool unlockResource(Resource *pResource);

        bool checkForOverallocation(void);

    protected:
        void refreshMemory(void);

        inline void resetMemory(void) { m_nCurrentUsedMemory = 0; }
        inline void addMemory(size_t nMem) { m_nCurrentUsedMemory += nMem; }
        inline void removeMemory(size_t nMem) { m_nCurrentUsedMemory -= nMem; }

    private:
        util::Dirent m_dataDir;
        DataVecItor m_currentResource;
        HandleVec m_resourceGroupHandles;
        std::unique_ptr<ResourceFactory> m_resourceFactory;
        base::ManagerBase *m_pEventMgr;
        size_t m_nCurrentUsedMemory;
        size_t m_nMaximumMemory;
        bool m_bResourceReserved;
    }; //# class ResourceManager

} //> namespace resource

#endif //> FG_INC_RESOURCE_MANAGER