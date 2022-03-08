#pragma once
#ifndef FG_INC_CALLBACKS_HELPER
#define FG_INC_CALLBACKS_HELPER

#include <functional>

namespace util
{
    struct CallbackHelper
    {
        CallbackHelper() = delete;
        CallbackHelper(const CallbackHelper &other) = delete;

        template <typename ReturnType, typename... Args>
        static std::function<ReturnType(Args...)> function(ReturnType (*input)(Args...))
        {
            return [input](Args &&...args)
            {
                return input(std::forward<Args>(args)...);
            };
        }

        template <typename UserClass, typename ReturnType, typename... Args>
        static std::function<ReturnType(Args...)> method(UserClass *pObject, ReturnType (UserClass::*input)(Args...))
        {
            return [pObject, input](Args &&...args)
            {
                return (pObject->*input)(std::forward<Args>(args)...);
            };
        }
    }; //# CallbackHelper

} //> namespace util

#endif //> FG_INC_CALLBACKS_HELPER