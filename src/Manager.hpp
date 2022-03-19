#pragma once
#ifndef FG_INC_MANAGER
#define FG_INC_MANAGER

#include <cstdint>
#include <mutex>

#include <Singleton.hpp>
#include <util/Handle.hpp>
#include <util/SimpleThread.hpp>

namespace base
{
    class ManagerBase : public util::ObjectWithIdentifier, protected util::SimpleThread
    {
    protected:
        inline static uint8_t s_gid = 0;

    public:
        virtual bool destroy(void) = 0;
        virtual bool initialize(void) = 0;
        inline uint32_t getManagerId(void) const { return m_managerId; };

        constexpr bool isInit(void) const noexcept { return m_init; }

        ManagerBase(uint32_t managerId = 0, uint64_t identifier = 0) : m_init(false),
                                                                       m_managerId(managerId),
                                                                       m_instanceId(identifier),
                                                                       m_thread(*this) {}
        virtual ~ManagerBase();

        inline bool startThread(void) { return this->start(); }
        inline bool signalThread(void) { return this->wakeup(); }
        inline bool joinThread(void) { return this->join(); }
        inline void stopThread(void) { this->stop(); }

        using util::SimpleThread::isJoinable;
        using util::SimpleThread::isRunning;
        using util::SimpleThread::isWakeable;

        ManagerBase(const ManagerBase &other) = delete;
        ManagerBase(ManagerBase &&other) = delete;
        ManagerBase &operator=(const ManagerBase &other) = delete;
        ManagerBase &operator=(ManagerBase &&other) = delete;

        inline uint64_t getIdentifier(void) const override { return m_instanceId; };

    protected:
        bool m_init;
        util::SimpleThread &m_thread;

    private:
        uint32_t m_managerId;
        uint64_t m_instanceId;
    }; //# class ManagerBase
    //#-----------------------------------------------------------------------------------

    template <class TManagerType>
    class Manager : virtual public ManagerBase
    {
    private:
        inline static uint64_t s_instanceId = 0;
        static uint64_t aquireInstanceId(void) { return ++s_instanceId; }

    public:
        using self_type = Manager<TManagerType>;

    public:
        Manager() : ManagerBase(self_type::id(), self_type::aquireInstanceId()) {}
        virtual ~Manager() {}

        /**
         * @brief Acquire and retrieve unique identifier for a given manager type (Manager Id),
         * this is very similar to an enum value - it can help differentiate managers.
         *
         * @return uint32_t Unique number identifier that starts from 1.
         */
        static uint32_t id()
        {
            if (!s_idset)
                s_lid = (s_idset ? s_gid : ++s_gid);
            s_idset = true;
            return s_lid;
        }

    private:
        inline static bool s_idset = false;
        inline static uint32_t s_lid = 0;
    }; //# class Manager
    //#-----------------------------------------------------------------------------------

    class ManagerRegistry : public fg::Singleton<ManagerRegistry>
    {
    protected:
        using base_type = fg::Singleton<ManagerRegistry>;
        struct Wrapped
        {
            uint32_t managerId;
            ManagerBase *manager;
            Wrapped(uint32_t _mId, ManagerBase *_mgr) : managerId(_mId), manager(_mgr) {}
        };
        using RegistryMap = std::unordered_map<uint32_t, Wrapped>;

        ManagerRegistry() : base_type(), m_registry(), m_mutex() {}
        ~ManagerRegistry() {}

    public:
        using self_type = ManagerRegistry;
        friend class fg::Singleton<self_type>;

        template <typename TManagerType>
        bool add(TManagerType *pManager)
        {
            static_assert(std::is_base_of_v<ManagerBase, TManagerType>,
                          "TManagerType template parameter type needs to be derived from ManagerBase");
            // unique manager id based on type of class (this is not identifier, instance id)
            const auto managerId = pManager->getManagerId();
            const std::lock_guard<std::mutex> lock(m_mutex);
            m_registry.emplace(managerId, Wrapped{managerId, pManager});
            return true;
        }

        template <typename TManagerType>
        std::remove_pointer_t<TManagerType> *get(uint32_t managerId = 0) const
        {
            static_assert(std::is_base_of_v<ManagerBase, TManagerType>,
                          "TManagerType template parameter type needs to be derived from ManagerBase");
            using manager_type = std::remove_pointer_t<TManagerType>;
            const std::lock_guard<std::mutex> lock(m_mutex);
            managerId = manager_type::id(); // ignores the input because data type is specified
            auto found = m_registry.find(managerId);
            if (found == m_registry.end())
                return nullptr;
            return static_cast<manager_type *>(found->second.manager);
        }

        template <>
        ManagerBase *get<ManagerBase>(uint32_t managerId) const
        {
            if (!managerId)
                return nullptr;
            const std::lock_guard<std::mutex> lock(m_mutex);
            auto found = m_registry.find(managerId);
            if (found == m_registry.end())
                return nullptr;
            return found->second.manager;
        }

        template <typename TManagerType>
        bool has(void) const
        {
            static_assert(std::is_base_of_v<ManagerBase, TManagerType>,
                          "TManagerType template parameter type needs to be derived from ManagerBase");
            using manager_type = std::remove_pointer_t<TManagerType>;
            const std::lock_guard<std::mutex> lock(m_mutex);
            return m_registry.find(manager_type::id()) != m_registry.end();
        }

        bool has(uint32_t managerId) const
        {
            const std::lock_guard<std::mutex> lock(m_mutex);
            return m_registry.find(managerId) != m_registry.end();
        }

        bool has(ManagerBase *pManager) const
        {
            const auto managerId = pManager->getManagerId();
            const std::lock_guard<std::mutex> lock(m_mutex);
            return m_registry.find(managerId) != m_registry.end();
        }

        template <typename TManagerType>
        bool remove(void)
        {
            static_assert(std::is_base_of_v<ManagerBase, TManagerType>,
                          "TManagerType template parameter type needs to be derived from ManagerBase");
            using manager_type = std::remove_pointer_t<TManagerType>;
            const std::lock_guard<std::mutex> lock(m_mutex);
            if (m_registry.find(manager_type::id()) != m_registry.end())
            {
                m_registry.erase(manager_type::id());
                return true;
            }
            return false;
        }

        bool remove(uint32_t managerId)
        {
            const std::lock_guard<std::mutex> lock(m_mutex);
            if (m_registry.find(managerId) != m_registry.end())
            {
                m_registry.erase(managerId);
                return true;
            }
            return false;
        }

        bool remove(ManagerBase *pManager)
        {
            const auto managerId = pManager->getManagerId();
            const std::lock_guard<std::mutex> lock(m_mutex);
            if (m_registry.find(managerId) != m_registry.end())
            {
                m_registry.erase(managerId);
                return true;
            }
            return false;
        }

        bool isJoinable(uint32_t managerId) const
        {
            auto manager = get<ManagerBase>(managerId);
            if (!manager)
                return false;
            return manager->isJoinable();
        }

        bool isRunning(uint32_t managerId) const
        {
            auto manager = get<ManagerBase>(managerId);
            if (!manager)
                return false;
            return manager->isRunning();
        }

        bool isWakeable(uint32_t managerId) const
        {
            auto manager = get<ManagerBase>(managerId);
            if (!manager)
                return false;
            return manager->isWakeable();
        }

        void signalAll(void)
        {
            for (auto &it : m_registry)
                it.second.manager->signalThread();
        }

        RegistryMap &getRegistryDirect(void) { return m_registry; }

    protected:
        RegistryMap m_registry;
        mutable std::mutex m_mutex;
    }; //# class ManagerRegistry
    //#-----------------------------------------------------------------------------------

    inline ManagerBase::~ManagerBase()
    {
        ManagerRegistry::instance()->remove(this->getManagerId());
    }
    //>-----------------------------------------------------------------------------------

} //> namespace base

#endif //> FG_INC_MANAGER