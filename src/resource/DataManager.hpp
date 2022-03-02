#pragma once
#ifndef FG_INC_DATA_MANAGER
#define FG_INC_DATA_MANAGER

#include <Manager.hpp>
#include <util/Vector.hpp>
#include <util/Handle.hpp>
#include <util/NamedHandle.hpp>
#include <util/HandleManager.hpp>
#include <resource/ManagedObject.hpp>

namespace resource
{
    template <typename THandleType>
    class DataManagerBase : public base::Manager<DataManagerBase<THandleType>>,
                            protected util::HandleManager<THandleType>
    {
    public:
        using handle_type = THandleType;
        using tag_type = typename handle_type::tag_type;
        using data_type = typename tag_type::user_type;

        using self_type = DataManagerBase<handle_type>;
        using manager_type = base::Manager<self_type>;
        using handle_mgr_type = util::HandleManager<handle_type>;

        static_assert(std::is_base_of<util::HandleBase, handle_type>::value, "THandleType template parameter type needs to be derived from HandleBase");
        static_assert(std::is_base_of<ManagedObjectBase, data_type>::value, "TDataType template parameter type needs to be derived from ManagedObjectBase");

    public:
        DataManagerBase() : manager_type(), handle_mgr_type() {}
        virtual ~DataManagerBase() {}

    public:
        virtual bool destroy(void) = 0;
        virtual bool initialize(void) = 0;

        virtual bool insert(data_type *pData, std::string_view nameTag);

        virtual bool rename(data_type *pData, std::string_view newName);
        bool rename(const handle_type &dhUniqueID, std::string_view newName) { return rename(self_type::get(dhUniqueID), newName); }

        virtual bool remove(data_type *pData);
        bool remove(const handle_type &dhUniqueID) { return remove(self_type::get(dhUniqueID)); }
        bool remove(const std::string &nameTag) { return remove(self_type::get(nameTag)); }
        bool remove(util::NamedHandle &nameTag) { return remove(self_type::get(nameTag)); }

        virtual bool destroyData(data_type *&pData);
        bool destroyData(const handle_type &dhUniqueID);
        bool destroyData(const std::string &nameTag);
        bool destroyData(util::NamedHandle &nameTag);

        virtual data_type *get(const handle_type &dhUniqueID);
        virtual data_type *get(const std::string &nameTag);
        virtual data_type *get(util::NamedHandle &nameTag);

        virtual bool isManaged(data_type *pData);
        inline bool isManaged(const handle_type &dhUniqueID) { return isManaged(self_type::get(dhUniqueID)); }
        inline bool isManaged(const std::string &nameTag) { return isManaged(self_type::get(nameTag)); }
        inline bool isManaged(util::NamedHandle &nameTag) { return isManaged(self_type::get(nameTag)); }
    }; //# class DataManagerBase

    class WrappedDataManager
    {
    public:
        using self_type = WrappedDataManager;

        WrappedDataManager(const self_type &other)
        {
            m_getDataManager = other.m_getDataManager;
            m_dereferenceHandle = other.m_dereferenceHandle;
            m_dereferenceString = other.m_dereferenceString;
            m_dereferenceNamedHandle = other.m_dereferenceNamedHandle;
        }

        WrappedDataManager(self_type &&other)
        {
            m_getDataManager = std::move(other.m_getDataManager);
            m_dereferenceHandle = std::move(other.m_dereferenceHandle);
            m_dereferenceString = std::move(other.m_dereferenceString);
            m_dereferenceNamedHandle = std::move(other.m_dereferenceNamedHandle);
        }

        ~WrappedDataManager() {}

    protected:
        WrappedDataManager() {}

    public:
        template <typename THandleType>
        static self_type wrap(DataManagerBase<THandleType> *pManager)
        {
            using handle_type = THandleType;
            using tag_type = typename handle_type::tag_type;
            using data_type = typename tag_type::user_type;
            WrappedDataManager self;
            // custom lambdas are used to obfuscate the real type used underneath -
            // DataManagerBase is a template class, we need to call dereference on a given
            // manager without specifying original template parameters.
            // This forces to static cast back and forth to void, but it is guaranteed that
            // return type is a pointer and a proper handle manager was wrapped at all.
            self.m_dereferenceHandle = [pManager](uint64_t handle)
            { return static_cast<void *>(pManager->get(THandleType(handle))); };
            self.m_dereferenceString = [pManager](const std::string &nameTag)
            { return static_cast<void *>(pManager->get(nameTag)); };
            self.m_dereferenceNamedHandle = [pManager](util::NamedHandle &nameTag)
            { return static_cast<void *>(pManager->get(nameTag)); };
            self.m_getDataManager = [pManager]()
            { return static_cast<void *>(pManager); };
            return self;
        }

    public:
        template <typename TUserType, typename THandleType>
        TUserType *dereference(const THandleType &handle) const { return static_cast<TUserType *>(m_dereferenceHandle(handle.getHandle())); }

        template <typename TUserType>
        TUserType *dereference(uint64_t handle) const { return static_cast<TUserType *>(m_dereferenceHandle(handle)); }

        template <typename TUserType>
        TUserType *dereference(const std::string &nameTag) const { return static_cast<TUserType *>(m_dereferenceString(nameTag)); }

        template <typename TUserType>
        TUserType *dereference(util::NamedHandle &nameTag) const { return static_cast<TUserType *>(m_dereferenceNamedHandle(nameTag)); }

        void *getManager(void) const { return m_getDataManager(); }

        self_type const *self(void) const { return this; }

        self_type *self(void) { return this; }

    private:
        using DereferenceHandle = std::function<void *(uint64_t)>;
        using DereferenceString = std::function<void *(const std::string &)>;
        using DereferenceNamedHandle = std::function<void *(util::NamedHandle &)>;
        using GetDataManager = std::function<void *()>;

        GetDataManager m_getDataManager;
        DereferenceHandle m_dereferenceHandle;
        DereferenceString m_dereferenceString;
        DereferenceNamedHandle m_dereferenceNamedHandle;
    }; //# class WrappedHandleManager
} //> namespace resource

template <typename THandleType>
bool resource::DataManagerBase<THandleType>::insert(data_type *pData, std::string_view nameTag)
{
    if (!pData)
    {
        // FG_MessageSubsystem->reportWarning(tag_type::name(), FG_ERRNO_RESOURCE_PARAMETER_NULL, FG_MSG_IN_FUNCTION);
        return false;
    }
    handle_type dhUniqueID;
    if (handle_mgr_type::isDataManaged(pData))
    {
        // FG_MessageSubsystem->reportError(tag_type::name(), FG_ERRNO_RESOURCE_ALREADY_MANAGED, FG_MSG_IN_FUNCTION);
        return false;
    }
    if (!pData->getHandle().isNull())
    {
        // Resource has already initialized handle
        // FG_MessageSubsystem->reportError(tag_type::name(), FG_ERRNO_RESOURCE_INITIALIZED_HANDLE, FG_MSG_IN_FUNCTION);
        return false;
    }
    // Acquire next valid resource handle
    // Insert the resource into the current catalog
    if (!handle_mgr_type::acquireHandle(dhUniqueID, pData))
    {
        // Could not aquire handle for the resource
        // FG_MessageSubsystem->reportError(tag_type::name(), FG_ERRNO_RESOURCE_ACQUIRE_HANDLE, FG_MSG_IN_FUNCTION);
        return false;
    }
    // This is important - on addition need to update the handle
    pData->setHandle(dhUniqueID);
    pData->setName(nameTag);
    if (!handle_mgr_type::setupName(std::string(nameTag), dhUniqueID))
    {
        // Could not setup handle string tag/name for the resource
        // FG_MessageSubsystem->reportError(tag_type::name(), FG_ERRNO_RESOURCE_SETUP_HANDLE_NAME, FG_MSG_IN_FUNCTION);
        return false;
    }
#if defined(FG_DEBUG)
    // FG_MessageSubsystem->reportSuccess(tag_type::name(), FG_ERRNO_RESOURCE_OK, "Inserted data with name[%s], index[%u], magic[%u], handle[%u], hash[%u]",
    //                                   nameTag.c_str(), pData->getName().getIndex(), dhUniqueID.getMagic(), dhUniqueID.getHandle(), pData->getName().getHash());
#endif
    return true;
}

template <typename THandleType>
bool resource::DataManagerBase<THandleType>::rename(data_type *pData, std::string_view nameTag)
{
    return true;
}

template <typename THandleType>
bool resource::DataManagerBase<THandleType>::remove(data_type *pData)
{
    if (!self_type::isManaged(pData))
        return false;
    return handle_mgr_type::releaseHandle(pData->getHandle());
}

template <typename THandleType>
bool resource::DataManagerBase<THandleType>::destroyData(data_type *&pData)
{
    if (!remove(pData))
    {
        // FG_MessageSubsystem->reportError(tag_type::name(), FG_ERRNO_RESOURCE_REMOVE);
        return false;
    }
    delete pData;
    pData = nullptr;
    return true;
}

template <typename THandleType>
bool resource::DataManagerBase<THandleType>::destroyData(const handle_type &dhUniqueID)
{
    data_type *pData = handle_mgr_type::dereference(dhUniqueID);
    if (!remove(pData))
    {
        // FG_MessageSubsystem->reportError(tag_type::name(), FG_ERRNO_RESOURCE_REMOVE);
        return false;
    }
    delete pData;
    return true;
}

template <typename THandleType>
bool resource::DataManagerBase<THandleType>::destroyData(const std::string &nameTag)
{
    data_type *pData = handle_mgr_type::dereference(nameTag);
    if (!remove(pData))
    {
        // FG_MessageSubsystem->reportError(tag_type::name(), FG_ERRNO_RESOURCE_REMOVE);
        return false;
    }
    delete pData;
    return true;
}

template <typename THandleType>
bool resource::DataManagerBase<THandleType>::destroyData(util::NamedHandle &nameTag)
{
    data_type *pData = handle_mgr_type::dereference(nameTag);
    if (!remove(pData))
    {
        // FG_MessageSubsystem->reportError(tag_type::name(), FG_ERRNO_RESOURCE_REMOVE);
        return false;
    }
    delete pData;
    return true;
}

template <typename THandleType>
typename resource::DataManagerBase<THandleType>::data_type *resource::DataManagerBase<THandleType>::get(const handle_type &dhUniqueID)
{
    data_type *pData = handle_mgr_type::dereference(dhUniqueID);
    return pData;
}

template <typename THandleType>
typename resource::DataManagerBase<THandleType>::data_type *resource::DataManagerBase<THandleType>::get(const std::string &nameTag)
{
    if (nameTag.empty())
    {
        // FG_MessageSubsystem->reportWarning(tag_type::name(), FG_ERRNO_RESOURCE_NAME_TAG_EMPTY, FG_MSG_IN_FUNCTION);
        return nullptr;
    }
    data_type *pData = handle_mgr_type::dereference(nameTag);
    if (!pData)
    {
        // FG_MessageSubsystem->reportError(tag_type::name(), FG_ERRNO_RESOURCE_NAME_TAG_INVALID, " tag='%s', in function: %s", nameTag.c_str(), __FUNCTION__);
        return nullptr;
    }
    return pData;
}

template <typename THandleType>
typename resource::DataManagerBase<THandleType>::data_type *resource::DataManagerBase<THandleType>::get(util::NamedHandle &nameTag)
{
    if (nameTag.empty())
    {
        // FG_MessageSubsystem->reportWarning(tag_type::name(), FG_ERRNO_RESOURCE_NAME_TAG_EMPTY, FG_MSG_IN_FUNCTION);
        return nullptr;
    }
    data_type *pData = handle_mgr_type::dereference(nameTag);
    if (!pData)
    {
        // FG_MessageSubsystem->reportError(tag_type::name(), FG_ERRNO_RESOURCE_NAME_TAG_INVALID, " tag='%s', in function: %s", nameTag.c_str(), __FUNCTION__);
        return nullptr;
    }
    return pData;
}

template <typename THandleType>
bool resource::DataManagerBase<THandleType>::isManaged(data_type *pData)
{
    if (!pData)
    {
        // FG_MessageSubsystem->reportWarning(tag_type::name(), FG_ERRNO_RESOURCE_PARAMETER_NULL, FG_MSG_IN_FUNCTION);
        return false;
    }
    if (pData->getHandle().isNull() || !handle_mgr_type::isHandleValid(pData->getHandle()))
    {
        // FG_MessageSubsystem->reportError(tag_type::name(), FG_ERRNO_RESOURCE_HANDLE_INVALID, FG_MSG_IN_FUNCTION);
        return false;
    }
    if (!handle_mgr_type::isDataManaged(pData))
    {
        // FG_MessageSubsystem->reportWarning(tag_type::name(), FG_ERRNO_RESOURCE_NOT_MANAGED, FG_MSG_IN_FUNCTION);
        return false;
    }
    return true;
}

#endif //> FG_INC_DATA_MANAGER