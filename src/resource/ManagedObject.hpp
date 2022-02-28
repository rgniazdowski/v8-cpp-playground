#pragma once
#ifndef FG_INC_MANAGED_OBJECT
#define FG_INC_MANAGED_OBJECT

#include <util/NamedHandle.hpp>
#include <functional>
#include <string>
#include <vector>

namespace resource
{
    class ManagedObjectBase
    {
    public:
        using self_type = ManagedObjectBase;
        using callback_type = std::function<bool(void *, void *)>;

    public:
        ManagedObjectBase() : m_nameTag(), m_isManaged(false), m_onDestructorCallbacks() {}

        ManagedObjectBase(const self_type &orig)
        {
            // m_pManager = orig.m_pManager;
            m_nameTag = orig.m_nameTag;
            m_isManaged = orig.m_isManaged;
        }

        virtual ~ManagedObjectBase()
        {
            for (auto &info : m_onDestructorCallbacks)
            {
                if (info.callback)
                {
                    info.callback((void *)this, (void *)info.userData);
                    info.callback.swap(callback_type());
                    info.userData = NULL;
                }
            }
            m_onDestructorCallbacks.clear();
        }

    public:
        bool registerOnDestruct(callback_type &callback, void *pUserData = NULL)
        {
            if (!callback)
                return false;
            CallbackData callbackInfo(callback, pUserData);
            m_onDestructorCallbacks.push_back(callbackInfo);
            return true;
        }

        inline util::NamedHandle &getName(void) { return m_nameTag; }

        inline util::NamedHandle const &getName(void) const { return m_nameTag; }

        inline void setManaged(bool toggle = true) { m_isManaged = toggle; }

    protected:
        util::NamedHandle m_nameTag;

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
    }; //# class ManagedObject

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

        ManagedObject(const self_type &orig)
        {
            m_handle = orig.m_handle;
        }

        virtual ~ManagedObject() {}

    protected:
        inline void setName(std::string_view name)
        {
            m_nameTag.set<tag_type>(name); // index should stay untouched
            // m_handle = handle_type(m_nameTag.getIndex(), tag_type::id(), m_nameTag.getHash());
        }

        inline util::NamedHandle &getName(void) { return m_nameTag; }

        inline void setHandle(const handle_type &handle)
        {
            m_nameTag.set<tag_type>(m_handle.getIndex());
            m_handle = handle; // overwrite fully
        }

        inline handle_type &getHandle(void) { return m_handle; }

    public:
        inline util::NamedHandle const &getName(void) const { return m_nameTag; }
        inline handle_type const &getHandle(void) const { return m_handle; }

        // inline bool isManaged(void) const { return m_isManaged; }
        // inline fg::base::CManager *getManager(void) const { return m_pManager; }
        // inline void setManager(fg::base::CManager *pManager) { m_pManager = pManager; }

    protected:
        // fg::base::CManager *m_pManager;
        //
        // Handle is partially separate from a named handle meaning that the managed object
        // has both to identify it, both contain 'index' and 'tag' parts (which are equal),
        // but hash/magic part is different - in any case, a handle should be setup once
        // and changing the name should update only the 'hash' part.
        handle_type m_handle;
    }; //# class ManagedObject

} //> namespace resource
#endif //> FG_INC_MANAGED_OBJECT