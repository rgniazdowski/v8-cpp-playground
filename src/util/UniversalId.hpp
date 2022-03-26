#pragma once
#ifndef FG_INC_UTIL_UNIVERSAL_ID
#define FG_INC_UTIL_UNIVERSAL_ID

#include <typeinfo>
#include <string>
#include <unordered_map>

#include <util/Util.hpp>

namespace util
{
    inline void collapseTypeString(std::string &output, std::string_view input)
    {
        const std::initializer_list<std::string_view> all_prefixes{"util::UniversalId<", "UniversalId<"};
        const std::initializer_list<std::string_view> any_suffixes{">::name"};
        for (auto &&prefix : all_prefixes)
        {
            const auto p = input.find(prefix);
            if (p != input.npos)
                input.remove_prefix(p + prefix.size());
        }
        for (auto &&suffix : any_suffixes)
        {
            const auto p = input.rfind(suffix);
            if (p != input.npos)
            {
                input.remove_suffix(input.size() - p);
                break;
            }
        }
        output.clear();
        output.reserve(input.length() + 1);
        output.append(input);
        strings::replaceAll(output, {"class ", "", "struct ", "", " >", ">"});
    }

    struct UniversalIdBase
    {
    protected:
        inline static uint32_t s_gid = 0;
    };

    template <typename UserClass>
    struct UniversalId;

    template <>
    struct UniversalId<void>
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
    }; //# struct UniversalId<void>
    //!-----------------------------------------------------------------------------------

    template <typename UserClass>
    struct UniversalId : UniversalIdBase
    {
        using user_type = std::remove_pointer_t<UserClass>;
        using self_type = UniversalId<UserClass>;

    private:
        inline static bool s_idset = false;
        inline static uint32_t s_lid = 0;

    public:
        static uint32_t id()
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
                UniversalId<void>::set(self_type::id(), s_setName);
                return s_setName.data();
            }
            else if (!s_setName.empty())
            {
                return s_setName.data();
            }
            if (UniversalId<void>::has(self_type::id()))
                return UniversalId<void>::name(self_type::id()).data();
            auto name = std::string_view(typeid(UserClass).name());
            /*
            #if defined(_MSC_VER) && !defined(__clang__)
            std::string_view name = __FUNCSIG__;
            #elif defined(__clang__) || defined(__GNUC__)
            std::string_view name = __PRETTY_FUNCTION__;
            #else
            #error "Unknown compiler"
            #endif
            */
            std::string transformed;
            collapseTypeString(transformed, name);
            // Register string name for a given id - id is static and incremented automatically
            if (!UniversalId<void>::has(self_type::id()))
                UniversalId<void>::set(self_type::id(), transformed);
            return UniversalId<void>::name(self_type::id()).data();
        }
        UniversalId() = delete;
        UniversalId(const self_type &in) = delete;
        self_type &operator=(const self_type &in) = delete;
    }; //# struct UniversalId<UserClass>
} //> namespace util

#endif //> FG_INC_UTIL_UNIVERSAL_ID