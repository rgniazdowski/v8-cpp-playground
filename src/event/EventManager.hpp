#pragma once
#ifndef FG_INC_EVENT_MANAGER
#define FG_INC_EVENT_MANAGER

#include <Manager.hpp>

#include <event/EventDefinitions.hpp>
#include <event/EventHelper.hpp>

namespace event
{
    class EventManager : public base::Manager<EventManager>
    {
    public:
        using base_type = base::Manager<EventManager>;
        using self_type = EventManager;

    public:
        /// Maximum number of allocated internal event structures
        /// When number of event structures reaches MAX and
        /// free slots are empty - this would mean that event queue is full
        static const unsigned int MAX_EVENT_STRUCTS = 256;
        /// Maximum number of thrown events
        static const unsigned int MAX_THROWN_EVENTS = 256;
        /// This is initial allocation for pointer vectors (initial capacity)
        static const unsigned int INITIAL_PTR_VEC_SIZE = 32;

    public:
        /**
         * Default constructor for Event Manager object
         */
        explicit EventManager();
        /**
         * Default destructor for Event Manager object
         */
        virtual ~EventManager();

    public:
        virtual bool initialize(void) override;
        virtual bool destroy(void) override;

        EventBase *requestEventStruct(Type eventType);
        template <typename EventStruct, Type EventType>
        EventStruct *requestEventStruct(void)
        {
            return static_cast<EventStruct *>(requestEventStruct(EventType));
        }
        WrappedArgs *requestWrappedArgs(void);
        //#-------------------------------------------------------------------------------

        /**
         * This adds event to the waiting queue, the *list object needs to be allocated before,
         * after event callback execution argument list must be freed
         * @param eventCode
         * @param list
         */
        ThrownEvent &throwEvent(Type eventCode, WrappedArgs *args = nullptr);

        //#-------------------------------------------------------------------------------

        bool isRegistered(Type eventCode, util::Callback *pCallback);

        template <typename UserClass, typename ReturnType, typename... Args>
        util::Callback *isRegistered(Type eventCode,
                                     ReturnType (UserClass::*methodMember)(Args...),
                                     UserClass *pObject = nullptr);

        template <typename ReturnType, typename... Args>
        util::Callback *isRegistered(Type eventCode, ReturnType (*function)(Args...));

        template <typename UserClass>
        util::Callback *isRegistered(Type eventCode, UserClass *pObject);

        //#-------------------------------------------------------------------------------

        Type isRegistered(util::Callback *pCallback);

        template <typename UserClass, typename ReturnType, typename... Args>
        Type isRegistered(ReturnType (UserClass::*methodMember)(Args...),
                          UserClass *pObject = nullptr);

        template <typename ReturnType, typename... Args>
        Type isRegistered(ReturnType (*function)(Args...));

        template <typename UserClass>
        Type isRegistered(UserClass *pObject);

        //#-------------------------------------------------------------------------------

        unsigned int executeEvent(Type eventCode);
        unsigned int executeEvent(const ThrownEvent &thrownEvent);
        unsigned int executeEvent(Type eventCode, WrappedArgs *pArgs);

        //#-------------------------------------------------------------------------------

        util::Callback *addCallback(Type eventCode, util::Callback *pCallback);

        template <typename UserClass, typename ReturnType, typename... Args>
        util::Callback *addCallback(Type eventCode,
                                    ReturnType (UserClass::*methodMember)(Args...),
                                    UserClass *pObject);

        template <typename ReturnType, typename... Args>
        util::Callback *addCallback(Type eventCode, ReturnType (*function)(Args...));

        //#-------------------------------------------------------------------------------

        bool removeCallback(Type eventCode, util::Callback *pCallback);

        /**
         * This will remove all callbacks bind to the given event. The destructors
         * will not be called.
         * @param eventCode
         * @return
         */
        bool removeCallbacks(Type eventCode);

        bool deleteCallback(Type eventCode, util::Callback *&pCallback);
        bool deleteCallbacks(Type eventCode);

        //#-------------------------------------------------------------------------------

        TimerEntryInfo *addTimeout(util::Callback *pCallback, const int timeout, WrappedArgs &args);

        template <typename ReturnType, typename... Args>
        TimerEntryInfo *addTimeout(const int timeout, ReturnType (*function)(Args...),
                                   WrappedArgs &args, const std::initializer_list<std::string> &argNames = {});

        template <typename UserClass, typename ReturnType, typename... Args>
        TimerEntryInfo *addTimeout(const int timeout, UserClass *pObject,
                                   ReturnType (UserClass::*methodMember)(Args...),
                                   WrappedArgs &args,
                                   const std::initializer_list<std::string> &argNames = {});

        bool hasTimer(const uint32_t id);
        bool removeTimer(const uint32_t id);
        size_t removeTimers(const std::vector<uint32_t> &ids);
        bool removeTimer(const util::Callback *pCallback);

        inline bool removeTimeout(const uint32_t id) { return removeTimer(id); };
        inline bool removeTimeout(const util::Callback *pCallback) { return removeTimer(pCallback); }

        //#-------------------------------------------------------------------------------

        TimerEntryInfo *addInterval(const int interval, const int repeats,
                                    util::Callback *pCallback, WrappedArgs &args);

        template <typename ReturnType, typename... Args>
        TimerEntryInfo *addInterval(const int interval, const int repeats, ReturnType (*function)(Args...),
                                    WrappedArgs &args, const std::initializer_list<std::string> &argNames = {});

        template <typename UserClass, typename ReturnType, typename... Args>
        TimerEntryInfo *addInterval(const int interval, const int repeats, UserClass *pObject,
                                    ReturnType (UserClass::*methodMember)(Args...),
                                    WrappedArgs &args,
                                    const std::initializer_list<std::string> &argNames = {});

        inline bool removeInterval(const uint32_t id) { return removeTimer(id); };
        inline bool removeInterval(const util::Callback *pCallback) { return removeTimer(pCallback); }

        //#-------------------------------------------------------------------------------

        /**
         * Execute (finalized) all events waiting in a queue
         * This function must be called in every frame in one of the threads
         * (or just the main thread)
         */
        void executeEvents(void);

    private:
        void pushToFreeSlot(EventBase *pEventStruct);
        void pushToFreeSlot(WrappedArgs *pArgs);

    private:
        /// Binding for all global events
        CallbacksBindingMap m_eventBinds;
        /// Events queue (message queue so to speak)
        EventsQueue m_eventsQueue;
        /// Pool with timers - these are one shot timeouts and intervals
        TimerEntries m_timerEntries;
        ///
        EventsPtrVec m_eventStructs;
        ///
        EventsPtrVec m_eventStructsFreeSlots;
        ///
        WrappedArgsPtrVec m_argsLists;
        ///
        WrappedArgsPtrVec m_argsListsFreeSlots;
    }; //# class EventManager

    template <typename UserClass, typename ReturnType, typename... Args>
    util::Callback *EventManager::addCallback(Type eventCode,
                                              ReturnType (UserClass::*methodMember)(Args...),
                                              UserClass *pObject)
    {
        if (!methodMember || (int)eventCode < 0 || !pObject)
            return nullptr;
        return addCallback(eventCode, util::MethodCallback<UserClass>::create(methodMember, pObject));
    }
    //>-----------------------------------------------------------------------------------

    template <typename ReturnType, typename... Args>
    util::Callback *EventManager::addCallback(Type eventCode, ReturnType (*function)(Args...))
    {
        if (!function || (int)eventCode < 0)
            return nullptr;
        return addCallback(eventCode, util::FunctionCallback::create(function));
    }
    //>-----------------------------------------------------------------------------------

    template <typename UserClass, typename ReturnType, typename... Args>
    util::Callback *EventManager::isRegistered(Type eventCode,
                                               ReturnType (UserClass::*methodMember)(Args...),
                                               UserClass *pObject)
    {
        if (eventCode == Type::Invalid || !methodMember)
            return nullptr;
        auto found = m_eventBinds.find(eventCode);
        if (found == m_eventBinds.end())
            return nullptr;
        auto &callbacks = found->second;
        for (auto callback : callbacks)
        {
            if (util::isMethodCallback(callback))
            {
                auto objectEquals = static_cast<util::MethodCallback<UserClass> *>(callback)->getObject() == pObject;
                auto methodEquals = util::BindingHelper::compare<UserClass, ReturnType, Args...>(callback->getBinding(), methodMember);
                if (methodEquals && ((pObject != nullptr && objectEquals) || (pObject == nullptr)))
                    return callback;
            }
        } //# for each callback
        return nullptr;
    }
    //>-----------------------------------------------------------------------------------

    template <typename ReturnType, typename... Args>
    util::Callback *EventManager::isRegistered(Type eventCode, ReturnType (*function)(Args...))
    {
        if (eventCode == Type::Invalid || !function)
            return nullptr;
        auto found = m_eventBinds.find(eventCode);
        if (found == m_eventBinds.end())
            return nullptr;
        auto &callbacks = found->second;
        for (auto callback : callbacks)
        {
            if (util::isFunctionCallback(callback) && util::BindingHelper::compare<ReturnType, Args...>(callback->getBinding(), function))
                return callback;
        } //# for each callback
        return nullptr;
    }
    //>-----------------------------------------------------------------------------------

    template <typename UserClass>
    util::Callback *EventManager::isRegistered(Type eventCode, UserClass *pObject)
    {
        if (eventCode == Type::Invalid || !pObject)
            return nullptr;
        auto found = m_eventBinds.find(eventCode);
        if (found == m_eventBinds.end())
            return nullptr;
        auto &callbacks = found->second;
        for (auto callback : callbacks)
        {
            if (util::isMethodCallback(callback) && static_cast<util::MethodCallback<UserClass> *>(callback)->getObject() == pObject)
                return callback;
        } //# for each callback
        return nullptr;
    }
    //>-----------------------------------------------------------------------------------

    template <typename UserClass, typename ReturnType, typename... Args>
    Type EventManager::isRegistered(ReturnType (UserClass::*methodMember)(Args...),
                                    UserClass *pObject)
    {
        if (!methodMember)
            return Type::Invalid;
        Type foundEvent = Type::Invalid;
        for (auto &it : m_eventBinds)
        {
            for (auto callback : it.second)
            {
                if (util::isMethodCallback(callback))
                {
                    auto objectEquals = static_cast<util::MethodCallback<UserClass> *>(callback)->getObject() == pObject;
                    auto methodEquals = util::BindingHelper::compare<UserClass, ReturnType, Args...>(callback->getBinding(), methodMember);
                    if (methodEquals && ((pObject != nullptr && objectEquals) || (pObject == nullptr)))
                        foundEvent = it.first;
                }
                if (foundEvent != Type::Invalid)
                    break;
            }
            if (foundEvent != Type::Invalid)
                break;
        } //# for each bound event type
        return foundEvent;
    }
    //>-----------------------------------------------------------------------------------

    template <typename ReturnType, typename... Args>
    Type EventManager::isRegistered(ReturnType (*function)(Args...))
    {
        if (!function)
            return Type::Invalid;
        Type foundEvent = Type::Invalid;
        for (auto &it : m_eventBinds)
        {
            for (auto callback : it.second)
            {
                if (util::isFunctionCallback(callback) && util::BindingHelper::compare<ReturnType, Args...>(callback->getBinding(), function))
                    foundEvent = it.first;
                if (foundEvent != Type::Invalid)
                    break;
            }
            if (foundEvent != Type::Invalid)
                break;
        } //# for each bound event type
        return foundEvent;
    }
    //>-----------------------------------------------------------------------------------

    template <typename UserClass>
    Type EventManager::isRegistered(UserClass *pObject)
    {
        if (!pObject)
            return Type::Invalid;
        Type foundEvent = Type::Invalid;
        for (auto &it : m_eventBinds)
        {
            for (auto callback : it.second)
            {
                if (util::isMethodCallback(callback) && static_cast<util::MethodCallback<UserClass> *>(callback)->getObject() == pObject)
                    foundEvent = it.first;
                if (foundEvent != Type::Invalid)
                    break;
            }
            if (foundEvent != Type::Invalid)
                break;
        } //# for each bound event type
        return foundEvent;
    }
    //>-----------------------------------------------------------------------------------

    template <typename ReturnType, typename... Args>
    TimerEntryInfo *EventManager::addTimeout(const int timeout, ReturnType (*function)(Args...),
                                             WrappedArgs &args, const std::initializer_list<std::string> &argNames)
    {
        m_timerEntries.emplace(TimerHelper::function<TimerEntryInfo::TIMEOUT, ReturnType, Args..>(timeout, function, argNames).setArgs(std::move(args)));
        return &m_timerEntries.back();
    }
    //>-----------------------------------------------------------------------------------

    template <typename UserClass, typename ReturnType, typename... Args>
    TimerEntryInfo *EventManager::addTimeout(const int timeout, UserClass *pObject,
                                             ReturnType (UserClass::*methodMember)(Args...),
                                             WrappedArgs &args,
                                             const std::initializer_list<std::string> &argNames)
    {
        m_timerEntries.emplace(TimerHelper::method<TimerEntryInfo::TIMEOUT, UserClass, ReturnType, Args..>(timeout, pObject, methodMember, argNames).setArgs(std::move(args)));
        return &m_timerEntries.back();
    }
    //>-----------------------------------------------------------------------------------

    template <typename ReturnType, typename... Args>
    TimerEntryInfo *EventManager::addInterval(const int interval, const int repeats, ReturnType (*function)(Args...),
                                              WrappedArgs &args, const std::initializer_list<std::string> &argNames)
    {
        m_timerEntries.emplace(TimerHelper::function<TimerEntryInfo::INTERVAL, ReturnType, Args..>(interval, repeats, function, argNames).setArgs(std::move(args)));
        return &m_timerEntries.back();
    }
    //>-----------------------------------------------------------------------------------

    template <typename UserClass, typename ReturnType, typename... Args>
    TimerEntryInfo *EventManager::addInterval(const int interval, const int repeats, UserClass *pObject,
                                              ReturnType (UserClass::*methodMember)(Args...),
                                              WrappedArgs &args,
                                              const std::initializer_list<std::string> &argNames)
    {
        m_timerEntries.emplace(TimerHelper::method<TimerEntryInfo::INTERVAL, UserClass, ReturnType, Args..>(interval, repeats, pObject, methodMember, argNames).setArgs(std::move(args)));
        return &m_timerEntries.back();
    }
    //>-----------------------------------------------------------------------------------

} //> namespace event

#endif //> FG_INC_EVENT_MANAGER