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

    public:
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
            auto unpacked = HandleHelper::unpack(handle);
            std::remove_pointer_t<TUserType> *found = nullptr;
            auto it = m_registry.find(handle);
            if (it != m_registry.end())
                found = static_cast<std::remove_pointer_t<TUserType> *>(it->second.pointer);
            return found;
        }

        template <typename TUserType>
        bool has(TUserType *data)
        {
            auto handle = data->getHandle().getHandle();
            return m_registry.find(handle) != m_registry.end();
        }

        bool has(uint64_t handle)
        {
            return m_registry.find(handle) != m_registry.end();
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