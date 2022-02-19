#pragma once
#ifndef FG_INC_UTIL_UNPACK_CALLER
#define FG_INC_UTIL_UNPACK_CALLER

namespace util
{
    template <typename ReturnType, typename... Args>
    struct function_traits_defs
    {
        static constexpr size_t arity = sizeof...(Args);

        using result_type = ReturnType;

        template <size_t i>
        struct arg
        {
            using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
        };
    };

    template <typename T>
    struct function_traits_impl;

    template <typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType(Args...)>
        : function_traits_defs<ReturnType, Args...>
    {
    };

    template <typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType (*)(Args...)>
        : function_traits_defs<ReturnType, Args...>
    {
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType (ClassType::*)(Args...)>
        : function_traits_defs<ReturnType, Args...>
    {
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType (ClassType::*)(Args...) const>
        : function_traits_defs<ReturnType, Args...>
    {
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType (ClassType::*)(Args...) const &>
        : function_traits_defs<ReturnType, Args...>
    {
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType (ClassType::*)(Args...) const &&>
        : function_traits_defs<ReturnType, Args...>
    {
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType (ClassType::*)(Args...) volatile>
        : function_traits_defs<ReturnType, Args...>
    {
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType (ClassType::*)(Args...) volatile &>
        : function_traits_defs<ReturnType, Args...>
    {
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType (ClassType::*)(Args...) volatile &&>
        : function_traits_defs<ReturnType, Args...>
    {
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType (ClassType::*)(Args...) const volatile>
        : function_traits_defs<ReturnType, Args...>
    {
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType (ClassType::*)(Args...) const volatile &>
        : function_traits_defs<ReturnType, Args...>
    {
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits_impl<ReturnType (ClassType::*)(Args...) const volatile &&>
        : function_traits_defs<ReturnType, Args...>
    {
    };

    template <typename T, typename V = void>
    struct function_traits
        : function_traits_impl<T>
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

    namespace details
    {
        template <typename FuncType,
                  typename VecType,
                  size_t... I,
                  typename Traits = function_traits<FuncType>,
                  typename ReturnType = typename Traits::result_type>
        ReturnType do_call(FuncType func,
                           VecType &args,
                           indices<I...>)
        {
            assert(args.size() >= Traits::arity);
            return func(*args[I]...);
        }
        template <typename ClassType,
                  typename FuncType,
                  typename VecType,
                  size_t... I,
                  typename Traits = function_traits<FuncType>,
                  typename ReturnType = typename Traits::result_type>
        ReturnType do_call(ClassType *obj,
                           FuncType func,
                           VecType &args,
                           indices<I...>)
        {
            assert(args.size() >= Traits::arity);
            return (obj->*func)(*args[I]...);
        }
    } // namespace details

    template <typename FuncType,
              typename VecType,
              typename Traits = function_traits<FuncType>,
              typename ReturnType = typename Traits::result_type>
    ReturnType unpack_caller(FuncType func,
                             VecType &args)
    {
        return details::do_call(func, args, BuildIndices<Traits::arity>());
    }
    template <typename ClassType,
              typename FuncType,
              typename VecType,
              typename Traits = function_traits<FuncType>,
              typename ReturnType = typename Traits::result_type>
    ReturnType unpack_caller(
        ClassType *obj,
        FuncType func,
        VecType &args)
    {
        return details::do_call(obj, func, args, BuildIndices<Traits::arity>());
    }
} //> namespace util

#endif //> FG_INC_UTIL_UNPACK_CALLER