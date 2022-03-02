#pragma once
#ifndef FG_INC_UTIL_GLOBAL_OBJECT_REGISTRY
#define FG_INC_UTIL_GLOBAL_OBJECT_REGISTRY

#include <Singleton.hpp>
#include <util/HandleManager.hpp>
#include <unordered_map>

namespace util
{
    class GlobalObjectRegistry : public fg::Singleton<GlobalObjectRegistry>
    {
    protected:
        struct Wrapped
        {
            HandleHelper::Unpacked unpacked;
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
        using HandleManagersMap = std::unordered_map<uint8_t, WrappedHandleManager>;

    protected:
        template <typename TUserType, typename THandleType>
        TUserType *dereference(const THandleType &handle)
        {
            auto manager = getHandleManager(handle.getTag());
            return !manager ? nullptr : manager->dereference<TUserType>(handle);
        }

        template <typename TUserType>
        TUserType *dereference(uint64_t handle)
        {
            auto manager = getHandleManager(HandleHelper::unpack(handle).tag);
            return !manager ? nullptr : manager->dereference<TUserType>(handle);
        }

        template <typename TUserType>
        TUserType *dereference(const std::string &nameTag)
        {
            for (auto &it : m_handleManagers)
            {
                // cannot retrieve
                auto data = it.second.dereference(nameTag);
                if (data != nullptr)
                    return data;
            }
            return nullptr;
        }

        template <typename TUserType>
        TUserType *dereference(NamedHandle &nameTag)
        {
            auto manager = getHandleManager(nameTag.getTag());
            return !manager ? nullptr : manager->dereference<TUserType>(nameTag);
        }

    public:
        template <typename THandleType>
        bool addHandleManager(HandleManager<THandleType> *pManager)
        {
            using handle_type = THandleType;
            using tag_type = typename handle_type::tag_type;
            using data_type = typename tag_type::user_type;
            if (hasHandleManager(tag_type::id()))
                return false;
            if (hasHandleManager(pManager))
                return false;
            auto wrapped = WrappedHandleManager::wrap<THandleType>(pManager);
            m_handleManagers.emplace(tag_type::id(), wrapped);
            return true;
        }

        bool hasHandleManager(uint8_t tag) const { return m_handleManagers.find(tag) != m_handleManagers.end(); }

        template <typename THandleType>
        bool hasHandleManager(HandleManager<THandleType> *pManager) const
        {
            using manager_type = HandleManager<THandleType>;
            for (auto &it : m_handleManagers)
            {
                auto hm = static_cast<manager_type *>(it.second.getManager());
                if (hm == pManager)
                    return true;
            }
            return false;
        }

        const WrappedHandleManager *getHandleManager(uint8_t tag) const
        {
            if (!hasHandleManager(tag))
                return nullptr;
            auto found = m_handleManagers.find(tag);
            return found->second.self();
        }

        template <typename TUserType>
        bool add(TUserType *data)
        {
            if (has<TUserType>(data))
                return false;
            auto &handle = data->getHandle();
            m_registry.emplace(handle.getHandle(), Wrapped{HandleHelper::unpack(handle), handle.getHandle(), data});
            return true;
        }

        template <typename TUserType>
        std::remove_pointer_t<TUserType> *find(const HandleBase &handle) { find<TUserType>(handle.getHandle()); }

        template <typename TUserType>
        std::remove_pointer_t<TUserType> *find(uint64_t handle)
        {
            std::remove_pointer_t<TUserType> *found = nullptr;
            auto it = m_registry.find(handle);
            if (it != m_registry.end())
                found = static_cast<std::remove_pointer_t<TUserType> *>(it->second.pointer);
            if (!found)
                found = dereference<TUserType>(handle);
            return found;
        }

        template <typename TUserType>
        bool has(TUserType *data)
        {
            auto handle = data->getHandle().getHandle();
            auto inRegistry = m_registry.find(handle) != m_registry.end();
            if (inRegistry)
                return true;
            return dereference<void>(handle) != nullptr;
        }

        bool has(uint64_t handle)
        {
            auto inRegistry = m_registry.find(handle) != m_registry.end();
            if (inRegistry)
                return true;
            return dereference<void>(handle) != nullptr;
        }

        bool has(const HandleBase &handle)
        {
            auto inRegistry = m_registry.find(handle.getHandle()) != m_registry.end();
            if (inRegistry)
                return true;
            return dereference<void>(handle.getHandle()) != nullptr;
        }

    protected:
        HandleManagersMap m_handleManagers;
        RegistryMap m_registry;
    }; //# class GlobalObjectRegistry

    template <typename TUserType>
    struct convert
    {
        using user_type = std::remove_pointer_t<TUserType>;
        static user_type *convertToPointer(void *data, uint64_t handle)
        {
            if (data != nullptr)
                return static_cast<user_type *>(data);
            auto registry = GlobalObjectRegistry::instance();
            return registry->find<user_type>(handle);
        }
    };
} //> namespace util

#endif //> FG_INC_UTIL_GLOBAL_OBJECT_REGISTRY