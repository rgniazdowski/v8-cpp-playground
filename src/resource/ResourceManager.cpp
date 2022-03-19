#include <resource/ResourceManager.hpp>
#include <resource/ResourceConfigJson.hpp>
#include <util/Util.hpp>
#include <util/File.hpp>
#include <util/JsonFile.hpp>
#include <Queue.hpp>

resource::ResourceManager::ResourceManager(base::ManagerBase *pEventMgr) : base_type(),
                                                                           m_currentResource(),
                                                                           m_resourceGroupHandles(),
                                                                           m_resourceFactory(std::make_unique<ResourceFactory>()),
                                                                           m_pEventMgr(pEventMgr),
                                                                           m_nCurrentUsedMemory(0),
                                                                           m_nMaximumMemory(0),
                                                                           m_bResourceReserved(false)
{
    m_thread.setThreadName("ResourceManager");
}
//>---------------------------------------------------------------------------------------

resource::ResourceManager::~ResourceManager()
{
    self_type::destroy();
}
//>---------------------------------------------------------------------------------------

bool resource::ResourceManager::destroy(void)
{
    if (!isInit())
        return false;
    return true;
}
//>---------------------------------------------------------------------------------------

bool resource::ResourceManager::initialize(void)
{
    if (isInit())
        return true;
    m_dataDir.read(".", true, true);
    m_dataDir.rewind();
    return true;
}
//>---------------------------------------------------------------------------------------

bool resource::ResourceManager::setMaximumMemory(size_t nMaxSize)
{
    m_nMaximumMemory = nMaxSize;
    return checkForOverallocation();
}
//>---------------------------------------------------------------------------------------

bool resource::ResourceManager::reserveMemory(size_t nMem)
{
    addMemory(nMem);
    if (!checkForOverallocation())
        return false;
    m_bResourceReserved = true;
    return true;
}
//>---------------------------------------------------------------------------------------

bool resource::ResourceManager::goToNext(ResourceType resType)
{
    if (resType == resource::INVALID)
        return goToNext();
    while (true)
    {
        m_currentResource++;
        if (!isValid())
            break;
        if (!(*m_currentResource).data)
            continue;
        if ((*m_currentResource).data->getResourceType() == resType)
            break;
    }
    return isValid();
}
//>---------------------------------------------------------------------------------------

bool resource::ResourceManager::goToNext(const ResourceType *resTypes, int n)
{
    if (!resTypes)
        return false;
    while (true)
    {
        goToNext();
        if (!isValid())
            break;
        if (!(*m_currentResource).data)
            continue;
        bool status = false;
        int i = 0;
        while (!status)
        {
            if (i == n || resTypes[i] == resource::INVALID)
                break;
            if ((*m_currentResource).data->getResourceType() == resTypes[i])
                status = true;
            i++;
        }
        if (status == true)
            break;
    }
    return isValid();
}
//>---------------------------------------------------------------------------------------

bool resource::ResourceManager::goToNext(ResourceType resType, Quality quality)
{
    while (true)
    {
        goToNext();
        if (!isValid())
            break;
        if ((*m_currentResource).data->getResourceType() == resType)
        {
            if ((*m_currentResource).data->getQuality() == quality)
                break;
        }
    }
    return isValid();
}
//>---------------------------------------------------------------------------------------

bool resource::ResourceManager::getResourceNames(util::StringVector &strVec, ResourceType resType)
{
    strVec.clear();
    unsigned int nFound = 0;
    goToBegin();
    do
    {
        Resource *pResource = getCurrentResource();
        if (!pResource)
            break;
        if (pResource->getResourceType() == resType || resType == resource::INVALID)
        {
            strVec.push_back(pResource->getName());
            nFound++;
        }
    } while (goToNext(resType));

    return (nFound > 0);
}
//>---------------------------------------------------------------------------------------

bool resource::ResourceManager::getResourceNames(util::StringVector &strVec,
                                                 const ResourceType *resTypes,
                                                 unsigned int n)
{
    if (!resTypes || !n)
        return false;
    strVec.clear();
    unsigned int nFound = 0;
    goToBegin();
    if (isValid())
    {
        Resource *pResource = getCurrentResource();
        for (unsigned int i = 0; i < n; i++)
        {
            if (pResource->getResourceType() == resTypes[i])
            {
                strVec.push_back(pResource->getName());
                nFound++;
                break;
            }
        }
    }
    while (goToNext(resTypes, n))
    {
        Resource *pResource = getCurrentResource();
        if (!pResource)
            break;
        strVec.push_back(pResource->getName());
        nFound++;
    }
    return (nFound > 0);
}
//>---------------------------------------------------------------------------------------

bool resource::ResourceManager::insertResource(Resource *pResource)
{
    if (!pResource)
    {
        return false;
    }
    if (!base_type::insert(pResource, pResource->getName()))
    {
        return false;
    }
    pResource->setManaged(true);
    // pResource->setManager(this);
    //  Get the memory and add it to the catalog total.  Note that we only have
    //  to check for memory overallocation if we haven't preallocated memory
    if (!m_bResourceReserved)
    {
        addMemory(pResource->getSize()); // ? nope
        // check to see if any overallocation has taken place
        if (!checkForOverallocation())
            return false;
    }
    else
        m_bResourceReserved = false;

    return true;
}
//>---------------------------------------------------------------------------------------

resource::Resource *resource::ResourceManager::refreshResource(Resource *pResource)
{
    if (!pResource)
        return nullptr;
    // Set the current time as the last time the object was accessed
    pResource->setLastAccess(time(0));
    // Recreate the object before giving it to the application
    if (pResource->isDisposed())
    {
        // if (m_pQualityMgr)
        //     pResource->setQuality(static_cast<CQualityManager *>(m_pQualityMgr)->getQuality());
        pResource->recreate();
        if (!pResource->isDisposed() && m_pEventMgr)
        {
            ////event::SResource *resEvent = fgMalloc<event::SResource>();
            // event::SResource *resEvent = (event::SResource*)static_cast<event::CEventManager *>(m_pEventMgr)->requestEventStruct();
            // event::CArgumentList *argList = static_cast<event::CEventManager *>(m_pEventMgr)->requestArgumentList();
            // resEvent->eventType = event::RESOURCE_CREATED;
            // resEvent->timeStamp = timesys::ticks();
            // resEvent->status = event::SResource::CREATED;
            // resEvent->resource = pResource;
            //
            ////event::CArgumentList *argList = new event::CArgumentList();
            // argList->push(event::SArgument::Type::ARG_TMP_POINTER, (void *)resEvent);
            // static_cast<event::CEventManager *>(m_pEventMgr)->throwEvent(event::RESOURCE_CREATED, argList);
        }
        addMemory(pResource->getSize());
        // check to see if any overallocation has taken place, but
        // make sure we don't swap out the same resource.
        ////pResource->Lock();
        checkForOverallocation();
        ////pResource->Unlock();
    }
    return pResource;
}
//>---------------------------------------------------------------------------------------

bool resource::ResourceManager::remove(Resource *pResource)
{
    if (!base_type::isManaged(pResource))
        return false;
    // if the resource was found, check to see that it's not locked
    ////if (pResource->isLocked())
    ////{
    ////    // Can't remove a locked resource
    ////    FG_MessageSubsystem->reportError(tag_type::name(), FG_ERRNO_RESOURCE_LOCKED_REMOVAL, FG_MSG_IN_FUNCTION);
    ////    return false;
    ////}
    // Get the memory and subtract it from the manager total
    removeMemory(pResource->getSize());
    releaseHandle(pResource->getHandle());
    pResource->setManaged(false);
    // pResource->setManager(nullptr);
    return true;
}
//>---------------------------------------------------------------------------------------

bool resource::ResourceManager::dispose(Resource *pResource)
{
    if (!base_type::isManaged(pResource))
        return false;
    // if the resource was found, check to see that it's not locked
    ////if (pResource->isLocked())
    ////{
    ////    // Can't remove a locked resource
    ////    FG_MessageSubsystem->reportError(tag_type::name(), FG_ERRNO_RESOURCE_LOCKED_REMOVAL, FG_MSG_IN_FUNCTION);
    ////    return false;
    ////}
    auto nDisposalSize = pResource->getSize();
    pResource->dispose();
    if (pResource->isDisposed())
    {
        // Get the memory and subtract it from the manager total
        removeMemory(nDisposalSize);
    }
    else
    {
        // For some reason the resource is not disposed
        // FG_MessageSubsystem->reportWarning(tag_type::name(), FG_ERRNO_RESOURCE_NOT_DISPOSED, FG_MSG_IN_FUNCTION);
        return false;
    }
    return true;
}
//>---------------------------------------------------------------------------------------

resource::Resource *resource::ResourceManager::request(std::string_view info, const ResourceType forcedType)
{
    if (!m_init || info.empty())
        return nullptr;
    Resource *resourcePtr = nullptr;
    // This is a fallback, if such resource already exists in the resource manager
    // it should not be searched and reloaded - however do not use request() in a main
    // loop as it may be slower
    resourcePtr = ResourceManager::get(std::string(info));
    if (resourcePtr)
    {
        // This print will flood output
        // FG_LOG_DEBUG("Resource: Found requested resource: name[%s], request[%s]", resourcePtr->getNameStr(), info.c_str());
        return resourcePtr;
    }
    // info cannot be a path, it has to be resource name or config name
    // required file will be found
    if (strings::containsChars(info, "/\\"))
    {
        // FG_LOG_ERROR("Resource: Request cannot contain full path: '%s'", info.c_str());
        return nullptr;
    }
    std::string pattern;
    std::string filePath;
    ResourceType resExtType = resource::INVALID;
    bool infoAsName = false;
    bool isFound = false;
    bool isConfig = false;
    const char *iext = path::fileExt(info.data(), true);
    if (!iext)
    { // no extension given so... search all
        infoAsName = true;
        pattern.append(info).append(".*;");
    }
    else
    { // extension is given, search for exact file
        pattern.append(info);
    }
    // Search file names of resources already in cache
    if (!infoAsName && iext)
    {
        // This is special search for file name within already loaded resources
        goToBegin();
        while (isValid())
        {
            Resource *res = getCurrentResource();
            if (!res)
            {
                goToNext();
                continue;
            }
            Resource::FileMapping &files = res->getFileMapping();
            Resource::FileMappingItor fit = files.begin(), fend = files.end();
            for (; fit != fend; fit++)
            {
                // Comparing using endsWith - resource contains relative file paths
                // not just file name - this request function takes in just file names
                // resource names or patterns (wildcards for extensions)
                if (strings::endsWith(fit->second, pattern, true))
                {
                    // if(fit->second.compare(pattern) == 0) {
                    //  Found resource containing specified file
                    ResourceManager::refreshResource(res);
                    return res;
                }
            }
            goToNext();
        }
    }
    m_dataDir.rewind();
    while (m_dataDir.searchForFile(filePath, "./", pattern, true).length())
    {
        const char *fext = nullptr;
        const char *fileName = path::fileName(filePath.c_str());
        if (iext)
            fext = iext;
        else
            fext = path::fileExt(fileName, true);
        if (strings::endsWith(fext, "res.json", true))
            isConfig = true;
        else
            resExtType = m_resourceFactory->getKeyTypeForFileExtension(fext);
        if (isConfig || resExtType != resource::INVALID)
        {
            isFound = true;
            break;
        }
    };
    if (!isFound)
        return nullptr;
    if (isConfig)
    {
        ResourceHeader config = util::JsonFile::loadInPlace(filePath); // just one resource in file (not mapped resource group)
        if (config.name.length())
        {
            if (m_resourceFactory->isRegistered(config.type))
            {
                resourcePtr = m_resourceFactory->create(config.type);
                resourcePtr->setName(config.name);
                resourcePtr->setFlags(config.flags);
                // resourcePtr->setPriority(config.priority);
                resourcePtr->setQuality(config.quality);
                for (auto &it : config.fileMapping)
                    resourcePtr->setFilePath(it.second, it.first);
                resourcePtr->setDefaultID(config.quality);
            }
        }
    }
    else if (resExtType != resource::INVALID)
    {
        if (forcedType != AUTO)
            resExtType = forcedType;
        if (m_resourceFactory->isRegistered(resExtType))
        {
            resourcePtr = m_resourceFactory->create(resExtType);
            resourcePtr->setName(info);
            resourcePtr->setPriority(ResourcePriority::LOW);
            resourcePtr->setQuality(Quality::UNIVERSAL);
            resourcePtr->setDefaultID(Quality::UNIVERSAL);
            resourcePtr->setFilePath(filePath);
        }
    }
    if (resourcePtr)
    {
        if (!insertResource(resourcePtr))
        {
            releaseHandle(resourcePtr->getHandle());
            delete resourcePtr;
            resourcePtr = nullptr;
            return nullptr;
        }
        // This will recreate the resource if necessary and throw proper event
        // if the pointer to the external event manager is set.
        ResourceManager::refreshResource(resourcePtr);
        // if (m_pEventMgr)
        //{
        //     // #FIXME ! ! ! !
        //     // event::SResource *resEvent = fgMalloc<event::SResource>();
        //     event::SResource *resEvent = (event::SResource *)static_cast<event::CEventManager *>(m_pEventMgr)->requestEventStruct();
        //     event::CArgumentList *argList = static_cast<event::CEventManager *>(m_pEventMgr)->requestArgumentList();
        //     resEvent->eventType = event::RESOURCE_REQUESTED;
        //     resEvent->timeStamp = timesys::ticks();
        //     resEvent->status = event::SResource::REQUESTED;
        //     resEvent->resource = resourcePtr;
        //     // event::CArgumentList *argList = new event::CArgumentList();
        //     argList->push(event::SArgument::Type::ARG_TMP_POINTER, (void *)resEvent);
        //     static_cast<event::CEventManager *>(m_pEventMgr)->throwEvent(event::RESOURCE_REQUESTED, argList);
        // }
    }
    return resourcePtr;
}
//>---------------------------------------------------------------------------------------

bool resource::ResourceManager::checkForOverallocation(void)
{
    if (m_nCurrentUsedMemory > m_nMaximumMemory)
    {
        resetMemory();
        // create a temporary priority queue to store the managed items
        PriorityQueue<Resource *, std::vector<Resource *>, util::PtrGreater<Resource *>> priorityResQ;
        DataVecItor begin = getDataVector().begin(), end = getDataVector().end();

        // insert copies of all the resource pointers into the priority queue, but
        // exclude those that are current disposed or are locked
        for (DataVecItor itor = begin; itor != end; ++itor)
        {
            if (!(*itor).data)
                continue;
            addMemory((*itor).data->getSize());
            // if (!(*itor).data->isDisposed() && !(*itor).data->isLocked())
            if (!(*itor).data->isDisposed())
                priorityResQ.push((*itor).data);
        }
        // Attempt to remove iMemToPurge bytes from the managed resource
        const auto iMemToPurge = m_nCurrentUsedMemory - m_nMaximumMemory;
        while ((!priorityResQ.empty()) && (m_nCurrentUsedMemory > m_nMaximumMemory))
        {
            auto nDisposalSize = priorityResQ.top()->getSize();
            // Dispose of the all loaded data, free all memory, but don't destroy the object
            priorityResQ.top()->dispose();
            if (priorityResQ.top()->isDisposed())
                removeMemory(nDisposalSize);
            priorityResQ.pop();
        }

        // If the resource queue is empty and we still have too much memory allocated,
        // then we return failure.  This could happen if too many resources were locked
        // or if a resource larger than the requested maximum memory was inserted.
        if (priorityResQ.empty() && (m_nCurrentUsedMemory > m_nMaximumMemory))
        {
            // FG_MessageSubsystem->reportWarning(tag_type::name(), FG_ERRNO_RESOURCE_OVERALLOCATION);
            return false;
        }
    }
    return true;
}
//>---------------------------------------------------------------------------------------

void resource::ResourceManager::refreshMemory(void)
{
    resetMemory();
    DataVecItor begin = getDataVector().begin(), end = getDataVector().end();
    for (DataVecItor itor = begin; itor != end; ++itor)
    {
        if (!(*itor).data)
            continue;
        addMemory((*itor).data->getSize());
    }
}
//>---------------------------------------------------------------------------------------