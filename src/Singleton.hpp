
#pragma once
#ifndef FG_INC_SINGLETON
#define FG_INC_SINGLETON

namespace fg
{
    template <typename Class>
    class Singleton
    {
    private:
        inline static bool _instanceFlag = false;
        inline static Class *_instance = nullptr;

    protected:
        Singleton() {}

    public:
        template <typename... Ts>
        static Class *instance(Ts... args)
        {
            if (!_instanceFlag || !_instance)
            {
                if (!_instance)
                    _instance = new Class(std::forward<Ts>(args)...);
                _instanceFlag = true;
                return _instance;
            }
            else
            {
                return _instance;
            }
        }
        static void deleteInstance()
        {
            if (_instanceFlag || _instance)
            {
                _instanceFlag = false;
                if (_instance)
                    delete _instance;
                _instance = nullptr;
            }
        }
        virtual ~Singleton()
        {
            _instanceFlag = false;
        }
    }; //# class Singleton
}

#endif /* FG_INC_SINGLETON */
