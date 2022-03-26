#pragma once
#ifndef FG_INC_UTIL_BINDINGS
#define FG_INC_UTIL_BINDINGS

#include <magic_enum.hpp>

#include <typeinfo>
#include <functional>
#include <string>
#include <util/UniversalId.hpp>
#include <util/Vector.hpp>
#include <util/Util.hpp>
#include <util/UnpackCaller.hpp>

namespace util
{
#if 0
    template <class>
    inline constexpr bool is_char_pointer_v = false;

    template <>
    inline constexpr bool is_char_pointer_v<char *> = true;

    template <>
    inline constexpr bool is_char_pointer_v<const char *> = true;

    template <>
    inline constexpr bool is_char_pointer_v<char *const> = true;

    template <>
    inline constexpr bool is_char_pointer_v<const char *const> = true;

    template <class CharType>
    struct is_char_pointer : std::bool_constant<is_char_pointer_v<CharType>>
    {
    };
#endif
    class WrappedValue
    {
    public:
        enum Type
        {
            INVALID = 0,
            CHAR = 1,           // 8
            SIGNED_CHAR = 2,    // 8
            UNSIGNED_CHAR = 3,  // 8
            SHORT = 4,          // 16
            SIGNED_SHORT = 5,   // 16
            UNSIGNED_SHORT = 6, // 16
            INT = 7,            // 32
            UNSIGNED_INT = 8,   // 32
            LONG = 9,
            UNSIGNED_LONG = 10,
            LONG_LONG = 11,          // 64
            UNSIGNED_LONG_LONG = 12, // 64
            FLOAT = 13,              // 32
            DOUBLE = 14,             // 64
            BOOL = 15,               // bool
            STRING = 16,             // std::string / const char *
            EXTERNAL = 17
        };

        template <Type WrappedValueType>
        struct PrimitiveType
        {
            using type = void *;
        };

        template <>
        struct PrimitiveType<INVALID>
        {
            using type = decltype(nullptr);
        };
        template <>
        struct PrimitiveType<CHAR>
        {
            using type = char;
        };
        template <>
        struct PrimitiveType<SIGNED_CHAR>
        {
            using type = signed char;
        };
        template <>
        struct PrimitiveType<UNSIGNED_CHAR>
        {
            using type = unsigned char;
        };
        template <>
        struct PrimitiveType<SHORT>
        {
            using type = short;
        };
        template <>
        struct PrimitiveType<SIGNED_SHORT>
        {
            using type = signed short;
        };
        template <>
        struct PrimitiveType<UNSIGNED_SHORT>
        {
            using type = unsigned short;
        };
        template <>
        struct PrimitiveType<INT>
        {
            using type = int;
        };
        template <>
        struct PrimitiveType<UNSIGNED_INT>
        {
            using type = unsigned int;
        };
        template <>
        struct PrimitiveType<LONG>
        {
            using type = long;
        };
        template <>
        struct PrimitiveType<UNSIGNED_LONG>
        {
            using type = unsigned long;
        };
        template <>
        struct PrimitiveType<LONG_LONG>
        {
            using type = long long;
        };
        template <>
        struct PrimitiveType<UNSIGNED_LONG_LONG>
        {
            using type = unsigned long long;
        };
        template <>
        struct PrimitiveType<FLOAT>
        {
            using type = float;
        };
        template <>
        struct PrimitiveType<DOUBLE>
        {
            using type = double;
        };
        template <>
        struct PrimitiveType<BOOL>
        {
            using type = bool;
        };
        template <>
        struct PrimitiveType<STRING>
        {
            using type = std::string;
        };

        template <Type WrappedValueType>
        using primitive_type = typename PrimitiveType<WrappedValueType>::type;

        inline static std::string_view enum_name(Type type)
        {
            return magic_enum::enum_name(type);
        }

        inline static Type enum_value(std::string_view name)
        {
            auto type = magic_enum::enum_cast<Type>(name);
            return (type.has_value() ? type.value() : Type::INVALID);
        }

        static uint32_t getPrimitiveTypeId(Type type)
        {
            if (type == INVALID)
                return UniversalId<primitive_type<INVALID>>::id();
            else if (type == CHAR)
                return UniversalId<primitive_type<CHAR>>::id();
            else if (type == SIGNED_CHAR)
                return UniversalId<primitive_type<SIGNED_CHAR>>::id();
            else if (type == UNSIGNED_CHAR)
                return UniversalId<primitive_type<UNSIGNED_CHAR>>::id();
            else if (type == SHORT)
                return UniversalId<primitive_type<SHORT>>::id();
            else if (type == SIGNED_SHORT)
                return UniversalId<primitive_type<SIGNED_SHORT>>::id();
            else if (type == UNSIGNED_SHORT)
                return UniversalId<primitive_type<UNSIGNED_SHORT>>::id();
            else if (type == INT)
                return UniversalId<primitive_type<INT>>::id();
            else if (type == UNSIGNED_INT)
                return UniversalId<primitive_type<UNSIGNED_INT>>::id();
            else if (type == LONG)
                return UniversalId<primitive_type<LONG>>::id();
            else if (type == UNSIGNED_LONG)
                return UniversalId<primitive_type<UNSIGNED_LONG>>::id();
            else if (type == LONG_LONG)
                return UniversalId<primitive_type<LONG_LONG>>::id();
            else if (type == UNSIGNED_LONG_LONG)
                return UniversalId<primitive_type<UNSIGNED_LONG_LONG>>::id();
            else if (type == FLOAT)
                return UniversalId<primitive_type<FLOAT>>::id();
            else if (type == DOUBLE)
                return UniversalId<primitive_type<DOUBLE>>::id();
            else if (type == BOOL)
                return UniversalId<primitive_type<BOOL>>::id();
            else if (type == STRING)
                return UniversalId<primitive_type<STRING>>::id();
            return UniversalId<primitive_type<INVALID>>::id();
        }

        inline uint32_t getPrimitiveTypeId(void) const
        {
            if (!isExternal())
                return getPrimitiveTypeId(type);
            else
                return packed.external.tid;
        }

        static Type determineInternalType(const std::string &typeName)
        {
            if (!typeName.length())
                return INVALID;
            if (typeName.find("char const *") != std::string::npos || typeName.find("char [") != std::string::npos || typeName.find("std::string") != std::string::npos || typeName.find("std::basic_string") != std::string::npos)
                return STRING;
            else if (typeName.compare(typeid(char).name()) == 0)
                return CHAR;
            else if (typeName.compare(typeid(signed char).name()) == 0)
                return SIGNED_CHAR;
            else if (typeName.compare(typeid(unsigned char).name()) == 0)
                return UNSIGNED_CHAR;
            else if (typeName.compare(typeid(short).name()) == 0)
                return SHORT;
            else if (typeName.compare(typeid(signed short).name()) == 0)
                return SIGNED_SHORT;
            else if (typeName.compare(typeid(unsigned short).name()) == 0)
                return UNSIGNED_SHORT;
            else if (typeName.compare(typeid(int).name()) == 0)
                return INT;
            else if (typeName.compare(typeid(unsigned int).name()) == 0)
                return UNSIGNED_INT;
            else if (typeName.compare(typeid(long).name()) == 0)
                return LONG;
            else if (typeName.compare(typeid(unsigned long).name()) == 0)
                return UNSIGNED_LONG;
            else if (typeName.compare(typeid(long long).name()) == 0)
                return LONG_LONG;
            else if (typeName.compare(typeid(unsigned long long).name()) == 0)
                return UNSIGNED_LONG_LONG;
            else if (typeName.compare(typeid(float).name()) == 0)
                return FLOAT;
            else if (typeName.compare(typeid(double).name()) == 0)
                return DOUBLE;
            else if (typeName.compare(typeid(bool).name()) == 0)
                return BOOL;
            return EXTERNAL;
        }

    protected:
        struct External
        {
            uint64_t handle;
            uint32_t tid;
            void *pointer;
        };
        union
        {
            char c_val;
            short s_val;
            int i_val;
            float f_val;
            double d_val;
            long l_val;
            void *p_val;
            External external;
        } packed;
        Type type;
        std::string tname;
        std::string strvalue;

    public:
        inline const std::string &getTypeName(void) const noexcept { return tname; }

        inline bool isExternal(void) const noexcept { return type == EXTERNAL; }

        inline bool isString(void) const noexcept { return type == STRING; }

        inline bool isValid(void) const noexcept { return type != INVALID; }

        inline bool isEmpty(void) const noexcept { return type == INVALID; }

        inline Type getType(void) const noexcept { return type; }

        inline uint64_t getExternalHandle(void) const
        {
            if (type == EXTERNAL)
                return packed.external.handle;
            return 0;
        }

        template <typename DataType>
        inline DataType *getExternalPointer(void) const
        {
            if (type == EXTERNAL)
            {
                using data_type = std::decay_t<DataType>;
                if (packed.external.tid != 0 && packed.external.tid != UniversalId<data_type>::id())
                    return nullptr;
                return util::convert<data_type>::convertToPointer(packed.external.pointer, packed.external.handle);
            }
            return nullptr; // nothing else to return
        }

        template <>
        inline void *getExternalPointer(void) const
        {
            if (type == EXTERNAL)
                return packed.external.pointer;
            return nullptr;
        }

        template <typename InputType>
        inline bool checkType(bool strict) const
        {
            // #FIXME
            if (type == EXTERNAL && strict && UniversalId<std::decay_t<InputType>>::id() == packed.external.tid)
                return true;
            if (strict && tname.compare(typeid(InputType).name()) == 0)
                return true;
            if (!strict && strings::stristr(tname.c_str(), typeid(InputType).name()))
                return true;
            return false;
        }

    public:
        template <typename InputType>
        inline void set(const InputType &value)
        {
            tname = typeid(InputType).name();
            type = WrappedValue::determineInternalType(tname); // overwrite
            void *_data = &packed;
            InputType *holder = static_cast<InputType *>(_data);
            *holder = value; // directly paste it in
        }

        template <>
        inline void set<std::string>(const std::string &value)
        {
            tname = typeid(std::string).name();
            type = STRING;
            strvalue = value;
        }

        // template <unsigned int N>
        // void set(const char (&value)[N])
        //{
        //     /* outside of template */
        //     tname = typeid(value).name();
        //     type = STRING;
        //     size = sizeof(value);
        //     strvalue = value;
        // }

        void set(const char *value)
        {
            /* outside of template */
            tname = typeid(value).name();
            type = STRING;
            strvalue = value;
        }

        template <typename T>
        const T &get(void) const
        {
            const void *_data = &packed;
            const T *holder = static_cast<const T *>(_data);
            return *holder;
        }

        template <>
        const std::string &get(void) const noexcept { return strvalue; }

    public:
        WrappedValue() : tname(), type(INVALID)
        {
            std::fill_n((uint8_t *)(&packed), sizeof(packed), static_cast<uint8_t>(0));
        }

        WrappedValue(const char *_tname) : tname(_tname), type(WrappedValue::determineInternalType(_tname))
        {
            std::fill_n((uint8_t *)(&packed), sizeof(packed), static_cast<uint8_t>(0));
        }

        WrappedValue(const char *_tname, void *_external, uint64_t _handle, uint32_t _tid) : tname(_tname), type(EXTERNAL)
        {
            std::fill_n((uint8_t *)(&packed), sizeof(packed), static_cast<uint8_t>(0));
            packed.external.pointer = _external;
            packed.external.handle = _handle;
            packed.external.tid = _tid;
        }

        WrappedValue(const std::string &str, const char *_tname) : tname(_tname), type(STRING), strvalue(str)
        {
            std::fill_n((uint8_t *)(&packed), sizeof(packed), static_cast<uint8_t>(0));
        }

        ~WrappedValue() { reset(); }

        WrappedValue &operator=(const WrappedValue &input)
        {
            std::copy_n(&input.packed, 1, &packed); // copy one structure
            tname = input.tname;
            type = input.type;
            strvalue = input.strvalue;
            return *this;
        }

        WrappedValue &operator=(WrappedValue &&input) noexcept
        {
            if (this != &input)
            {
                type = input.type;
                tname = std::move(input.tname);
                strvalue = std::move(input.strvalue);
                std::copy_n(&input.packed, 1, &packed); // copy one structure
                std::fill_n((uint8_t *)(&input.packed), sizeof(input.packed), static_cast<uint8_t>(0));
                input.type = WrappedValue::INVALID;
            }
            return *this;
        }

        WrappedValue(const WrappedValue &input)
        {
            std::copy_n(&input.packed, 1, &packed); // copy one structure
            tname = input.tname;
            type = input.type;
            strvalue = input.strvalue;
        }

        WrappedValue(WrappedValue &&input) noexcept : type(input.type),
                                                      tname(std::move(input.tname)),
                                                      strvalue(std::move(input.strvalue))
        {
            std::copy_n(&input.packed, 1, &packed); // copy one structure
            std::fill_n((uint8_t *)(&input.packed), sizeof(input.packed), static_cast<uint8_t>(0));
            input.type = WrappedValue::INVALID;
        }

        void reset(void)
        {
            std::fill_n((uint8_t *)(&packed), sizeof(packed), static_cast<uint8_t>(0));
            type = INVALID;
            tname.clear();
            strvalue.clear();
        }
        //>-------------------------------------------------------------------------------

        template <typename InputType, bool is_pointer = std::is_pointer_v<InputType>>
        static typename std::enable_if<is_pointer == false, WrappedValue *>::type wrap(const InputType &value)
        {
            WrappedValue *pWrapped = new WrappedValue(typeid(value).name());
            pWrapped->set(value);
            return pWrapped;
        }

        template <typename InputType, bool is_pointer = std::is_pointer_v<InputType>>
        static typename std::enable_if<is_pointer == true, WrappedValue *>::type wrap(InputType value, uint32_t tid = 0)
        {
            return external<std::remove_pointer_t<InputType>>(value, 0, tid);
        }

        template <>
        static WrappedValue *wrap<const char *, true>(const char *value, uint32_t tid)
        {
            return new WrappedValue(std::string(value), typeid(value).name());
        }

        template <unsigned int N>
        static WrappedValue *wrap(const char (&value)[N])
        {
            return new WrappedValue(std::string(value), typeid(value).name());
        }

        template <>
        static WrappedValue *wrap<WrappedValue *, true>(WrappedValue *value, uint32_t tid)
        {
            return new WrappedValue(*value);
        }
        //>-------------------------------------------------------------------------------

        template <typename InputType, bool is_pointer = std::is_pointer_v<InputType>>
        static typename std::enable_if<is_pointer == false, WrappedValue>::type wrapInPlace(const InputType &value)
        {
            WrappedValue wrapped = WrappedValue(typeid(value).name());
            wrapped.set(value);
            return wrapped;
        }

        template <typename InputType, bool is_pointer = std::is_pointer_v<InputType>>
        static typename std::enable_if<is_pointer == true, WrappedValue>::type wrapInPlace(InputType value, uint32_t tid = 0)
        {
            return externalInPlace<std::remove_pointer_t<InputType>>(value, 0, tid);
        }

        template <>
        static WrappedValue wrapInPlace<const char *, true>(const char *value, uint32_t tid)
        {
            return WrappedValue(std::string(value), typeid(value).name());
        }

        template <unsigned int N>
        static WrappedValue wrapInPlace(const char (&value)[N])
        {
            return WrappedValue(std::string(value), typeid(value).name());
        }
        //>-------------------------------------------------------------------------------

        template <typename InputType>
        static WrappedValue *external(const InputType *value, uint64_t id, uint32_t tid = 0)
        {
            using data_type = std::decay_t<InputType>;
            return new WrappedValue(typeid(data_type).name(),
                                    !value ? nullptr : (void *)value,
                                    !value ? id : value->getIdentifier(),
                                    tid ? tid : UniversalId<data_type>::id());
        }

        template <typename InputType>
        static WrappedValue externalInPlace(const InputType *value, uint64_t id, uint32_t tid = 0)
        {
            using data_type = std::decay_t<InputType>;
            return WrappedValue(typeid(data_type).name(),
                                !value ? nullptr : (void *)value,
                                !value ? id : value->getIdentifier(),
                                tid ? tid : UniversalId<data_type>::id());
        }
        //>-------------------------------------------------------------------------------

    public:
        template <typename DataType>
        operator DataType() { return get<DataType>(); }

        operator std::string &() { return strvalue; }

        operator const char *()
        {
            if (type == STRING)
                return strvalue.c_str();
            return nullptr; // can't cast packed data
        }

        template <typename DataType>
        operator DataType *() { return getExternalPointer<DataType>(); }

        template <typename DataType>
        operator const DataType *() const { return getExternalPointer<DataType>(); }

        using Args = std::vector<WrappedValue *>;
    }; //# class WrappedValue
    //#-----------------------------------------------------------------------------------

    using WrappedArgs = WrappedValue::Args;

    inline WrappedArgs &reset_arguments(WrappedArgs &args)
    {
        for (auto idx = 0; idx < args.size(); idx++)
        {
            auto arg = args[idx];
            delete arg;
            args[idx] = nullptr;
        }
        args.clear();
        return args;
    }

    inline void copy_arguments(const WrappedArgs &args, WrappedArgs &output)
    {
        util::reset_arguments(output); // force clear the target (destructors are called)
        if (args.empty())
            return; // nothing to do
        output.reserve(args.size());
        for (auto &it : args)
            output.push_back(WrappedValue::wrap(it));
    }
    //#-----------------------------------------------------------------------------------

    /**
     * @brief
     *
     */
    class BindInfo
    {
        friend struct std::default_delete<BindInfo>;

    public:
        using Bindings = std::vector<BindInfo *>;
        enum Type
        {
            INVALID = 0x0,
            FUNCTION = 0x1,
            METHOD = 0x2,
            PROPERTY = 0x4,
            VARIABLE = 0x8
        };

        static std::string_view enum_name(Type type) { return magic_enum::enum_name(type); }
        static Type enum_value(std::string_view name)
        {
            auto type = magic_enum::enum_cast<Type>(name);
            return (type.has_value() ? type.value() : Type::INVALID);
        }

    protected:
        Type type;
        std::string name;

    protected:
        BindInfo(std::string_view _name, Type _type) : name(_name), type(_type) {}

        virtual ~BindInfo() {}

    public:
        constexpr bool isFunction(void) const noexcept { return type == FUNCTION; }
        constexpr bool isMethod(void) const noexcept { return type == METHOD; }
        constexpr bool isProperty(void) const noexcept { return type == PROPERTY; }
        constexpr bool isVariable(void) const noexcept { return type == VARIABLE; }
        constexpr Type getType(void) const noexcept { return type; }
        const std::string &getName(void) const noexcept { return name; }
    }; //# class BindInfo
    //#-----------------------------------------------------------------------------------

    /**
     * @brief
     *
     */
    class BindInfoFunctionWrappedBase
    {
    public:
        struct ParameterInfo
        {
            std::string name;
            std::string type;
        };

    protected:
        std::vector<ParameterInfo> parameters;
        std::string returnType;

        void setupParameters(const std::vector<std::string> &names, const std::vector<std::string> &types)
        {
            auto size = (names.size() + types.size() + 1) / 2;
            std::string name;
            std::string type;
            for (int idx = 0; idx < size; idx++)
            {
                name.clear();
                type.clear();
                bool gotName = idx < names.size();
                bool gotType = idx < types.size();
                if (!gotName && !gotType)
                    break; // all processed
                type.assign(!gotType ? "void" : types[idx]);
                if (!gotName)
                {
                    name.append("param");
                    name.append(std::to_string(idx + 1));
                }
                else
                    name = names[idx];
                parameters.push_back({name, type});
            } //# for each name or type
        }

        BindInfoFunctionWrappedBase(const std::vector<std::string> &names,
                                    const std::vector<std::string> &types,
                                    std::string_view _returnType) : parameters(), returnType(_returnType)
        {
            setupParameters(names, types);
        }
        virtual ~BindInfoFunctionWrappedBase() {}

    public:
        const std::vector<ParameterInfo> &getParameters(void) const { return parameters; }

        const std::string &getReturnType(void) const { return returnType; }
    }; //# BindInfoFunctionWrappedBase

    /**
     * @brief
     *
     */
    class BindInfoFunctionWrapped : public BindInfoFunctionWrappedBase
    {
    public:
        using base_type = BindInfoFunctionWrappedBase;
        using WrappedFunction = std::function<WrappedValue(const WrappedArgs &)>;

    protected:
        WrappedFunction wrappedFunction;

    public:
        BindInfoFunctionWrapped(const std::vector<std::string> &names,
                                const std::vector<std::string> &types,
                                std::string_view _returnType) : base_type(names, types, _returnType), wrappedFunction() {}
        virtual ~BindInfoFunctionWrapped() {}

        WrappedValue callWrapped(const WrappedArgs &args) const
        {
            if (wrappedFunction)
                return wrappedFunction(args);
            return WrappedValue(); // empty
        }
    }; //# BindInfoMethodWrapped
    //#-----------------------------------------------------------------------------------

    /**
     * @brief
     *
     * @tparam ReturnType
     * @tparam Args
     */
    template <typename ReturnType, typename... Args>
    class BindInfoFunction : public BindInfo, public virtual BindInfoFunctionWrapped
    {
        using DirectFunction = std::function<ReturnType(Args...)>;
        using FunctionType = ReturnType (*)(Args...);

    private:
        DirectFunction direct;
        size_t address;

        template <bool is_pointer = std::is_pointer<ReturnType>::value>
        typename std::enable_if<is_pointer == true>::type wrap(const DirectFunction &function)
        {
            this->wrappedFunction = [function](const WrappedArgs &args)
            {
                auto value = unpack_caller(function, args);
                return WrappedValue::externalInPlace(value, value->getIdentifier());
            };
        }

        template <bool is_pointer = std::is_pointer<ReturnType>::value>
        typename std::enable_if<is_pointer == false>::type wrap(const DirectFunction &function)
        {
            this->wrappedFunction = [function](const WrappedArgs &args)
            {
                auto value = unpack_caller(function, args);
                return WrappedValue::wrapInPlace(value);
            };
        }

    public:
        BindInfoFunction(std::string_view functionName,
                         FunctionType function,
                         const std::initializer_list<std::string> &argNames = {})
            : BindInfo(functionName, FUNCTION),
              BindInfoFunctionWrapped(argNames, {std::string(typeid(Args).name())...}, typeid(ReturnType).name()),
              direct(function)
        {
            this->wrap(direct);
            address = (size_t)function;
        }

        BindInfoFunction(std::string_view functionName,
                         DirectFunction &function,
                         const std::initializer_list<std::string> &argNames = {})
            : BindInfo(functionName, FUNCTION),
              BindInfoFunctionWrapped(argNames, {std::string(typeid(Args).name())...}, typeid(ReturnType).name()),
              direct(std::move(function))
        {
            this->wrap(direct);
            FunctionType **fnPointer = direct.template target<FunctionType *>();
            address = (size_t)*fnPointer;
        }

        virtual ~BindInfoFunction() {}

        bool compare(FunctionType function) { return address == (size_t)function; }
        bool compare(const DirectFunction &function)
        {
            FunctionType **fnPointer = function.template target<FunctionType *>();
            size_t cmpaddr = (size_t)*fnPointer;
            return address == cmpaddr;
        }

        ReturnType call(Args &&...args) const
        {
            if (this->type == FUNCTION && this->direct)
                return this->direct(std::forward<Args>(args)...); // call anyway
            throw std::invalid_argument("Unable to call function - it's not available");
        }

        ReturnType callWithArgs(const WrappedArgs &args) const
        {
            if (this->type == FUNCTION && this->direct)
                return unpack_caller(this->direct, args);
            throw std::invalid_argument("Unable to call function - it's not available");
        }
    }; //# class BindInfoFunction
    //#-----------------------------------------------------------------------------------

    /**
     * @brief
     *
     */
    class BindInfoMethodWrapped : public BindInfoFunctionWrappedBase
    {
    public:
        using base_type = BindInfoFunctionWrappedBase;
        using WrappedMethod = std::function<WrappedValue(void *, const WrappedArgs &)>;

    protected:
        WrappedMethod wrappedMethod;

    public:
        BindInfoMethodWrapped(const std::vector<std::string> &names,
                              const std::vector<std::string> &types,
                              std::string_view _returnType) : base_type(names, types, _returnType), wrappedMethod() {}
        virtual ~BindInfoMethodWrapped() {}

        template <typename UserClass>
        WrappedValue callWrapped(UserClass *pObject, const WrappedArgs &args) const
        {
            if (wrappedMethod)
                return wrappedMethod(static_cast<void *>(pObject), args);
            return WrappedValue(); // empty
        }

    }; //# BindInfoMethodWrapped
    //#-----------------------------------------------------------------------------------

    /**
     * @brief
     *
     * @tparam UserClass
     * @tparam ReturnType
     * @tparam Args
     */
    template <typename UserClass, typename ReturnType, typename... Args>
    class BindInfoMethod : public BindInfo, public virtual BindInfoMethodWrapped
    {
        using DirectMethod = ReturnType (UserClass::*)(Args...);

    private:
        DirectMethod direct;

        template <bool is_pointer = std::is_pointer<ReturnType>::value>
        typename std::enable_if<is_pointer == true>::type wrap(DirectMethod methodMember)
        {
            this->wrappedMethod = [methodMember](void *pObject, const WrappedArgs &args)
            {
                auto value = unpack_caller(static_cast<UserClass *>(pObject), methodMember, args);
                return WrappedValue::externalInPlace(value, value->getIdentifier());
            };
        }

        template <bool is_pointer = std::is_pointer<ReturnType>::value>
        typename std::enable_if<is_pointer == false>::type wrap(DirectMethod methodMember)
        {
            this->wrappedMethod = [methodMember](void *pObject, const WrappedArgs &args)
            {
                auto value = unpack_caller(static_cast<UserClass *>(pObject), methodMember, args);
                return WrappedValue::wrapInPlace(value);
            };
        }

    public:
        BindInfoMethod(std::string_view methodName,
                       DirectMethod methodMember,
                       const std::initializer_list<std::string> &argNames = {})
            : BindInfo(methodName, METHOD),
              BindInfoMethodWrapped(argNames, {std::string(typeid(Args).name())...}, typeid(ReturnType).name()),
              direct(methodMember)
        {
            this->wrap(methodMember);
        }

        virtual ~BindInfoMethod() {}

        bool compare(DirectMethod methodMember) { return direct == methodMember; }

        ReturnType call(UserClass *pObject, Args &&...args) const
        {
            if (this->type == METHOD && this->direct)
            {
                return (pObject->*(this->direct))(std::forward<Args>(args)...); // call anyway
            }
            throw std::invalid_argument("Unable to call member method - it's a property");
        }

        ReturnType callWithArgs(UserClass *pObject, const WrappedArgs &args) const
        {
            if (this->type == METHOD && this->direct)
                return unpack_caller(pObject, this->direct, args);
            throw std::invalid_argument("Unable to call member method - it's a property");
        }
    }; //# class BindInfoMethod
    //#-----------------------------------------------------------------------------------

    /**
     * @brief
     *
     * @tparam ReturnType
     * @tparam Args
     */
    template <typename ReturnType, typename... Args>
    class BindInfoMethod<void, ReturnType, Args...> : public BindInfo, public virtual BindInfoMethodWrapped
    {
        using DirectMethod = ReturnType (*)(Args...);

    private:
        DirectMethod direct;

        template <bool is_pointer = std::is_pointer<ReturnType>::value>
        typename std::enable_if<is_pointer == true>::type wrap(DirectMethod function)
        {
            this->wrappedMethod = [function](void *pObject, const WrappedArgs &args)
            {
                auto value = unpack_caller(function, args);
                return WrappedValue::externalInPlace(value, value->getIdentifier());
            };
        }

        template <bool is_pointer = std::is_pointer<ReturnType>::value>
        typename std::enable_if<is_pointer == false>::type wrap(DirectMethod function)
        {
            this->wrappedMethod = [function](void *pObject, const WrappedArgs &args)
            {
                auto value = unpack_caller(function, args);
                return WrappedValue::wrapInPlace(value);
            };
        }

    public:
        BindInfoMethod(std::string_view functionName,
                       DirectMethod function,
                       const std::initializer_list<std::string> &argNames = {})
            : BindInfo(functionName, METHOD),
              BindInfoMethodWrapped(argNames, {std::string(typeid(Args).name())...}, typeid(ReturnType).name()),
              direct(function)
        {
            this->wrap(function);
        }

        virtual ~BindInfoMethod() {}

        ReturnType call(void *pObject, Args &&...args) const
        {
            if (this->type == METHOD && this->direct)
            {
                return this->direct(std::forward<Args>(args)...); // call anyway
            }
            throw std::invalid_argument("Unable to call member method - it's a property");
        }

        ReturnType callWithArgs(void *pObject, const WrappedArgs &args) const
        {
            if (this->type == METHOD && this->direct)
                return unpack_caller(this->direct, args);
            throw std::invalid_argument("Unable to call member method - it's a property");
        }
    }; //# class BindInfoMethod<void>
    //#-----------------------------------------------------------------------------------

    /**
     * @brief
     *
     */
    class BindInfoPropertyWrapped
    {
    public:
        using GetterWrappedFunction = std::function<WrappedValue(void *)>;
        using SetterWrappedFunction = std::function<void(void *, const WrappedArgs &)>;

    protected:
        GetterWrappedFunction getterWrapped;
        SetterWrappedFunction setterWrapped;
        std::string valueType;

    public:
        BindInfoPropertyWrapped() {}
        virtual ~BindInfoPropertyWrapped() {}

        template <typename UserClass>
        WrappedValue getWrapped(UserClass *pObject) const
        {
            if (getterWrapped)
                return getterWrapped(static_cast<void *>(pObject));
            else
                return WrappedValue(); // empty
        }

        template <typename UserClass>
        void setWrapped(UserClass *pObject, const WrappedArgs &args) const
        {
            if (setterWrapped)
                setterWrapped(static_cast<void *>(pObject), args);
        }

        const std::string &getValueType(void) const noexcept { return valueType; }
    }; //# BindInfoPropertyWrapped
    //#---------------------------------------------------------------------------------------

    template <typename UserClass, typename ReturnType>
    class BindInfoProperty : public BindInfo, public virtual BindInfoPropertyWrapped
    {
        using GetterMethod = ReturnType (UserClass::*)() const;
        using SetterMethod = void (UserClass::*)(ReturnType);

    private:
        GetterMethod getter;
        SetterMethod setter;

    protected:
        template <bool is_pointer = std::is_pointer<ReturnType>::value>
        typename std::enable_if<is_pointer == true>::type wrap(GetterMethod _getter, SetterMethod _setter = nullptr)
        {
            this->getterWrapped = [_getter](void *pObject)
            {
                auto value = unpack_caller(static_cast<UserClass *>(pObject), _getter, WrappedArgs(0));
                return WrappedValue::externalInPlace(value, value->getIdentifier());
            };
            if (_setter != nullptr)
            {
                this->setterWrapped = [_setter](void *pObject, const WrappedArgs &args)
                {
                    unpack_caller(static_cast<UserClass *>(pObject), _setter, args);
                };
            }
        }

        template <bool is_pointer = std::is_pointer<ReturnType>::value>
        typename std::enable_if<is_pointer == false>::type wrap(GetterMethod _getter, SetterMethod _setter = nullptr)
        {
            this->getterWrapped = [_getter](void *pObject)
            {
                auto value = unpack_caller(static_cast<UserClass *>(pObject), _getter, WrappedArgs(0));
                return WrappedValue::wrapInPlace(value);
            };
            this->setterWrapped = [_setter](void *pObject, const WrappedArgs &args)
            {
                unpack_caller(static_cast<UserClass *>(pObject), _setter, args);
            };
        }

    public:
        BindInfoProperty(std::string_view propertyName, GetterMethod _getter, SetterMethod _setter) : BindInfo(propertyName, PROPERTY), BindInfoPropertyWrapped(), getter(_getter), setter(_setter)
        {
            this->valueType.assign(typeid(ReturnType).name());
            this->wrap(_getter, _setter);
        }

        BindInfoProperty(std::string_view propertyName, GetterMethod _getter) : BindInfo(propertyName, PROPERTY), BindInfoPropertyWrapped(), getter(_getter), setter(nullptr)
        {
            this->valueType.assign(typeid(ReturnType).name());
            this->wrap(_getter, nullptr);
        }

        ~BindInfoProperty() {}

        constexpr bool isReadOnly(void) const { return setter == nullptr; }

        ReturnType get(UserClass *pObject) const
        {
            if (this->type == PROPERTY && this->getter)
                return ((pObject)->*(this->getter))();
            throw std::invalid_argument("Unable to use getter on a property");
        }

        template <bool is_void = std::is_void<ReturnType>::value, class InputType>
        typename std::enable_if<is_void == true>::type set(UserClass *pObject, const InputType &value) const {}

        template <bool is_void = std::is_void<ReturnType>::value, class InputType>
        typename std::enable_if<is_void == false>::type set(UserClass *pObject, const InputType &value) const
        {
            if (this->type == PROPERTY && this->setter)
                ((pObject)->*(this->setter))(value);
            else
                throw std::invalid_argument("Unable to use setter on a property");
        }
    }; //# class BindInfoProperty
    //#-----------------------------------------------------------------------------------

    class BindInfoVariableWrapped : public virtual BindInfoPropertyWrapped
    {
    public:
        BindInfoVariableWrapped() : BindInfoPropertyWrapped() {}
        virtual ~BindInfoVariableWrapped() {}
    }; //# class BindInfoVariableWrapped
    //#-----------------------------------------------------------------------------------

    template <typename UserClass, typename VarType>
    class BindInfoVariable : public BindInfo, public virtual BindInfoVariableWrapped
    {
        using DecayedVarType = typename std::decay<VarType>::type;
        using DecayedVariable = DecayedVarType UserClass::*;

    private:
        DecayedVariable var;

    protected:
        template <bool is_pointer = std::is_pointer<VarType>::value>
        typename std::enable_if<is_pointer == true>::type wrap(BindInfoVariable *pThis)
        {
            this->getterWrapped = [pThis](void *pObject)
            {
                auto value = pThis->get(static_cast<UserClass *>(pObject));
                return WrappedValue::externalInPlace(value, value->getIdentifier());
            };
            this->setterWrapped = [pThis](void *pObject, const WrappedArgs &args)
            {
                pThis->set(static_cast<UserClass *>(pObject), *(args[0]));
            };
        }

        template <bool is_pointer = std::is_pointer<VarType>::value>
        typename std::enable_if<is_pointer == false>::type wrap(BindInfoVariable *pThis)
        {
            this->getterWrapped = [pThis](void *pObject)
            {
                auto value = pThis->get(static_cast<UserClass *>(pObject));
                return WrappedValue::wrapInPlace(value); // struct
            };
            this->setterWrapped = [pThis](void *pObject, const WrappedArgs &args)
            {
                pThis->set(static_cast<UserClass *>(pObject), *(args[0]));
            };
        }

    public:
        BindInfoVariable(std::string_view variableName, DecayedVariable _var) : BindInfo(variableName, VARIABLE), BindInfoVariableWrapped(), var(_var)
        {
            this->valueType.assign(typeid(VarType).name());
            this->wrap(this);
        }

        ~BindInfoVariable() {}

        VarType get(UserClass *pObject) const
        {
            if (this->type == VARIABLE && this->var)
                return pObject->*(this->var);
            throw std::invalid_argument("Unable to get value of a variable");
        }

        template <bool is_void = std::is_void<VarType>::value>
        typename std::enable_if<is_void == true>::type set(UserClass *pObject, const DecayedVarType &value) const {}

        template <bool is_void = std::is_void<VarType>::value>
        typename std::enable_if<is_void == false>::type set(UserClass *pObject, const DecayedVarType &value) const
        {
            if (this->type == VARIABLE && this->var)
                pObject->*(this->var) = value;
            else
                throw std::invalid_argument("Unable to set value on a variable");
        }
    }; //# class BindInfoVariable
    //#-----------------------------------------------------------------------------------

    /**
     * @brief Helper structure for holding information about action to perform on an object.
     * Holds all required information along with possible arguments, property or function
     * target name, object identification number (this is a global handle, serializable).
     */
    struct WrappedAction
    {
        enum Type
        {
            UNSET,
            CALL,
            SET,
            GET
        } type;

    public:
        static std::string_view enum_name(Type type) { return magic_enum::enum_name(type); }
        static Type enum_value(std::string_view name)
        {
            auto type = magic_enum::enum_cast<Type>(name);
            return (type.has_value() ? type.value() : Type::UNSET);
        }

        BindInfo::Type targetType;
        std::string targetName;
        int64_t objectId;
        std::string objectTypeName;
        WrappedArgs args;
        WrappedValue result;
        std::string nonce;

        WrappedAction() : type(UNSET), targetType(BindInfo::METHOD), targetName(), objectId(0), objectTypeName(), args(), result() {}

        WrappedAction(WrappedAction &&in) noexcept : type(std::exchange(in.type, UNSET)),
                                                     targetType(std::exchange(in.targetType, BindInfo::METHOD)),
                                                     targetName(std::move(in.targetName)),
                                                     objectId(std::exchange(in.objectId, 0)),
                                                     objectTypeName(std::move(in.objectTypeName)),
                                                     args(std::move(in.args)),
                                                     result(std::move(in.result)),
                                                     nonce(std::move(in.nonce))
        {
        }

        ~WrappedAction()
        {
            type = UNSET;
            objectId = 0;
            targetName.clear();
            objectTypeName.clear();
            nonce.clear();
            util::reset_arguments(args);
        }

        constexpr bool isValid(void) const { return type != UNSET && objectId > 0 && !targetName.empty(); }
    }; //# struct WrappedAction
    //#-----------------------------------------------------------------------------------

    class MetadataBindings
    {
    public:
        using self_type = MetadataBindings;

    protected:
        std::string typeName;
        BindInfo::Bindings bindings;

    public:
        MetadataBindings(std::string_view _typeName) : typeName(_typeName), bindings() {}

        const std::string &getTypeName(void) const noexcept { return typeName; }

        const BindInfo::Bindings &getBindings(void) const noexcept { return bindings; }

        template <typename ReturnType, typename... Args>
        self_type &function(std::string_view functionName, ReturnType (*function)(Args...), const std::initializer_list<std::string> &argNames = {})
        {
            bindings.push_back(new BindInfoFunction<ReturnType, Args...>(functionName, function, argNames));
            return *this;
        }

        template <typename UserClass, typename ReturnType, typename... Args>
        self_type &method(std::string_view methodName, ReturnType (UserClass::*methodMember)(Args...), const std::initializer_list<std::string> &argNames = {})
        {
            bindings.push_back(new BindInfoMethod<UserClass, ReturnType, Args...>(methodName, methodMember, argNames));
            return *this;
        }

        template <typename UserClass, typename ReturnType>
        self_type &property(std::string_view propertyName, ReturnType (UserClass::*getter)() const, void (UserClass::*setter)(ReturnType))
        {
            bindings.push_back(new BindInfoProperty<UserClass, ReturnType>(propertyName, getter, setter));
            return *this;
        }

        template <typename UserClass, typename ReturnType>
        self_type &property(std::string_view propertyName, ReturnType (UserClass::*getter)() const)
        {
            bindings.push_back(new BindInfoProperty<UserClass, ReturnType>(propertyName, getter));
            return *this;
        }

        template <typename UserClass, typename VarType>
        self_type &variable(std::string_view variableName, VarType UserClass::*var)
        {
            bindings.push_back(new BindInfoVariable<UserClass, VarType>(variableName, var));
            return *this;
        }

        bool has(BindInfo::Type type) const
        {
            auto found = std::find_if(bindings.begin(), bindings.end(), [&type](BindInfo *const &binding)
                                      { return binding->getType() == type; });
            return found != bindings.end();
        }

        bool has(BindInfo::Type type, std::string_view name) const
        {
            return (get(type, name) != nullptr);
        }

        const BindInfo *get(BindInfo::Type type, std::string_view name) const
        {
            auto found = std::find_if(bindings.begin(), bindings.end(), [&type, &name](BindInfo *const &binding)
                                      { return binding->getType() == type && binding->getName().compare(name) == 0; });
            return found != bindings.end() ? *found : nullptr;
        }

        const BindInfo *operator[](std::string_view name) const
        {
            auto found = std::find_if(bindings.begin(), bindings.end(), [&name](BindInfo *const &binding)
                                      { return binding->getName().compare(name) == 0; });
            return found != bindings.end() ? *found : nullptr;
        }
    }; //# class MetadataBindings
    //#-----------------------------------------------------------------------------------

    template <typename UserClass>
    class TypeMetadataBindings : public MetadataBindings
    {
    public:
        TypeMetadataBindings() : MetadataBindings(typeid(UserClass).name()) {}
    }; //# class TypeMetadataBindings
    //#-----------------------------------------------------------------------------------

    class BindingsInvokerBase
    {
    protected:
        const MetadataBindings *pMetadata;

    public:
        BindingsInvokerBase(const MetadataBindings &metadata) : pMetadata(&metadata) {}
        BindingsInvokerBase(const MetadataBindings *metadata) : pMetadata(metadata) {}

        virtual ~BindingsInvokerBase() {}

        virtual WrappedValue callWrapped(std::string_view name, const WrappedArgs &args) = 0;

        virtual WrappedValue getWrapped(std::string_view name) = 0;

        virtual void setWrapped(std::string_view name, const WrappedArgs &args) = 0;

        virtual bool executeAction(WrappedAction &action) = 0;
    }; //# class BindingsInvokerBase
    //#-----------------------------------------------------------------------------------

    struct BindingHelper
    {
        BindingHelper() = delete;
        BindingHelper(const BindingHelper &other) = delete;
        //>-------------------------------------------------------------------------------

        template <typename UserClass, typename ReturnType, typename... Args>
        static bool compare(const BindInfo *pBinding, ReturnType (UserClass::*methodMember)(Args...))
        {
            if (pBinding->isMethod())
                return static_cast<const BindInfoMethod<UserClass, ReturnType, Args...> *>(pBinding)->compare(methodMember);
            return false;
        }
        //>-------------------------------------------------------------------------------

        template <typename ReturnType, typename... Args>
        static bool compare(const BindInfo *pBinding, ReturnType (*function)(Args...))
        {
            if (pBinding->isFunction())
                return static_cast<const BindInfoFunction<ReturnType, Args...> *>(pBinding)->compare(function);
            return false;
        }
        //>-------------------------------------------------------------------------------

        template <typename UserClass, typename ReturnType, typename... Args>
        static ReturnType call(UserClass *pObject, const BindInfo *pBinding, Args &&...args)
        {
            if (pBinding->isFunction())
                return BindingHelper::callFunction<ReturnType, Args...>(pBinding, std::forward<Args>(args)...);
            if (pBinding->isMethod())
                return BindingHelper::callMethod<UserClass, ReturnType, Args...>(pObject, pBinding, std::forward<Args>(args)...);
            throw std::invalid_argument("Unable to call binding - not a function / method");
        }
        //>-------------------------------------------------------------------------------

        template <typename ReturnType, typename... Args>
        static ReturnType callFunction(const BindInfo *pBinding, Args &&...args)
        {
            if (pBinding->isFunction())
            {
                auto pBindingCast = static_cast<const BindInfoFunction<ReturnType, Args...> *>(pBinding);
                return pBindingCast->call(std::forward<Args>(args)...); // direct
            }
            throw std::invalid_argument("Unable to call binding - not a function");
        }
        //>-------------------------------------------------------------------------------

        template <typename UserClass, typename ReturnType, typename... Args>
        static ReturnType callMethod(UserClass *pObject, const BindInfo *pBinding, Args &&...args)
        {
            if (pBinding->isMethod())
            {
                auto pBindingCast = static_cast<const BindInfoMethod<UserClass, ReturnType, Args...> *>(pBinding);
                return pBindingCast->call(pObject, std::forward<Args>(args)...); // direct
            }
            throw std::invalid_argument("Unable to call binding - not a method");
        }
        //>-------------------------------------------------------------------------------

        template <typename UserClass, typename ReturnType>
        static ReturnType get(UserClass *pObject, const BindInfo *pBinding)
        {
            if (pBinding->isProperty())
            {
                auto pBindingCast = static_cast<const BindInfoProperty<UserClass, ReturnType> *>(pBinding); // PROPERTY
                return pBindingCast->get(pObject);                                                          // direct
            }
            if (pBinding->isVariable())
            {
                auto pBindingCast = static_cast<const BindInfoVariable<UserClass, ReturnType> *>(pBinding); // VARIABLE
                return pBindingCast->get(pObject);                                                          // direct
            }
            throw std::invalid_argument("Unable to call get() - not a variable / property");
        }
        //>-------------------------------------------------------------------------------

        template <typename UserClass, typename InputType>
        static void set(UserClass *pObject, const BindInfo *pBinding, const InputType &value)
        {
            if (pBinding->isProperty())
            {
                auto pBindingCast = static_cast<const BindInfoProperty<UserClass, InputType> *>(pBinding); // PROPERTY
                pBindingCast->set(pObject, value);                                                         // direct
                return;
            }
            if (pBinding->isVariable())
            {
                auto pBindingCast = static_cast<const BindInfoVariable<UserClass, InputType> *>(pBinding); // VARIABLE
                pBindingCast->set(pObject, value);                                                         // direct
                return;
            }
            throw std::invalid_argument("Unable to call set() - not a variable / property");
        }
        //>-------------------------------------------------------------------------------

        template <typename UserClass>
        static WrappedValue callWrapped(UserClass *pObject, const BindInfo *pBinding, const WrappedArgs &args)
        {
            if (pBinding->isMethod() && pObject != nullptr)
            {
                auto pBindingCast = dynamic_cast<const BindInfoMethodWrapped *>(pBinding);
                return pBindingCast->callWrapped(pObject, args); // pointer - allocated
            }
            if (pBinding->isFunction())
            {
                auto pBindingCast = dynamic_cast<const BindInfoFunctionWrapped *>(pBinding);
                return pBindingCast->callWrapped(args);
            }
            throw std::invalid_argument("Unable to call binding - not a function / method");
        }
        //>-------------------------------------------------------------------------------

        template <typename UserClass>
        static WrappedValue getWrapped(UserClass *pObject, const BindInfo *pBinding)
        {
            if (pBinding->isProperty() && pObject != nullptr)
            {
                const BindInfoPropertyWrapped *pBindingCast = dynamic_cast<const BindInfoPropertyWrapped *>(pBinding); // PROPERTY
                return pBindingCast->getWrapped(pObject);                                                              // pointer - allocated
            }
            if (pBinding->isVariable() && pObject != nullptr)
            {
                const BindInfoVariableWrapped *pBindingCast = dynamic_cast<const BindInfoVariableWrapped *>(pBinding); // VARIABLE
                return pBindingCast->getWrapped(pObject);                                                              // pointer - allocated
            }
            throw std::invalid_argument("Unable to call get() - not a variable / property");
        }
        //>-------------------------------------------------------------------------------

        template <typename UserClass>
        static void setWrapped(UserClass *pObject, const BindInfo *pBinding, const WrappedArgs &args)
        {
            if (pBinding->isProperty() && pObject != nullptr)
            {
                const BindInfoPropertyWrapped *pBindingCast = dynamic_cast<const BindInfoPropertyWrapped *>(pBinding);
                pBindingCast->setWrapped(pObject, args);
                return;
            }
            if (pBinding->isVariable() && pObject != nullptr)
            {
                const BindInfoVariableWrapped *pBindingCast = dynamic_cast<const BindInfoVariableWrapped *>(pBinding);
                pBindingCast->setWrapped(pObject, args); // wrapped args - no result
                return;
            }
            throw std::invalid_argument("Unable to call set() - not a variable / property");
        }
        //>-------------------------------------------------------------------------------
    }; //# struct BindingHelper

    template <typename UserClass>
    class BindingsInvoker : public BindingsInvokerBase
    {
        UserClass *pObject;

    public:
        BindingsInvoker(UserClass *object, const MetadataBindings &metadata) : BindingsInvokerBase(metadata), pObject(object) {}
        BindingsInvoker(UserClass *object, const MetadataBindings *metadata) : BindingsInvokerBase(metadata), pObject(object) {}
        virtual ~BindingsInvoker()
        {
            pMetadata = nullptr;
            pObject = nullptr;
        }

        void setObject(UserClass *object) { pObject = object; }

    public:
        template <typename ReturnType, typename... Args>
        ReturnType call(std::string_view name, Args &&...args)
        {
            auto pBinding = pMetadata->get(BindInfo::METHOD, name);
            if (pBinding != nullptr)
                return BindingHelper::callMethod<UserClass, ReturnType, Args...>(pObject, pBinding, std::forward<Args>(args)...); // direct
            pBinding = pMetadata->get(BindInfo::FUNCTION, name);
            if (pBinding != nullptr)
                return BindingHelper::callFunction<ReturnType, Args...>(pBinding, std::forward<Args>(args)...); // direct
            throw std::invalid_argument("Unable to find bound method / function");
        }

        template <typename ReturnType>
        ReturnType get(std::string_view name)
        {
            auto pBinding = pMetadata->get(BindInfo::PROPERTY, name);
            if (!pBinding)
                pBinding = pMetadata->get(BindInfo::VARIABLE, name);
            if (pBinding != nullptr)
                return BindingHelper::get<UserClass, ReturnType>(pObject, pBinding);
            throw std::invalid_argument("Unable to call get() - not a variable / property");
        }

        // template <typename InputType, bool is_pointer = std::is_pointer<InputType>::value, bool is_reference = std::is_reference<InputType>::value>
        // typename std::enable_if<is_pointer == true || is_reference == false>::type set(std::string_view &name, InputType value)
        template <typename InputType>
        void set(std::string_view name, const InputType &value)
        {
            auto pBinding = pMetadata->get(BindInfo::PROPERTY, name);
            if (!pBinding)
                pBinding = pMetadata->get(BindInfo::VARIABLE, name);
            if (pBinding != nullptr)
                return BindingHelper::set<UserClass, InputType>(pObject, pBinding, value);
            throw std::invalid_argument("Unable to call set() - not a variable / property");
        }

        // template <typename InputType, bool is_pointer = std::is_pointer<InputType>::value, bool is_reference = std::is_reference<InputType>::value>
        // typename std::enable_if<is_pointer == false && is_reference == true>::type set(std::string_view name, const InputType &value)
        //{
        //     auto pBinding = pMetadata->get(BindInfo::PROPERTY, name);
        //     if (pBinding == nullptr)
        //         pBinding = pMetadata->get(BindInfo::VARIABLE, name);
        //     if (pBinding != nullptr)
        //         return util::bound_set(pBinding, pObject, value);
        //     throw std::invalid_argument("Unable to call set() - not a variable / property");
        // }

        WrappedValue callWrapped(std::string_view name, const WrappedArgs &args) override
        {
            auto pBinding = pMetadata->get(BindInfo::METHOD, name);
            if (!pBinding)
                pBinding = pMetadata->get(BindInfo::FUNCTION, name);
            if (pBinding != nullptr)
                return BindingHelper::callWrapped<UserClass>(pObject, pBinding, args);
            throw std::invalid_argument("Unable to find bound method / function");
        }

        WrappedValue getWrapped(std::string_view name) override
        {
            auto pBinding = pMetadata->get(BindInfo::PROPERTY, name);
            if (!pBinding)
                pBinding = pMetadata->get(BindInfo::VARIABLE, name);
            if (pBinding != nullptr)
                return BindingHelper::getWrapped<UserClass>(pObject, pBinding);
            throw std::invalid_argument("Unable to call get() - not a variable / property");
        }

        void setWrapped(std::string_view name, const WrappedArgs &args) override
        {
            auto pBinding = pMetadata->get(BindInfo::PROPERTY, name);
            if (!pBinding)
                pBinding = pMetadata->get(BindInfo::VARIABLE, name);
            if (pBinding != nullptr)
                return BindingHelper::setWrapped<UserClass>(pObject, pBinding, args);
            throw std::invalid_argument("Unable to call set() - not a variable / property");
        }

        bool executeAction(WrappedAction &action) override
        {
            if (!action.isValid())
            {
                action.result.reset();
                return false; // skip
            }
            if (!this->pObject)
                return false;
            // need to assume that this object (invoker is already prepared and object pointer
            // was externally updated before calling action.
            // Wrapped action contains unique object identification, it allows to retrieve pointer
            // to any type of registered object (it has to be kept in map to be found).
            //
            // action contains wrapped value for result - before executing it should be cleared
            action.result.reset();
            if (action.type == WrappedAction::CALL)
                action.result = this->callWrapped(action.targetName, action.args);
            else if (action.type == WrappedAction::GET)
                action.result = this->getWrapped(action.targetName);
            else if (action.type == WrappedAction::SET)
                this->setWrapped(action.targetName, action.args);
            return true;
        }
    }; //# class BindingsInvoker
    //#-----------------------------------------------------------------------------------
} //> namespace util

#endif //> FG_INC_UTIL_BINDINGS