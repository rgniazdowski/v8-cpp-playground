#pragma once
#ifndef FG_INC_HANDLE
#define FG_INC_HANDLE

#include <util/Tag.hpp>

namespace util
{
    struct HandleHelper;
    class HandleBase
    {
        friend struct HandleHelper;

    protected:
        enum
        {
            MAX_BITS_INDEX = 26, //* 2^26  = 67 108 864
            MAX_BITS_TAG = 6,    //* 2^6      = 64 - support for 64 separate tags / handle managers
            MAX_BITS_HASH = 32,  //* 2^32  = 4 294 967 296
            MAX_BITS_MAGIC = 32, //# alias

            MAX_INDEX = (1ULL << MAX_BITS_INDEX) - 1ULL,
            MAX_TAG = (1ULL << MAX_BITS_TAG) - 1ULL,
            MAX_HASH = (1ULL << MAX_BITS_HASH) - 1ULL,
            MAX_MAGIC = (1ULL << MAX_BITS_MAGIC) - 1ULL,
        };

        union
        {
            struct
            {
                uint32_t m_index : MAX_BITS_INDEX;
                uint8_t m_tag : MAX_BITS_TAG;
                union
                {
                    uint32_t m_hash : MAX_BITS_HASH;
                    uint32_t m_magic : MAX_BITS_MAGIC; /* read only */
                };
            };
            uint32_t m_indexTag;
            uint64_t m_handle; /* read only */
        };
        HandleBase() : m_handle(0) {}
        HandleBase(uint64_t handle) : m_handle(handle) {}
        HandleBase(uint32_t index, uint8_t tag, uint32_t hash) : m_index(index), m_tag(tag), m_hash(hash) {}

    public:
        void reset(void) { m_handle = 0; }

        constexpr uint32_t getIndex(void) const noexcept { return m_index; }
        constexpr uint8_t getTag(void) const noexcept { return m_tag; }
        constexpr uint32_t getMagic(void) const noexcept { return m_magic; }
        constexpr uint32_t getHash(void) const noexcept { return m_hash; }
        constexpr uint64_t getHandle(void) const noexcept { return m_handle; }

        constexpr bool isNull(void) const { return (m_index == 0 && m_magic == 0); }

        operator uint64_t(void) const { return m_handle; }
    };

    struct HandleHelper
    {
        struct Unpacked
        {
            uint32_t index;
            uint8_t tag;
            union
            {
                uint32_t hash;
                uint32_t magic;
            };
            Unpacked(uint64_t handle)
            {
                auto _handle = HandleBase(handle);
                index = _handle.getIndex();
                tag = _handle.getTag();
                hash = _handle.getHash();
            }
            Unpacked(const HandleBase &handle)
            {
                index = handle.getIndex();
                tag = handle.getTag();
                hash = handle.getHash();
            }
        };
        static Unpacked unpack(uint64_t handle) { return Unpacked(handle); }
        static Unpacked unpack(const HandleBase &handle) { return Unpacked(handle); }
    };

    template <typename TagType>
    class Handle : public HandleBase
    {
        static_assert(std::is_base_of<TagBase, TagType>::value, "TagType template parameter type needs to be derived from TagBase");

    public:
        using self_type = Handle<TagType>;
        using type = Handle<TagType>;
        using tag_type = TagType;

    public:
        Handle() : HandleBase(0, tag_type::id(), 0) {}
        Handle(uint64_t handle) : HandleBase(handle) { m_tag = tag_type::id(); }
        Handle(uint32_t index, uint32_t hash) : HandleBase(index, tag_type::id(), hash) {}
        Handle(const self_type &other)
        {
            m_handle = other.m_handle;
            m_tag = tag_type::id(); // setup tag (back to proper if overwritten)
        }
        virtual ~Handle() { m_handle = 0; }

        self_type &operator=(const self_type &other)
        {
            m_handle = other.m_handle;
            m_tag = tag_type::id(); // setup tag (back to proper if overwritten)
            return *this;
        }

        self_type &operator=(uint64_t handle)
        {
            this->m_handle = handle;
            m_tag = tag_type::id();
            return *this;
        }

        bool init(uint32_t index, uint32_t hash = 0)
        {
            if (!isNull() || index > MAX_INDEX)
                return false;
            m_hash = hash;
            m_tag = tag_type::id(); // set tag, forced
            if (!m_hash)
            {
                static uint32_t s_autoMagic = 0;
                if (++s_autoMagic >= MAX_MAGIC)
                    s_autoMagic = 1;
                m_magic = s_autoMagic;
            }
            m_index = index;
            return true;
        }

        static const char *getTagName(void) { return tag_type::name(); }
    }; //# class Handle<TagType>

    class ObjectWithIdentifier {
        public:
        virtual uint64_t getIdentifier(void) const = 0;
    }; //> class ObjectWithIdentifier
    class ObjectWithHandle : public ObjectWithIdentifier
    {
    public:
        virtual util::HandleBase const &getHandleBase(void) const = 0;
    }; //> class ObjectWithHandle
} //> namespace util

template <typename TagType>
inline bool operator!=(const util::Handle<TagType> &l, const util::Handle<TagType> &r)
{
    return (l.getHash() != r.getHash() && l.getTag() != r.getTag());
}

template <typename TagType>
inline bool operator==(const util::Handle<TagType> &l, const util::Handle<TagType> &r)
{
    return (l.getHash() == r.getHash() && l.getTag() == r.getTag());
}

template <typename TagType>
inline bool operator>(const util::Handle<TagType> &l, const util::Handle<TagType> &r)
{
    return (l.getHandle() > r.getHandle());
}

template <typename TagType>
inline bool operator<(const util::Handle<TagType> &l, const util::Handle<TagType> &r)
{
    return (l.getHandle() < r.getHandle());
}

template <typename TagType>
inline bool operator>=(const util::Handle<TagType> &l, const util::Handle<TagType> &r)
{
    return (l.getHandle() >= r.getHandle());
}

template <typename TagType>
inline bool operator<=(const util::Handle<TagType> &l, const util::Handle<TagType> &r)
{
    return (l.getHandle() <= r.getHandle());
}

#endif //> FG_INC_HANDLE