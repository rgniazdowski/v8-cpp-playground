#pragma once
#ifndef FG_INC_HANDLE_MANAGER
#define FG_INC_HANDLE_MANAGER

#include <util/Vector.hpp>
#include <util/Handle.hpp>
#include <util/NamedHandle.hpp>
#include <unordered_map>

namespace util
{
    template <typename THandleType>
    class HandleManager
    {
        static_assert(std::is_base_of<HandleBase, THandleType>::value, "THandleType template parameter type needs to be derived from HandleBase");

    public:
        using handle_type = THandleType;
        using tag_type = typename handle_type::tag_type;
        using data_type = typename tag_type::user_type;
        using self_type = HandleManager<handle_type>;
        using HashKeyString = std::string;

        /// Special Name map - maps std::string to the index
        using NameMap = std::unordered_map<HashKeyString, uint32_t>;
        /// Hash map - maps the name tags hash sum to the data vector index
        using HashMap = std::unordered_map<uint32_t, uint32_t>;

    protected:
        /**
         * Special data holder - DataVec[index] -> data / magic / nameTag
         */
        struct DataHolder
        {
            data_type *data;
            // this is just the magic part of a number-only handle, which should be immutable
            // meaning that once assigned to a given object - it shouldn't update
            uint32_t magic;
            // named handle contains hash, tag and index along with string representation
            NamedHandle nameTag;

            DataHolder() : data(nullptr), magic(0), nameTag() {}
            ~DataHolder() { clear(); }
            /**
             *
             */
            void clear(void)
            {
                data = nullptr;
                magic = 0;
                nameTag.reset();
                nameTag.clear();
            }
        }; // struct DataHolder

        // Type for vector storing Data pointers
        using DataVec = Vector<DataHolder>;
        using DataVecItor = typename DataVec::iterator;

    private:
        /// Free slots vector
        using FreeSlotsVec = Vector<uint32_t>;

        /// Free slots in the database
        FreeSlotsVec m_freeSlots;
        /// Special data storage
        DataVec m_managedData;
        /// Map for name (string) IDs - bind str name to index
        NameMap m_nameMap;
        /// Map for binding hash sum to index
        HashMap m_hashMap;

    public:
        HandleManager() : m_freeSlots(), m_managedData(), m_nameMap(), m_hashMap() {}

        virtual ~HandleManager() { releaseAllHandles(); }

        bool acquireHandle(handle_type &rHandle, data_type *pResource);

        bool setupName(const std::string &name, const handle_type &rHandle);

        bool releaseHandle(const handle_type &handle);
        void releaseAllHandles(void);

        data_type *dereference(const handle_type &handle);
        data_type *dereference(NamedHandle &name);
        data_type *dereference(const std::string &name);

        uint32_t getUsedHandleCount(void) const
        {
            return (uint32_t)(m_managedData.size() - m_freeSlots.size());
        }
        bool hasUsedHandles(void) const { return (bool)(!!getUsedHandleCount()); }
        DataVec &getDataVector(void) { return m_managedData; }
        const DataVec &getDataVector(void) const { return m_managedData; }

        bool isDataManaged(data_type *pData);
        bool isHandleValid(const handle_type &handle);
    }; //# class HandleManager

} //> namespace util
//#---------------------------------------------------------------------------------------

template <typename THandleType>
bool util::HandleManager<THandleType>::acquireHandle(handle_type &rHandle, data_type *pResource)
{
    // If free list is empty, add a new one otherwise use first one found
    uint32_t index;
    if (m_freeSlots.empty())
    {
        index = (uint32_t)m_managedData.size();
        if (!rHandle.init(index))
            return false;
        DataHolder holder;
        holder.data = pResource;
        holder.magic = rHandle.getMagic();
        holder.nameTag.set<tag_type>(index);
        m_managedData.push_back(holder);
    }
    else
    {
        index = m_freeSlots.back();
        if (!rHandle.init(index))
            return false;
        m_freeSlots.pop_back();
        auto &holder = m_managedData[index];
        holder.data = pResource;
        holder.magic = rHandle.getMagic();
        holder.nameTag.set<tag_type>(index);
    }
    return true;
}
//>---------------------------------------------------------------------------------------

template <typename THandleType>
bool util::HandleManager<THandleType>::setupName(const std::string &name, const handle_type &rHandle)
{
    if (!isHandleValid(rHandle))
    {
        return false;
    }
    auto index = rHandle.getIndex();
    // auto tag = rHandle.getTag();
    if (m_nameMap.find(name) != m_nameMap.end())
    {
        // FG_LOG_ERROR("HandleManager[%s] Such key already exists in name map - index[%u], name_tag[%s]",
        //              THandleType::getTagName(),
        //              index,
        //              name.c_str());
        return false; // Such key already exists
    }
    auto &holder = m_managedData[index];
    if (holder.magic != rHandle.getMagic())
    {
        return false; // invalid?
    }
    if (!holder.nameTag.empty())
    {
        // FG_LOG_ERROR("HandleManager[%s]: There is name tag already in the vector - index[%u], name_tag[%s]",
        //              THandleType::getTagName(),
        //              index,
        //              name.c_str());
        //  There is already some set on the current index
        return false;
    }
    holder.nameTag.reset();
    // this is template set<>, will update tag, hash and index fields (and string of course)
    holder.nameTag.set<tag_type>(name, index);
    uint32_t hash = holder.nameTag.getHash();
    // need to find any existing entries that point to the same index and remove them
    // before setting up new ones - this will work while renaming to new name the same obj
    for (auto &it : m_nameMap)
    {
        if (it.second == index)
        {
            m_nameMap.erase(it.first);
            break;
        }
    } //# for each name mapped
    for (auto &it : m_hashMap)
    {
        if (it.second == index)
        {
            m_hashMap.erase(it.first);
            break;
        }
    } //# for each hash mapped
    // assign new name/hash to index
    m_nameMap[name] = index;
    m_hashMap[hash] = index;
    // FG_LOG_DEBUG("HandleManager[%s]: Setup name[%s], hash[%10u], index[%u]",
    //              THandleType::getTagName(),
    //              name.c_str(),
    //              hash,
    //              index);
    return true;
}
//>---------------------------------------------------------------------------------------

template <typename THandleType>
bool util::HandleManager<THandleType>::releaseHandle(const handle_type &rHandle)
{
    if (!isHandleValid(rHandle))
    {
        // FG_LOG_DEBUG("HandleManager[%s]: can't release handle - handle is invalid, tag_name[%s]", THandleType::getTagName(), THandleType::getTagName());
        return false;
    }
    // which one?
    auto index = rHandle.getIndex();
    // auto tag = rHandle.getTag();
    //  ok remove it - tag as unused and add to free list
    auto &holder = m_managedData[index];
    // FG_LOG_DEBUG("HandleManager[%s]: Releasing handle: index[%d], magic[%d], handle[%d], name[%s]",
    //              THandleType::getTagName(),
    //              index,
    //              handle.getMagic(),
    //              handle.getHandle(),
    //              m_managedData[index].nameTag.c_str());
    if (!holder.nameTag.empty())
        m_nameMap.erase(holder.nameTag);
    auto hash = holder.nameTag.getHash();
    if (hash)
        m_hashMap.erase(hash);
    holder.clear();
    m_freeSlots.push_back(index);
    if (!this->getUsedHandleCount())
        this->releaseAllHandles(); // no handle used - reset all to start from scratch
    return true;
}
//>---------------------------------------------------------------------------------------

template <typename THandleType>
void util::HandleManager<THandleType>::releaseAllHandles(void)
{
    m_managedData.clear();
    m_freeSlots.clear();
    m_nameMap.clear();
    m_hashMap.clear();
}
//>---------------------------------------------------------------------------------------

template <typename THandleType>
typename util::HandleManager<THandleType>::data_type *util::HandleManager<THandleType>::dereference(const handle_type &handle)
{
    if (!this->isHandleValid(handle))
        return nullptr;
    if (handle.getTag() != tag_type::id())
        return nullptr;
    return (*(m_managedData.begin() + handle.getIndex())).data;
}
//>---------------------------------------------------------------------------------------

template <typename THandleType>
typename util::HandleManager<THandleType>::data_type *util::HandleManager<THandleType>::dereference(const std::string &name)
{
    if (name.empty())
        return nullptr;
    NamedHandle nameTag(name);
    uint32_t index = 0;
    auto hash = nameTag.getHash();
    auto hashIt = m_hashMap.find(hash);
    if (hashIt == m_hashMap.end())
    {
        auto nameIt = m_nameMap.find(name);
        if (nameIt == m_nameMap.end())
            return nullptr;
        index = nameIt->second;
    }
    (hashIt != m_hashMap.end()) && (index = hashIt->second);
    if (index >= m_managedData.size())
        return nullptr;
    DataHolder &holder = m_managedData[index];
    if (holder.nameTag.getHash() == nameTag.getHash())
        return holder.data;
    return nullptr;
}
//>---------------------------------------------------------------------------------------

template <typename THandleType>
typename util::HandleManager<THandleType>::data_type *util::HandleManager<THandleType>::dereference(NamedHandle &name)
{
    if (name.empty())
        return nullptr;
    if (name.isIndexSet())
    {
        auto index = name.getIndex();
        if (index >= m_managedData.size())
            return nullptr;
        DataHolder &holder = m_managedData[index];
        if (holder.nameTag.getHash() == name.getHash())
            return holder.data;
    }
    else
    {
        uint32_t index = 0;
        auto hash = name.getHash();
        auto hashIt = m_hashMap.find(hash);
        if (hashIt == m_hashMap.end())
        {
            auto nameIt = m_nameMap.find(name);
            if (nameIt == m_nameMap.end())
                return nullptr;
            index = nameIt->second;
        }
        (hashIt != m_hashMap.end()) && (index = hashIt->second);
        if (index >= m_managedData.size())
            return nullptr;
        name.set<tag_type>(index);
        DataHolder &holder = m_managedData[index];
        return holder.data;
    }
    return nullptr;
}
//>---------------------------------------------------------------------------------------

template <typename THandleType>
bool util::HandleManager<THandleType>::isDataManaged(data_type *pData)
{
    for (auto &item : m_managedData)
    {
        if (item.data == pData)
            return true;
    }
    return false;
}
//>---------------------------------------------------------------------------------------

template <typename THandleType>
bool util::HandleManager<THandleType>::isHandleValid(const handle_type &handle)
{
    if (handle.isNull())
        return false;
    // check handle validity - $ this check can be removed for speed
    // if you can assume all handle references are always valid.
    auto index = handle.getIndex();
    if (index >= m_managedData.size())
        return false;
    DataHolder &holder = m_managedData[index];
    if (holder.nameTag.getHash() != handle.getHash())
    {
        // no good! invalid handle == client programming error
        // FG_LOG_DEBUG("HandleManager[%s]: invalid handle, magic numbers don't match: index[%d], magic[%d], handle[%d], true_magic[%d]",
        //             THandleType::getTagName(),
        //             index,
        //             handle.getMagic(),
        //             handle.getHandle(),
        //             m_managedData[index].magic);
        return false;
    }
    return true;
}
//>---------------------------------------------------------------------------------------

#if 0
template <typename DataType, typename HandleType>
const DataType* HandleManager <DataType, HandleType>
::Dereference(HandleType handle) const {
    // this lazy cast is ok - non-const version does not modify anything
    typedef HandleManager <DataType, HandleType> ThisType;
    return ( const_cast<ThisType*>(this)->Dereference(handle));
}
#endif

#endif //> FG_INC_HANDLE_MANAGER