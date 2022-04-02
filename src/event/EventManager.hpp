#pragma once
#ifndef FG_INC_EVENT_MANAGER
#define FG_INC_EVENT_MANAGER

#include <Manager.hpp>

#include <event/EventDefinitions.hpp>
#include <event/EventHelper.hpp>

#include <mutex>

namespace event
{
    class EventManager : public base::Manager<EventManager>
    {
    public:
        using base_type = base::Manager<EventManager>;
        using self_type = EventManager;

        using EventFilterFunction = std::function<bool(const ThrownEvent &)>;
        using TimerFilterFunction = std::function<bool(const TimerEntryInfo &)>;

    public:
        /// Maximum number of allocated internal event structures
        /// When number of event structures reaches MAX and
        /// free slots are empty - this would mean that event queue is full
        static const unsigned int MAX_EVENT_STRUCTS = 256;
        /// Maximum number of thrown events
        static const unsigned int MAX_THROWN_EVENTS = 256;
        /// This is initial allocation for pointer vectors (initial capacity)
        static const unsigned int INITIAL_PTR_VEC_SIZE = 128;

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
        //#-------------------------------------------------------------------------------

        /**
         * This adds event to the waiting queue and moves the input arguments
         * @param eventCode
         * @param list
         */
        ThrownEvent &throwEvent(Type eventCode, WrappedArgs &args);

        template <typename... Args>
        ThrownEvent &throwEvent(Type eventCode, Args &&...args)
        {
            WrappedArgs wrapped = {util::WrappedValue::wrap(args)...};
            return throwEvent(eventCode, wrapped);
        }

        //#-------------------------------------------------------------------------------

        bool isRegisteredCallback(Type eventCode, util::Callback *pCallback);
        Type isRegisteredCallback(util::Callback *pCallback);

        template <typename MethodType,
                  typename Traits = util::function_traits<MethodType>,
                  typename UserClass = Traits::class_type,
                  bool is_function = std::is_function_v<MethodType>,
                  bool is_member = std::is_member_function_pointer_v<MethodType>,
                  bool is_void = std::is_void_v<UserClass>>
        typename std::enable_if<is_function == true && is_member == true && is_void == false, util::Callback *>::type
        isRegistered(Type eventCode, MethodType methodMember, UserClass *pObject)
        {
            if (eventCode == Type::Invalid || !methodMember)
                return nullptr;
            const std::lock_guard<std::mutex> lock(m_mutexEventBinds);
            auto found = m_eventBinds.find(eventCode);
            if (found == m_eventBinds.end())
                return nullptr;
            auto &callbacks = found->second;
            for (auto callback : callbacks)
            {
                if (!util::isMethodCallback(callback))
                    continue;
                auto objectEquals = static_cast<util::MethodCallback<UserClass> *>(callback)->getObject() == pObject;
                auto methodEquals = util::BindingHelper::compare<MethodType>(callback->getBinding(), methodMember);
                if (methodEquals && ((pObject != nullptr && objectEquals) || (pObject == nullptr)))
                    return callback;
            } //# for each callback
            return nullptr;
        } //> isRegistered(...)
        //>-------------------------------------------------------------------------------

        template <typename FunctionType,
                  typename Traits = util::function_traits<FunctionType>,
                  typename UserClass = Traits::class_type,
                  bool is_function = std::is_function_v<FunctionType>,
                  bool is_member = std::is_member_function_pointer_v<FunctionType>,
                  bool is_void = std::is_void_v<UserClass>>
        typename std::enable_if<is_function == true && is_member == false && is_void == true, util::Callback *>::type
        isRegistered(Type eventCode, FunctionType function)
        {
            if (eventCode == Type::Invalid || !function)
                return nullptr;
            const std::lock_guard<std::mutex> lock(m_mutexEventBinds);
            auto found = m_eventBinds.find(eventCode);
            if (found == m_eventBinds.end())
                return nullptr;
            auto &callbacks = found->second;
            for (auto callback : callbacks)
            {
                if (util::isFunctionCallback(callback) &&
                    util::BindingHelper::compare<FunctionType>(callback->getBinding(), function))
                    return callback;
            } //# for each callback
            return nullptr;
        } //> isRegistered(...)
        //>-------------------------------------------------------------------------------

        template <typename UserClass,
                  typename Traits = util::function_traits<UserClass>,
                  typename UserClassNested = Traits::class_type,
                  bool is_function = std::is_function_v<UserClass>,
                  bool is_member = std::is_member_function_pointer_v<UserClass>,
                  bool is_void = std::is_void_v<UserClass>>
        typename std::enable_if<is_function == false && is_member == false && is_void == false, util::Callback *>::type
        isRegistered(Type eventCode, UserClass *pObject)
        {
            if (eventCode == Type::Invalid || !pObject)
                return nullptr;
            const std::lock_guard<std::mutex> lock(m_mutexEventBinds);
            auto found = m_eventBinds.find(eventCode);
            if (found == m_eventBinds.end())
                return nullptr;
            auto &callbacks = found->second;
            for (auto callback : callbacks)
            {
                if (util::isMethodCallback(callback) &&
                    static_cast<util::MethodCallback<UserClass> *>(callback)->getObject() == pObject)
                    return callback;
            } //# for each callback
            return nullptr;
        } //> isRegistered(...)
        //>-------------------------------------------------------------------------------

        template <typename MethodType,
                  typename Traits = util::function_traits<MethodType>,
                  typename UserClass = Traits::class_type,
                  bool is_function = std::is_function_v<MethodType>,
                  bool is_member = std::is_member_function_pointer_v<MethodType>,
                  bool is_void = std::is_void_v<UserClass>>
        typename std::enable_if<is_function == true && is_member == true && is_void == false, Type>::type
        isRegistered(MethodType methodMember, UserClass *pObject)
        {
            if (!methodMember)
                return Type::Invalid;
            const std::lock_guard<std::mutex> lock(m_mutexEventBinds);
            Type foundEvent = Type::Invalid;
            for (auto &it : m_eventBinds)
            {
                for (auto callback : it.second)
                {
                    if (!util::isMethodCallback(callback))
                        continue;
                    auto objectEquals = static_cast<util::MethodCallback<UserClass> *>(callback)->getObject() == pObject;
                    auto methodEquals = util::BindingHelper::compare<MethodType>(callback->getBinding(), methodMember);
                    if (methodEquals && ((pObject != nullptr && objectEquals) || (pObject == nullptr)))
                        foundEvent = it.first;
                    if (foundEvent != Type::Invalid)
                        break;
                }
                if (foundEvent != Type::Invalid)
                    break;
            } //# for each bound event type
            return foundEvent;
        } //> isRegistered(...)
        //>-------------------------------------------------------------------------------

        template <typename FunctionType,
                  typename Traits = util::function_traits<FunctionType>,
                  typename UserClass = Traits::class_type,
                  bool is_function = std::is_function_v<FunctionType>,
                  bool is_member = std::is_member_function_pointer_v<FunctionType>,
                  bool is_void = std::is_void_v<UserClass>>
        typename std::enable_if<is_function == true && is_member == false && is_void == true, Type>::type
        isRegistered(FunctionType function)
        {
            if (!function)
                return Type::Invalid;
            const std::lock_guard<std::mutex> lock(m_mutexEventBinds);
            Type foundEvent = Type::Invalid;
            for (auto &it : m_eventBinds)
            {
                for (auto callback : it.second)
                {
                    if (util::isFunctionCallback(callback) &&
                        util::BindingHelper::compare<FunctionType>(callback->getBinding(), function))
                        foundEvent = it.first;
                    if (foundEvent != Type::Invalid)
                        break;
                }
                if (foundEvent != Type::Invalid)
                    break;
            } //# for each bound event type
            return foundEvent;
        } //> isRegistered(...)
        //>-------------------------------------------------------------------------------

        template <typename UserClass,
                  typename Traits = util::function_traits<UserClass>,
                  typename UserClassNested = Traits::class_type,
                  bool is_function = std::is_function_v<UserClass>,
                  bool is_member = std::is_member_function_pointer_v<UserClass>,
                  bool is_void = std::is_void_v<UserClass>>
        typename std::enable_if<is_function == false && is_member == false && is_void == false, Type>::type
        isRegistered(UserClass *pObject)
        {
            if (!pObject)
                return Type::Invalid;
            const std::lock_guard<std::mutex> lock(m_mutexEventBinds);
            Type foundEvent = Type::Invalid;
            for (auto &it : m_eventBinds)
            {
                for (auto callback : it.second)
                {
                    if (util::isMethodCallback(callback) &&
                        static_cast<util::MethodCallback<UserClass> *>(callback)->getObject() == pObject)
                        foundEvent = it.first;
                    if (foundEvent != Type::Invalid)
                        break;
                }
                if (foundEvent != Type::Invalid)
                    break;
            } //# for each bound event type
            return foundEvent;
        } //> isRegistered(...)
        //>-------------------------------------------------------------------------------

        unsigned int executeEvent(Type eventCode);
        unsigned int executeEvent(ThrownEvent &thrownEvent);
        unsigned int executeEvent(Type eventCode, WrappedArgs &args);

        //#-------------------------------------------------------------------------------

        util::Callback *addCallback(util::Callback *pCallback, Type eventCode);

        template <typename MethodType, typename UserClass>
        util::Callback *addCallback(Type eventCode, MethodType methodMember, UserClass *pObject)
        {
            if (!methodMember || (int)eventCode < 0 || !pObject)
                return nullptr;
            return addCallback((util::Callback *)util::MethodCallback<UserClass>::create(methodMember, pObject), eventCode);
        }
        //>-------------------------------------------------------------------------------

        template <typename FunctionType>
        util::Callback *addCallback(Type eventCode, FunctionType function)
        {
            if (!function || (int)eventCode < 0)
                return nullptr;
            return addCallback((util::Callback *)util::FunctionCallback::create(function), eventCode);
        }
        //>-------------------------------------------------------------------------------

        bool removeCallback(Type eventCode, util::Callback *pCallback);

        /**
         * This will remove all callbacks bind to the given event. The destructors
         * will not be called.
         * @param eventCode
         * @return
         */
        CallbacksVec removeCallbacks(Type eventCode);

        bool deleteCallback(Type eventCode, util::Callback *&pCallback);
        size_t deleteCallbacks(Type eventCode);

        //#-------------------------------------------------------------------------------

        uint32_t addTimeout(util::Callback *pCallback, const int timeout, WrappedArgs &args);

        template <typename FunctionType>
        uint32_t addTimeout(const int timeout, FunctionType function,
                            WrappedArgs &args = WrappedArgs(),
                            const std::initializer_list<std::string> &argNames = {})
        {
            const std::lock_guard<std::mutex> lock(m_mutexTimers);
            m_timerEntries.emplace(std::move(
                TimerHelper::function<TimerEntryInfo::TIMEOUT, FunctionType>(
                    timeout, function, argNames)
                    .setArgs(std::move(args))));
            return m_timerEntries.back().getId();
        }
        //>-------------------------------------------------------------------------------

        template <typename MethodType, typename Traits = util::function_traits<MethodType>,
                  typename UserClass = typename Traits::class_type>
        uint32_t addTimeout(const int timeout, UserClass *pObject,
                            MethodType methodMember,
                            WrappedArgs &args = WrappedArgs(),
                            const std::initializer_list<std::string> &argNames = {})
        {
            const std::lock_guard<std::mutex> lock(m_mutexTimers);
            m_timerEntries.emplace(std::move(
                TimerHelper::method<TimerEntryInfo::TIMEOUT, MethodType>(
                    timeout, pObject, methodMember, argNames)
                    .setArgs(std::move(args))));
            return m_timerEntries.back().getId();
        }
        //>-------------------------------------------------------------------------------

        TimerEntryInfo *getTimer(const uint32_t id);
        TimerEntryInfo const *getTimer(const uint32_t id) const;

        inline bool hasTimers(void) const
        {
            const std::lock_guard<std::mutex> lock(m_mutexTimers);
            return !m_timerEntries.empty();
        }
        bool hasTimer(const uint32_t id) const;
        bool removeTimer(const uint32_t id);
        size_t removeTimers(const std::vector<uint32_t> &ids);
        size_t removeInactiveTimers(void);
        bool removeTimer(const util::Callback *pCallback);

        inline bool removeTimeout(const uint32_t id) { return removeTimer(id); };
        inline bool removeTimeout(const util::Callback *pCallback) { return removeTimer(pCallback); }

        //#-------------------------------------------------------------------------------

        uint32_t addInterval(util::Callback *pCallback, const int interval,
                             const int repeats = -1, WrappedArgs &args = WrappedArgs());

        template <typename FunctionType>
        uint32_t addInterval(const int interval, FunctionType function,
                             const int repeats = -1, WrappedArgs &args = WrappedArgs(),
                             const std::initializer_list<std::string> &argNames = {})
        {
            const std::lock_guard<std::mutex> lock(m_mutexTimers);
            m_timerEntries.emplace(std::move(
                TimerHelper::function<TimerEntryInfo::INTERVAL, FunctionType>(
                    interval, function, repeats, argNames)
                    .setArgs(std::move(args))));
            return m_timerEntries.back().getId();
        }
        //>-------------------------------------------------------------------------------

        template <typename MethodType,
                  typename Traits = util::function_traits<MethodType>,
                  typename UserClass = typename Traits::class_type>
        uint32_t addInterval(const int interval, UserClass *pObject,
                             MethodType methodMember,
                             const int repeats = -1, WrappedArgs &args = WrappedArgs(),
                             const std::initializer_list<std::string> &argNames = {})
        {
            const std::lock_guard<std::mutex> lock(m_mutexTimers);
            m_timerEntries.emplace(std::move(
                TimerHelper::method<TimerEntryInfo::INTERVAL, MethodType>(
                    interval, pObject, methodMember, repeats, argNames)
                    .setArgs(std::move(args))));
            return m_timerEntries.back().getId();
        }
        //>-------------------------------------------------------------------------------

        inline bool removeInterval(const uint32_t id) { return removeTimer(id); };
        inline bool removeInterval(const util::Callback *pCallback) { return removeTimer(pCallback); }

        //#-------------------------------------------------------------------------------

        //?void addEventFilter(const EventFilterFunction &eventFilter);
        //?void addTimerFilter(const TimerFilterFunction &timerFilter);

        /**
         * Execute (finalized) all events waiting in a queue
         * This function must be called in every frame in one of the threads
         * (or just the main thread)
         */
        void processEventsAndTimers(void);
        void processTimers(void);
        void processEvents(void);

    private:
        void pushToFreeSlot(EventBase *pEventStruct);
        void resetArguments(WrappedArgs &args);

    private:
        /// Binding for all global events
        CallbacksBindingMap m_eventBinds;
        /// Events queue (message queue so to speak)
        EventsQueue m_eventsQueue;
        /// Pool with timers - these are one shot timeouts and intervals
        TimerEntries m_timerEntries;
        ///
        uint32_t m_cleanupIntervalId;
        ///
        std::vector<uint32_t> m_markedTimeouts;
        ///
        EventsPtrVec m_eventStructs;
        ///
        EventsPtrVec m_eventStructsFreeSlots;
        ///
        mutable std::mutex m_mutexEventsQueue;
        ///
        mutable std::mutex m_mutexEventBinds;
        ///
        mutable std::mutex m_mutexTimers;
    }; //# class EventManager
    //#-----------------------------------------------------------------------------------
} //> namespace event

#endif //> FG_INC_EVENT_MANAGER