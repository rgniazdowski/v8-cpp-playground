#pragma once
#ifndef FG_UTIL_NAMED_HANDLE
#define FG_UTIL_NAMED_HANDLE

#include <string>
#include <util/Handle.hpp>

namespace util
{
    class NamedHandle : public std::string, public HandleBase
    {
    public:
        using base_type = std::string;
        using self_type = NamedHandle;
        static const size_type npos = base_type::npos;

    public:
        NamedHandle() : base_type(), HandleBase(), m_isIdxSet(false) {}

        NamedHandle(uint32_t index) : base_type(), HandleBase(index, 0, 0), m_isIdxSet(true) {}

        NamedHandle(std::string_view nameTag) : base_type(nameTag), HandleBase(0, 0, calculateHash()), m_isIdxSet(false) {}

        NamedHandle(std::string_view nameTag, uint32_t index) : base_type(nameTag), HandleBase(index, 0, calculateHash()), m_isIdxSet(true) {}

        NamedHandle(const NamedHandle &nameTag) : base_type(), HandleBase() { set(nameTag); }

        virtual ~NamedHandle()
        {
            HandleBase::reset();
            base_type::resize(0);
            base_type::reserve(0);
            std::string str;
            base_type::swap(str);
        }

    public:
        self_type &operator=(const self_type &str)
        {
            base_type::assign(str);
            calculateHash();
            m_index = str.m_index;
            m_tag = str.m_tag;
            m_isIdxSet = str.m_isIdxSet;
            return (*this);
        }
        self_type &operator=(const base_type &str)
        {
            base_type::assign(str);
            calculateHash();
            return (*this);
        }
        self_type &operator=(const char *str)
        {
            base_type::assign(str);
            calculateHash();
            return (*this);
        }
        self_type &operator=(char c)
        {
            base_type::assign(1, c);
            calculateHash();
            return (*this);
        }
        self_type &operator=(uint64_t other_handle)
        {
            this->m_handle = other_handle;
            calculateHash();
            return *this;
        }
        using base_type::begin;
        using base_type::c_str;
        using base_type::capacity;
        using base_type::empty;
        using base_type::end;
        using base_type::length;
        using base_type::max_size;
        using base_type::rbegin;
        using base_type::rend;
        using base_type::size;

    public:
        void resize(size_type n, char c)
        {
            base_type::resize(n, c);
            calculateHash();
        }
        void resize(size_type n)
        {
            base_type::resize(n);
            calculateHash();
        }
        void reserve(size_type res_size)
        {
            base_type::reserve(res_size);
            calculateHash();
        }
        void clear(void)
        {
            base_type::clear();
            calculateHash();
        }

    public:
        using base_type::operator[];
        using base_type::at;
        using base_type::back;
        using base_type::front;

        self_type &operator+=(const self_type &str)
        {
            base_type::append(str);
            calculateHash();
            return (*this);
        }
        self_type &operator+=(const std::string &str)
        {
            base_type::append(str);
            calculateHash();
            return (*this);
        }
        self_type &operator+=(const char *str)
        {
            base_type::append(str);
            calculateHash();
            return (*this);
        }
        self_type &operator+=(char c)
        {
            base_type::push_back(c);
            calculateHash();
            return (*this);
        }

    public:
        inline self_type &append(const self_type &str)
        {
            base_type::append(str);
            calculateHash();
            return (*this);
        }
        inline self_type &append(const self_type &str, size_type pos, size_type n)
        {
            base_type::append(str, pos, n);
            calculateHash();
            return (*this);
        }
        inline self_type &append(const std::string &str)
        {
            base_type::append(str);
            calculateHash();
            return (*this);
        }
        inline self_type &append(const std::string &str, size_type pos, size_type n)
        {
            base_type::append(str, pos, n);
            calculateHash();
            return (*this);
        }
        inline self_type &append(const char *str, size_type n)
        {
            base_type::append(str, n);
            calculateHash();
            return (*this);
        }
        inline self_type &append(const char *str)
        {
            base_type::append(str);
            calculateHash();
            return (*this);
        }
        inline self_type &append(size_type n, char c)
        {
            base_type::append(n, c);
            calculateHash();
            return (*this);
        }

    public:
        inline void push_back(char c)
        {
            base_type::push_back(c);
            calculateHash();
        }

        inline self_type &assign(const self_type &str)
        {
            base_type::assign(str);
            calculateHash();
            return (*this);
        }

        inline self_type &assign(const std::string &str)
        {
            base_type::assign(str);
            calculateHash();
            return (*this);
        }

    public:
        using base_type::compare;
        inline int compare(const self_type &str) const
        {
            // return base_type::compare(str);
            return (this->m_hash == str.m_hash);
        }

    public:
        void set(std::string_view nameTag)
        {
            base_type::assign(nameTag);
            calculateHash();
        }

        template <typename TagType>
        void set(std::string_view nameTag)
        {
            static_assert(std::is_base_of<TagBase, TagType>::value, "TagType template parameter type needs to be derived from TagBase");
            set<TagType>(nameTag, m_index);
        }

        template <typename TagType>
        void set(std::string_view nameTag, uint32_t index)
        {
            static_assert(std::is_base_of<TagBase, TagType>::value, "TagType template parameter type needs to be derived from TagBase");
            set(nameTag);
            set(index, TagType::id());
        }
        //#-------------------------------------------------------------------------------

        void set(uint32_t index, uint8_t tag)
        {
            m_index = index;
            m_tag = tag;
            m_isIdxSet = true;
        }

        template <typename TagType>
        void set(uint32_t index)
        {
            static_assert(std::is_base_of<TagBase, TagType>::value, "TagType template parameter type needs to be derived from TagBase");
            set(index, TagType::id());
        }

        void set(const NamedHandle &nameTag)
        {
            base_type::assign(nameTag);
            m_hash = nameTag.m_hash;
            m_tag = nameTag.m_tag;
            m_index = nameTag.m_index;
            m_isIdxSet = nameTag.m_isIdxSet;
        }
        //#-------------------------------------------------------------------------------

        constexpr bool isIndexSet(void) const noexcept { return m_isIdxSet; }

        std::string &getString(void) { return (*this); }
        std::string const &getString(void) const { return (*this); }

    protected:
        uint32_t calculateHash(void)
        {
            static std::hash<base_type> hasher;
            m_hash = (uint32_t)hasher(*this);
            return m_hash;
        }

    private:
        bool m_isIdxSet;
    }; //# class NamedHandle

} //> namespace util

#endif //> FG_UTIL_NAMED_HANDLE