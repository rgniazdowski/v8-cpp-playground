#pragma once
#ifndef FG_INC_UTIL_UNPACK_CALLER
#define FG_INC_UTIL_UNPACK_CALLER

#include <tuple>
#include <string>
#include <vector>
#include <functional>

namespace util
{
    template <typename... Args>
    struct args_pack
    {
    };

    template <typename... Args>
    inline std::vector<std::string> args_typeid_names(args_pack<Args...>)
    {
        return std::vector<std::string>({std::string(typeid(Args).name())...});
    }

    template <typename FuncType,
              typename Traits = function_traits<FuncType>,
              typename ArgsType = typename Traits::args>
    inline std::vector<std::string> args_typeid_names(void)
    {
        return args_typeid_names(ArgsType());
    }

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits_defs
    {
        static constexpr size_t arity = sizeof...(Args);

        using std_function = std::function<ReturnType(Args...)>;
        using result_type = ReturnType;
        using args = args_pack<Args...>;
        using args_tuple = std::tuple<Args...>;
        using class_type = ClassType;

        template <size_t i>
        struct arg
        {
            using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
        };
    };

    template <>
    struct function_traits_defs<void, void>
    {
        static constexpr size_t arity = 0;
        using std_function = std::function<void()>;
        using result_type = void;
        using args = args_pack<void>;
        using args_tuple = void;
        using class_type = void;

        template <size_t i>
        struct arg
        {
            using type = void;
        };
    };
    //#-----------------------------------------------------------------------------------

    template <typename T>
    struct function_traits_impl : function_traits_defs<void, void> {};

    template <typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType(Args...)>
        : function_traits_defs<void, ReturnType, Args...>
    {
    };

    template <typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType (*)(Args...)>
        : function_traits_defs<void, ReturnType, Args...>
    {
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType (ClassType::*)(Args...)>
        : function_traits_defs<ClassType, ReturnType, Args...>
    {
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType (ClassType::*)(Args...) const>
        : function_traits_defs<ClassType, ReturnType, Args...>
    {
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType (ClassType::*)(Args...) const &>
        : function_traits_defs<ClassType, ReturnType, Args...>
    {
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType (ClassType::*)(Args...) const &&>
        : function_traits_defs<ClassType, ReturnType, Args...>
    {
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType (ClassType::*)(Args...) volatile>
        : function_traits_defs<ClassType, ReturnType, Args...>
    {
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType (ClassType::*)(Args...) volatile &>
        : function_traits_defs<ClassType, ReturnType, Args...>
    {
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType (ClassType::*)(Args...) volatile &&>
        : function_traits_defs<ClassType, ReturnType, Args...>
    {
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType (ClassType::*)(Args...) const volatile>
        : function_traits_defs<ClassType, ReturnType, Args...>
    {
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType (ClassType::*)(Args...) const volatile &>
        : function_traits_defs<ClassType, ReturnType, Args...>
    {
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType (ClassType::*)(Args...) const volatile &&>
        : function_traits_defs<ClassType, ReturnType, Args...>
    {
    };

    template <typename T, typename V = void>
    struct function_traits : function_traits_impl<T>
    {
    };

    template <typename T>
    struct function_traits<T, decltype((void)&T::operator())>
        : function_traits_impl<decltype(&T::operator())>
    {
    };

    template <size_t... Indices>
    struct indices
    {
        using next = indices<Indices..., sizeof...(Indices)>;
    };
    template <size_t N>
    struct build_indices
    {
        using type = typename build_indices<N - 1>::type::next;
    };
    template <>
    struct build_indices<0>
    {
        using type = indices<>;
    };
    template <size_t N>
    using BuildIndices = typename build_indices<N>::type;
    //#-----------------------------------------------------------------------------------

    namespace details
    {
        template <typename FuncType,
                  typename VecType,
                  typename FirstArgType,
                  typename Traits = function_traits<FuncType>,
                  typename ReturnType = typename Traits::result_type,
                  bool IsFirstArgVec = std::is_same_v<std::decay_t<VecType>, std::decay_t<FirstArgType>>>
        std::enable_if_t<Traits::arity == 1 && IsFirstArgVec == true, ReturnType>
        do_call_arg0(FuncType func, VecType &args)
        {
            //* First argument type is the same as input vector for arguments, pass it as is
            return func(args);
        }
        //#-------------------------------------------------------------------------------

        template <typename FuncType,
                  typename VecType,
                  typename FirstArgType,
                  typename Traits = function_traits<FuncType>,
                  typename ReturnType = typename Traits::result_type,
                  bool IsFirstArgVec = std::is_same_v<std::decay_t<VecType>, std::decay_t<FirstArgType>>>
        std::enable_if_t<Traits::arity == 1 && IsFirstArgVec == false, ReturnType>
        do_call_arg0(FuncType func, VecType &args)
        {
            //* First argument is of different type - need to choose the first element and cast it
            return func(*args[0]);
        }
        //#-------------------------------------------------------------------------------

        template <typename FuncType,
                  typename VecType,
                  size_t... I,
                  typename Traits = function_traits<FuncType>,
                  typename ReturnType = typename Traits::result_type,
                  typename FirstArg = Traits::arg<0>>
        std::enable_if_t<Traits::arity == 1, ReturnType>
        do_call(FuncType func, VecType &args, indices<I...>)
        {
            using FirstArgType = typename FirstArg::type; //> select first argument type
            assert(args.size() >= Traits::arity);
            return do_call_arg0<FuncType, VecType, FirstArgType>(func, args); //! function call
        }
        //#-------------------------------------------------------------------------------

        template <typename FuncType,
                  typename VecType,
                  size_t... I,
                  typename Traits = function_traits<FuncType>,
                  typename ReturnType = typename Traits::result_type>
        std::enable_if_t<Traits::arity != 1, ReturnType>
        do_call(FuncType func, VecType &args, indices<I...>)
        {
            assert(args.size() >= Traits::arity);
            return func(*args[I]...); //! function call
        }
        //#-------------------------------------------------------------------------------
    } //> namespace details

    namespace details
    {
        template <typename ClassType,
                  typename FuncType,
                  typename VecType,
                  typename FirstArgType,
                  typename Traits = function_traits<FuncType>,
                  typename ReturnType = typename Traits::result_type,
                  bool IsFirstArgVec = std::is_same_v<std::decay_t<VecType>, std::decay_t<FirstArgType>>>
        std::enable_if_t<Traits::arity == 1 && IsFirstArgVec == true, ReturnType>
        do_call_arg0(ClassType *obj, FuncType func, VecType &args)
        {
            //* First argument type is the same as input vector for arguments, pass it as is
            return (obj->*func)(args);
        }
        //#-------------------------------------------------------------------------------

        template <typename ClassType,
                  typename FuncType,
                  typename VecType,
                  typename FirstArgType,
                  typename Traits = function_traits<FuncType>,
                  typename ReturnType = typename Traits::result_type,
                  bool IsFirstArgVec = std::is_same_v<std::decay_t<VecType>, std::decay_t<FirstArgType>>>
        std::enable_if_t<Traits::arity == 1 && IsFirstArgVec == false, ReturnType>
        do_call_arg0(ClassType *obj, FuncType func, VecType &args)
        {
            //* First argument is of different type - need to choose the first element and cast it
            return (obj->*func)(*args[0]);
        }
        //#-------------------------------------------------------------------------------

        template <typename ClassType,
                  typename FuncType,
                  typename VecType,
                  size_t... I,
                  typename Traits = function_traits<FuncType>,
                  typename ReturnType = typename Traits::result_type,
                  typename FirstArg = Traits::arg<0>>
        std::enable_if_t<Traits::arity == 1, ReturnType>
        do_call(ClassType *obj, FuncType func, VecType &args, indices<I...>)
        {
            using FirstArgType = typename FirstArg::type; //> select first argument type
            assert(args.size() >= Traits::arity);
            return do_call_arg0<ClassType, FuncType, VecType, FirstArgType>(obj, func, args); //! member call
        }
        //#-------------------------------------------------------------------------------

        template <typename ClassType,
                  typename FuncType,
                  typename VecType,
                  size_t... I,
                  typename Traits = function_traits<FuncType>,
                  typename ReturnType = typename Traits::result_type>
        std::enable_if_t<Traits::arity != 1, ReturnType>
        do_call(ClassType *obj, FuncType func, VecType &args, indices<I...>)
        {
            assert(args.size() >= Traits::arity);
            return (obj->*func)(*args[I]...); //! member call
        }
        //#-------------------------------------------------------------------------------
    } //> namespace details

    template <typename FuncType,
              typename VecType,
              typename Traits = function_traits<FuncType>,
              typename ReturnType = typename Traits::result_type>
    ReturnType unpack_caller(FuncType func, VecType &args)
    {
        return details::do_call(func, args, BuildIndices<Traits::arity>());
    }
    //#-----------------------------------------------------------------------------------

    template <typename ClassType,
              typename FuncType,
              typename VecType,
              typename Traits = function_traits<FuncType>,
              typename ReturnType = typename Traits::result_type>
    ReturnType unpack_caller(ClassType *obj, FuncType func, VecType &args)
    {
        return details::do_call(obj, func, args, BuildIndices<Traits::arity>());
    }
    //#-----------------------------------------------------------------------------------
} //> namespace util

#endif //> FG_INC_UTIL_UNPACK_CALLER