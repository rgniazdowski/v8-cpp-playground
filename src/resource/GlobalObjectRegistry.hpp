#pragma once
#ifndef FG_INC_RESOURCE_GLOBAL_OBJECT_REGISTRY
#define FG_INC_RESOURCE_GLOBAL_OBJECT_REGISTRY

#include <Singleton.hpp>
#include <resource/DataManager.hpp>
#include <resource/ManagedObject.hpp>
#include <unordered_map>

namespace resource
{
    class GlobalObjectRegistry : public fg::Singleton<GlobalObjectRegistry>
    {
    protected:
        struct Wrapped
        {
            util::HandleHelper::Unpacked unpacked;
            uint64_t handle;
            void *pointer;
        };
        using RegistryMap = std::unordered_map<uint64_t, Wrapped>;

        GlobalObjectRegistry() {}
        ~GlobalObjectRegistry() {}

    public:
        using self_type = GlobalObjectRegistry;
        friend class fg::Singleton<self_type>;
        /// Map tag id to a wrapped handle manager (contains wrapped 'dereference' functions)
        using DataManagersMap = std::unordered_map<uint8_t, WrappedDataManager>;

    protected:
        template <typename TUserType>
        std::remove_pointer_t<TUserType> *dereference(uint64_t handle)
        {
            using data_type = std::remove_pointer_t<TUserType>;
            static_assert(std::is_void_v<data_type> || (!std::is_void_v<data_type> && std::is_base_of_v<resource::ManagedObjectBase, data_type>), "TUserType template parameter type needs to be derived from ManagedObjectBase");
            auto manager = getDataManager(util::HandleHelper::unpack(handle).tag);
            return !manager ? nullptr : manager->dereference<TUserType>(handle);
        }

        template <typename TUserType>
        std::remove_pointer_t<TUserType> *dereference(const std::string &nameTag)
        {
            using data_type = std::remove_pointer_t<TUserType>;
            static_assert(std::is_void_v<data_type> || (!std::is_void_v<data_type> && std::is_base_of_v<resource::ManagedObjectBase, data_type>), "TUserType template parameter type needs to be derived from ManagedObjectBase");
            for (auto &it : m_dataManagers)
            {
                // cannot retrieve
                auto data = it.second.dereference<data_type>(nameTag);
                if (data != nullptr)
                    return data;
            }
            return nullptr;
        }

        template <typename TUserType>
        std::remove_pointer_t<TUserType> *dereference(util::NamedHandle &nameTag)
        {
            using data_type = std::remove_pointer_t<TUserType>;
            static_assert(std::is_void_v<data_type> || (!std::is_void_v<data_type> && std::is_base_of_v<resource::ManagedObjectBase, data_type>), "TUserType template parameter type needs to be derived from ManagedObjectBase");
            auto manager = getDataManager(nameTag.getTag());
            return !manager ? nullptr : manager->dereference<data_type>(nameTag);
        }

    public:
        template <typename THandleType>
        bool addDataManager(DataManagerBase<THandleType> *pManager)
        {
            using handle_type = THandleType;
            using tag_type = typename handle_type::tag_type;
            if (hasDataManager(tag_type::id()))
                return false;
            if (hasDataManager(pManager))
                return false;
            auto wrapped = WrappedDataManager::wrap<THandleType>(pManager);
            m_dataManagers.emplace(tag_type::id(), wrapped);
            return true;
        }

        bool hasDataManager(uint8_t tag) const { return m_dataManagers.find(tag) != m_dataManagers.end(); }

        template <typename THandleType>
        bool hasDataManager(DataManagerBase<THandleType> *pManager) const
        {
            using manager_type = DataManagerBase<THandleType>;
            for (auto &it : m_dataManagers)
            {
                auto hm = static_cast<manager_type *>(it.second.getManager());
                if (hm == pManager)
                    return true;
            }
            return false;
        }

        const WrappedDataManager *getDataManager(uint8_t tag) const
        {
            if (!hasDataManager(tag))
                return nullptr;
            auto found = m_dataManagers.find(tag);
            return found->second.self();
        }

        template <typename TUserType>
        bool add(TUserType *data)
        {
            if (has<TUserType>(data))
                return false;
            auto &handle = data->getHandleBase();
            m_registry.emplace(handle.getHandle(), Wrapped{util::HandleHelper::unpack(handle), handle.getHandle(), data});
            return true;
        }

        template <typename TUserType>
        std::remove_pointer_t<TUserType> *find(const util::HandleBase &handle) { return find<TUserType>(handle.getHandle()); }

        template <typename TUserType>
        std::remove_pointer_t<TUserType> *find(uint64_t handle)
        {
            using data_type = std::remove_pointer_t<TUserType>;
            data_type *found = nullptr;
            auto it = m_registry.find(handle);
            if (it != m_registry.end())
                found = static_cast<data_type *>(it->second.pointer);
            if (!found)
                found = this->dereference<data_type>(handle);
            return found;
        }

        template <typename TUserType>
        bool has(TUserType *data)
        {
            auto handle = data->getHandleBase().getHandle();
            auto inRegistry = m_registry.find(handle) != m_registry.end();
            if (inRegistry)
                return true;
            return this->dereference<void>(handle) != nullptr;
        }

        bool has(uint64_t handle)
        {
            auto inRegistry = m_registry.find(handle) != m_registry.end();
            if (inRegistry)
                return true;
            return this->dereference<void>(handle) != nullptr;
        }

        bool has(const util::HandleBase &handle)
        {
            auto inRegistry = m_registry.find(handle.getHandle()) != m_registry.end();
            if (inRegistry)
                return true;
            return this->dereference<void>(handle.getHandle()) != nullptr;
        }

        bool has(const std::string &nameTag)
        {
            return this->dereference<void>(nameTag) != nullptr;
        }

        bool has(util::NamedHandle &nameTag)
        {
            auto inRegistry = m_registry.find(nameTag.getHandle()) != m_registry.end();
            if (inRegistry)
                return true;
            return this->dereference<void>(nameTag) != nullptr;
        }

    protected:
        DataManagersMap m_dataManagers;
        RegistryMap m_registry;
    }; //# class GlobalObjectRegistry
} //> namespace resource

namespace util
{
    template <typename TUserType>
    struct convert
    {
        using user_type = std::remove_pointer_t<TUserType>;
        static user_type *convertToPointer(void *data, uint64_t handle)
        {
            if (data != nullptr)
                return static_cast<user_type *>(data);
            auto registry = resource::GlobalObjectRegistry::instance();
            return registry->find<user_type>(handle);
        }
    };
} //> namespace util

#endif //> FG_INC_RESOURCE_GLOBAL_OBJECT_REGISTRY