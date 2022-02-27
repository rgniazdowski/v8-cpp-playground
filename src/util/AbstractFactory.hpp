#ifndef FG_INC_ABSTRACT_FACTORY
#define FG_INC_ABSTRACT_FACTORY

#include <unordered_map>
#include <functional>
#include <string>

namespace util
{
    class FactoryObjectBase
    {
    public:
        using self_type = FactoryObjectBase;

    public:
        std::string fileExtension;

    public:
        FactoryObjectBase() : fileExtension() {}
        FactoryObjectBase(std::string_view fileExt) : fileExtension(fileExt) {}
        virtual ~FactoryObjectBase() {}

        virtual void *create(void) = 0;
        virtual void *recreate(void *pSource) = 0;
    }; //# class FactoryObjectBase
    //#-----------------------------------------------------------------------------------

    template <typename TClassType>
    class FactoryObject : public FactoryObjectBase
    {
    public:
        using return_type = TClassType;
        using base_type = FactoryObjectBase;
        using self_type = FactoryObject<TClassType>;

    public:
        FactoryObject() : base_type() {}
        FactoryObject(std::string_view fileExt) : base_type(fileExt) {}
        FactoryObject(const self_type &orig) { fileExtension = orig.fileExtension; }

        virtual ~FactoryObject() {}

        virtual void *create(void)
        {
            return (void *)new TClassType();
        }

        virtual void *recreate(void *pSource)
        {
            if (!pSource)
                return create();
            else
                return (void *)new TClassType(*(static_cast<TClassType *>(pSource)));
        }
    }; //# class FactoryObject<TClassType>
    //#-----------------------------------------------------------------------------------

    template <typename TKeyType, typename TClassType>
    class AbstractFactory
    {
    public:
        using key_type = TKeyType;
        using return_type = TClassType;
        using object_type = FactoryObjectBase;
        using self_type = AbstractFactory<key_type, return_type>;

        using NameMap = std::unordered_map<std::string, key_type>;
        using FactoryMap = std::unordered_map<key_type, object_type *>;

    public:
        AbstractFactory() : m_factoryMap(), m_nameMap() {}
        AbstractFactory(const self_type &orig) = delete;

        virtual ~AbstractFactory() { destroy(); }

        void destroy(void)
        {
            for (auto &it : m_factoryMap)
            {
                delete it.second;
                it.second = nullptr;
            }
            m_factoryMap.clear();
            m_nameMap.clear();
        }

        template <typename TClassType>
        bool registerObjectType(const key_type &key, std::string_view keyName, std::string_view fileExt = "")
        {
            using UserClass = std::remove_pointer_t<TClassType>;
            using TagType = typename UserClass::tag_type;
            TagType::id();          // ensure to bump up the unique id (automatic)
            TagType::name(keyName); // try to trigger first use of input name
            auto it = m_factoryMap.find(key);
            if (it != m_factoryMap.end())
                return false;
            m_factoryMap[key] = new FactoryObject<UserClass>(fileExt);
            m_nameMap[keyName] = key;
            return true;
        }

        template <typename TClassType>
        bool registerObjectType(std::string_view fileExt = "")
        {
            using UserClass = std::remove_pointer_t<TClassType>;
            using TagType = typename UserClass::tag_type;
            return registerObjectType<UserClass>(TagType::id(), TagType::name(), fileExt);
        }

        bool unregisterObjectType(const key_type &key)
        {
            auto it = m_factoryMap.find(key);
            if (it != m_factoryMap.end())
            {
                delete (it->second);
                it->second = nullptr;
                m_factoryMap.erase(it);
                return true;
            }
            return false;
        }

        bool unregisterObjectType(std::string_view keyName)
        {
            auto it = m_nameMap.find(keyName);
            if (it != m_nameMap.end())
            {
                auto key = it->second;
                m_nameMap.erase(it);
                return unregisterObjectType(key);
            }
            return false;
        }

        bool isRegistered(const key_type &key) const { return m_factoryMap.find(key) != m_factoryMap.end(); }

        bool isRegistered(std::string_view keyName) const
        {
            auto it = m_nameMap.find(keyName);
            if (it != m_nameMap.end())
            {
                return isRegistered(it->second);
            }
            return false;
        }

        bool create(const key_type &key, return_type **pResult)
        {
            if (!pResult)
                return false;
            if (!isRegistered(key))
                return false;
            if (!m_factoryMap[key])
                return false;
            *pResult = static_cast<return_type *>(m_factoryMap[key]->create());
            return true;
        }

        bool create(std::string_view keyName, return_type **pResult)
        {
            if (!pResult)
                return false;
            if (!isRegistered(keyName))
                return false;
            auto it = m_nameMap.find(keyName);
            auto key = it->second;
            if (!m_factoryMap[key])
                return false;
            *pResult = static_cast<return_type *>(m_factoryMap[key]->create());
            return true;
        }

        bool recreate(const key_type &key, return_type **pResult, return_type *pSource)
        {
            if (!pResult)
                return false;
            if (!isRegistered(key))
                return false;
            if (!m_factoryMap[key])
                return false;
            *pResult = static_cast<return_type *>(m_factoryMap[key]->recreate((void *)pSource));
            return true;
        }

        bool recreate(std::string_view keyName, return_type **pResult, return_type *pSource)
        {
            if (!pResult)
                return false;
            if (!isRegistered(keyName))
                return false;
            auto it = m_nameMap.find(keyName);
            auto key = it->second;
            if (!m_factoryMap[key])
                return false;
            *pResult = static_cast<return_type *>(m_factoryMap[key]->recreate((void *)pSource));
            return true;
        }

        return_type *create(const key_type &key)
        {
            return_type *pObject = nullptr;
            create(key, &pObject);
            return pObject;
        }

        return_type *create(std::string_view keyName)
        {
            return_type *pObject = nullptr;
            create(keyName, &pObject);
            return pObject;
        }

        return_type *recreate(const key_type &key, return_type *pSource)
        {
            return_type *pObject = nullptr;
            recreate(key, &pObject, pSource);
            return pObject;
        }

        return_type *recreate(std::string_view keyName, return_type *pSource)
        {
            return_type *pObject = nullptr;
            recreate(keyName, &pObject, pSource);
            return pObject;
        }

    private:
        FactoryMap m_factoryMap;
        NameMap m_nameMap;
    }; //# class AbstractFactory<TKeyType, TClassType>
    //#-----------------------------------------------------------------------------------

} //> namespace util

#endif //> FG_INC_ABSTRACT_FACTORY