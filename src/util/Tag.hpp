#pragma once
#ifndef FG_INC_UTIL_TAG
#define FG_INC_UTIL_TAG

#include <typeinfo>
#include <string>
#include <unordered_map>

#include <util/UniversalId.hpp>

namespace util
{
    struct TagBase
    {
    protected:
        inline static uint8_t s_gid = 0;
    };

    template <typename UserClass>
    struct Tag;

    template <>
    struct Tag<void>
    {
    private:
        inline static std::unordered_map<uint32_t, std::string> registry = std::unordered_map<uint32_t, std::string>();

    public:
        static void set(uint32_t id, std::string_view key) { registry[id] = key; }
        static bool has(uint32_t id) { return (registry.find(id) != registry.end()); }
        static bool has(std::string_view key)
        {
            for (auto &it : registry)
            {
                if (it.second.compare(key) == 0)
                    return true;
            }
            return false;
        }
        static std::string_view name(uint32_t id)
        {
            auto it = registry.find(id);
            if (it == registry.end())
                return std::string_view();
            return it->second;
        }
        static uint32_t id(std::string_view key)
        {
            for (auto &it : registry)
            {
                if (it.second.compare(key) == 0)
                    return it.first;
            }
            return 0;
        }
    }; //# struct Tag<void>
    //!-----------------------------------------------------------------------------------

    template <typename UserClass>
    struct Tag : TagBase
    {
        using user_type = std::remove_pointer_t<UserClass>;
        using self_type = Tag<UserClass>;

    private:
        inline static bool s_idset = false;
        inline static uint8_t s_lid = 0;

    public:
        static uint8_t id()
        {
            if (!s_idset)
                s_lid = (s_idset ? s_gid : ++s_gid);
            s_idset = true;
            return s_lid;
        }

        static const char *name(std::string_view setName = "")
        {
            static std::string_view s_setName = "";
            if (s_setName.empty() && !setName.empty())
            {
                s_setName = setName;
                Tag<void>::set(self_type::id(), s_setName);
                return s_setName.data();
            }
            else if (!s_setName.empty())
            {
                return s_setName.data();
            }
            if (Tag<void>::has(self_type::id()))
                return Tag<void>::name(self_type::id()).data();
            auto name = std::string_view(typeid(UserClass).name());
            std::string transformed;
            collapseTypeString(transformed, name);
            if (!Tag<void>::has(self_type::id()))
                Tag<void>::set(self_type::id(), transformed);
            return Tag<void>::name(self_type::id()).data();
        }
        Tag() = delete;
        Tag(const self_type &in) = delete;
        self_type &operator=(const self_type &in) = delete;
    }; //# struct Tag<UserClass>
} //> namespace util

#endif //> FG_INC_UTIL_TAG