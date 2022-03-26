#pragma once
#ifndef FG_INC_MANAGED_OBJECT
#define FG_INC_MANAGED_OBJECT

#include <util/NamedHandle.hpp>
#include <functional>
#include <string>
#include <vector>

namespace resource
{
    class ManagedObjectBase : public util::ObjectWithHandle
    {
    public:
        using self_type = ManagedObjectBase;
        using callback_type = std::function<void(const ManagedObjectBase *, void *)>;
        using function_type = void (*)(const ManagedObjectBase *, void *);

    public:
        ManagedObjectBase() : m_nameTag(), m_isManaged(false), m_onDestructorCallbacks() {}

        ManagedObjectBase(const self_type &orig)
        {
            m_nameTag = orig.m_nameTag;
            m_isManaged = orig.m_isManaged;
        }

        virtual ~ManagedObjectBase()
        {
            for (auto &info : m_onDestructorCallbacks)
            {
                if (!info.callback)
                    continue;
                info.callback(this, info.userData);
                info.callback.swap(callback_type());
                info.userData = nullptr;
            }
            m_onDestructorCallbacks.clear();
        }

    public:
        bool registerOnDestruct(const callback_type &callback, void *pUserData = nullptr)
        {
            if (!callback)
                return false;
            auto *fnPointerA = callback.template target<function_type>();
            size_t cmpaddrA = (size_t)(fnPointerA != nullptr ? *fnPointerA : 0);
            for (auto const &info : m_onDestructorCallbacks)
            {
                if (!info.callback)
                    continue;
                auto *fnPointerB = info.callback.template target<function_type>();
                size_t cmpaddrB = (size_t)(fnPointerB != nullptr ? *fnPointerB : 0);
                if (cmpaddrA == cmpaddrB && (cmpaddrA != 0 || cmpaddrB != 0))
                    return false; // already pushed
            }
            CallbackData callbackInfo(callback, pUserData);
            m_onDestructorCallbacks.push_back(callbackInfo);
            return true;
        }

        inline util::NamedHandle const &getName(void) const { return m_nameTag; }

    protected:
        inline void setManaged(bool toggle = true) { m_isManaged = toggle; }

        util::NamedHandle m_nameTag;
        inline util::NamedHandle &getName(void) { return m_nameTag; }

    private:
        bool m_isManaged;
        struct CallbackData
        {
            callback_type callback;
            void *userData;

            CallbackData() : userData(nullptr), callback() {}
            CallbackData(callback_type _callback, void *_userData) : callback(_callback), userData(_userData) {}
        };

        using CallbacksVec = std::vector<CallbackData>;
        CallbacksVec m_onDestructorCallbacks;
    }; //# class ManagedObjectBase
    //#-----------------------------------------------------------------------------------

    template <typename THandleType>
    class DataManagerBase;

    template <typename THandleType>
    class ManagedObject : public ManagedObjectBase
    {
    public:
        using handle_type = THandleType;
        using tag_type = typename handle_type::tag_type;
        using data_type = typename tag_type::user_type;
        using self_type = ManagedObject<handle_type>;

        friend class DataManagerBase<handle_type>;

    protected:
        ManagedObject() : ManagedObjectBase(), m_handle() {}

        virtual ~ManagedObject() {}

    protected:
        inline void setName(std::string_view name)
        {
            // index should stay untouched
            m_nameTag.set<tag_type>(name);
            // m_handle = handle_type(m_nameTag.getIndex(), m_nameTag.getHash()); // overwrite fully
        }

        inline void setHandle(const handle_type &handle)
        {
            // just get index, everything else is untouched
            m_nameTag.set<tag_type>(handle.getIndex());
            m_handle = handle; // overwrite fully
            // m_handle = handle_type(handle.getIndex(), m_nameTag.getHash()); // overwrite fully
        }

        inline void resetHandle(void)
        {
            m_handle = 0;
            m_nameTag.set("", 0);
        }

        inline handle_type &getHandle(void) { return m_handle; }

    public:
        inline handle_type const &getHandle(void) const { return m_handle; }
        inline util::HandleBase const &getHandleBase(void) const override { return m_handle; }
        inline uint64_t getIdentifier(void) const override { return m_handle.getHandle(); }

    protected:
        // Handle is partially separate from a named handle meaning that the managed object
        // has both to identify it, both contain 'index' and 'tag' parts (which are equal),
        // but hash/magic part is different - in any case, a handle should be setup once
        // and changing the name should update only the 'hash' part.
        handle_type m_handle;
    }; //# class ManagedObject<THandleType>
} //> namespace resource

#endif //> FG_INC_MANAGED_OBJECT