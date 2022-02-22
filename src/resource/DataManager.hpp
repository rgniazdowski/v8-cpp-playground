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
        virtual ~DataManagerBase()
        {
            clear();
        }

    protected:
        virtual void clear(void)
        {
            handle_mgr_type::releaseAllHandles();
        }

    public:
        virtual bool destroy(void) = 0;
        virtual bool initialize(void) = 0;

        virtual bool insert(data_type *pData, std::string_view nameTag);

        virtual bool rename(data_type *pData, std::string_view newName);
        virtual bool rename(const handle_type &dhUniqueID, std::string_view newName);

        virtual bool remove(data_type *pData);
        virtual bool remove(const handle_type &dhUniqueID);
        virtual bool remove(std::string_view nameTag);

        virtual bool destroyData(data_type *&pData);
        virtual bool destroyData(const handle_type &dhUniqueID);
        virtual bool destroyData(std::string_view nameTag);

        virtual data_type *get(const handle_type &dhUniqueID);
        virtual data_type *get(std::string_view nameTag);

        virtual data_type *request(std::string_view info) = 0;

        virtual bool isManaged(data_type *pData);
        virtual bool isManaged(const handle_type &dhUniqueID);

        virtual bool isManaged(std::string_view nameTag);
    }; //# class DataManagerBase
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
    if (!handle_mgr_type::setupName(nameTag, dhUniqueID))
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
bool resource::DataManagerBase<THandleType>::rename(const handle_type &dhUniqueID, std::string_view nameTag)
{
    return true;
}

template <typename THandleType>
bool resource::DataManagerBase<THandleType>::remove(data_type *pData)
{
    if (!self_type::isManaged(pData))
    {
        return false;
    }
    return handle_mgr_type::releaseHandle(pData->getHandle());
}

template <typename THandleType>
bool resource::DataManagerBase<THandleType>::remove(const handle_type &dhUniqueID)
{
    data_type *pData = self_type::get(dhUniqueID);
    if (!pData)
        return false;
    return handle_mgr_type::releaseHandle(pData->getHandle());
}

template <typename THandleType>
bool resource::DataManagerBase<THandleType>::remove(std::string_view nameTag)
{
    data_type *pData = self_type::get(nameTag);
    if (!pData)
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
bool resource::DataManagerBase<THandleType>::destroyData(std::string_view nameTag)
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
typename resource::DataManagerBase<THandleType>::data_type *resource::DataManagerBase<THandleType>::get(std::string_view nameTag)
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
    if (FG_IS_INVALID_HANDLE(pData->getHandle()) || !handle_mgr_type::isHandleValid(pData->getHandle()))
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

template <typename THandleType>
bool resource::DataManagerBase<THandleType>::isManaged(const handle_type &dhUniqueID)
{
    data_type *pData = self_type::get(dhUniqueID);
    return bool(pData != NULL);
}

template <typename THandleType>
bool resource::DataManagerBase<THandleType>::isManaged(std::string_view nameTag)
{
    data_type *pData = self_type::get(nameTag);
    return bool(pData != NULL);
}

#endif //> FG_INC_DATA_MANAGER