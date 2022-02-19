#pragma once
#ifndef FG_INC_UTIL_ENUM_NAME
#define FG_INC_UTIL_ENUM_NAME

#include <string>
#include <algorithm>

namespace util
{
    class default_t
    {
    public:
        template <typename T>
        operator T() const { return T(); }
    };

    default_t const default = default_t();

    template <typename T>
    struct IdAndName
    {
        T id;
        std::string name;
        bool operator<(const IdAndName &rhs) const { return id < rhs.id; }
        bool operator>(const IdAndName &rhs) const { return id > rhs.id; }
    };
#define ID_NAME(x) \
    {              \
        x, #x      \
    }

    namespace detail
    {
        template <typename T>
        const char *id_to_name(T id, IdAndName<T> *table_begin, IdAndName<T> *table_end)
        {
            if ((table_end - table_begin) > 1 && table_begin[0].id > table_begin[1].id)
                std::stable_sort(table_begin, table_end);

            IdAndName<T> searchee = {id, default};
            IdAndName<T> *p = std::lower_bound(table_begin, table_end, searchee);
            return (p == table_end || p->id != id) ? nullptr : p->name.c_str();
        }
        template <typename T>
        T name_to_id(const std::string &name, IdAndName<T> *table_begin, IdAndName<T> *table_end)
        {
            IdAndName<T> *p = std::find_if(
                table_begin, table_end, [&name](const IdAndName<T> &arg)
                { return name.compare(arg.name) == 0; });
            return (p == table_end) ? default : p->id;
        }
    };

    template <typename T, int N>
    const char *id_to_name(T id, IdAndName<T> (&table)[N])
    {
        return detail::id_to_name<T>(id, &table[0], &table[N]);
    }

    template <typename T, int N>
    T name_to_id(const std::string &name, IdAndName<T> (&table)[N])
    {
        return detail::name_to_id<T>(name, &table[0], &table[N]);
    }

    template <typename T, typename E>
    const char *enum_name(E id)
    {
        return T::enum_name(id);
    }

    template <typename T, typename E>
    E enum_value(const std::string &name)
    {
        return T::enum_value(name);
    }
} //> namespace util

#endif //> FG_INC_UTIL_ENUM_NAME