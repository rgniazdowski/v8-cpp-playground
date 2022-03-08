#pragma once
#ifndef FG_INC_MANAGER
#define FG_INC_MANAGER

#include <cstdint>

namespace base
{
    class ManagerBase
    {
    protected:
        inline static uint8_t s_gid = 0;

    public:
        virtual bool destroy(void) = 0;
        virtual bool initialize(void) = 0;
        virtual uint32_t getManagerId(void) const = 0;

        constexpr bool isInit(void) const noexcept { return m_init; }

    protected:
        bool m_init;
        uint32_t m_managerId;
    }; //# class ManagerBase

    template <class TManagerType>
    class Manager : virtual protected ManagerBase
    {
    public:
        using self_type = Manager<TManagerType>;

    public:
        Manager() { self_type::id(); }
        virtual ~Manager() {}

        static uint32_t id()
        {
            if (!s_idset)
                s_lid = (s_idset ? s_gid : ++s_gid);
            s_idset = true;
            return s_lid;
        }

        uint32_t getManagerId(void) const override { return self_type::id(); }

    private:
        inline static bool s_idset = false;
        inline static uint8_t s_lid = 0;

    }; //# class Manager
} //> namespace base

#endif //> FG_INC_MANAGER