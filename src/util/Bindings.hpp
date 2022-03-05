#pragma once
#ifndef FG_INC_UTIL_BINDINGS
#define FG_INC_UTIL_BINDINGS

#include <magic_enum.hpp>

#include <typeinfo>
#include <functional>
#include <string>
#include <util/Vector.hpp>
#include <util/UnpackCaller.hpp>

namespace util
{
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

        static std::string_view enum_name(Type type) { return magic_enum::enum_name(type); }
        static Type enum_value(std::string_view name)
        {
            auto type = magic_enum::enum_cast<Type>(name);
            return (type.has_value() ? type.value() : Type::INVALID);
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
        const std::string &getTypeName(void) const noexcept { return tname; }

        constexpr bool isExternal(void) const noexcept { return type == EXTERNAL; }

        constexpr bool isString(void) const noexcept { return type == STRING; }

        constexpr bool isValid(void) const noexcept { return type != INVALID; }

        constexpr bool isEmpty(void) const noexcept { return type == INVALID; }

        constexpr Type getType(void) const noexcept { return type; }

        constexpr uint64_t getExternalHandle(void) const
        {
            if (type == EXTERNAL)
                return packed.external.handle;
            return 0;
        }

    public:
        template <typename InputType>
        void set(const InputType &value)
        {
            tname = typeid(InputType).name();
            type = WrappedValue::determineInternalType(tname); // overwrite
            void *_data = &packed;
            InputType *holder = static_cast<InputType *>(_data);
            *holder = value; // directly paste it in
            printf("WrappedValue(%p)::set() - tname='%s' type='%s' value='%d'\n", this, tname.c_str(), enum_name(type).data(), packed.i_val);
        }

        template <>
        void set(const std::string &value)
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
            printf("WrappedValue(%p)::WrappedValue(%s)\n", this, _tname);
            std::fill_n((uint8_t *)(&packed), sizeof(packed), static_cast<uint8_t>(0));
        }

        WrappedValue(const char *_tname, void *_external, uint64_t _handle) : tname(_tname), type(EXTERNAL)
        {
            printf("WrappedValue(%p)::WrappedValue(%s, %p, %llu)\n", this, _tname, _external, _handle);
            std::fill_n((uint8_t *)(&packed), sizeof(packed), static_cast<uint8_t>(0));
            packed.external.pointer = _external;
            packed.external.handle = _handle;
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
            printf("WrappedValue(%p)::WrappedValue(%p/copy-assign) - tname='%s' type='%s' value='%d'\n", this, &input, tname.c_str(), enum_name(type).data(), packed.i_val);
            return *this;
        }

        WrappedValue &operator=(WrappedValue &&input)
        {
            if (this != &input)
            {
                type = input.type;
                tname = std::move(input.tname);
                strvalue = std::move(input.strvalue);
                std::copy_n(&input.packed, 1, &packed); // copy one structure
                std::fill_n((uint8_t *)(&input.packed), sizeof(input.packed), static_cast<uint8_t>(0));
                input.type = WrappedValue::INVALID;
                printf("WrappedValue(%p)::WrappedValue(%p/move-assign) - tname='%s' type='%s' value='%d'\n", this, &input, tname.c_str(), enum_name(type).data(), packed.i_val);
            }
            return *this;
        }

        WrappedValue(const WrappedValue &input)
        {
            std::copy_n(&input.packed, 1, &packed); // copy one structure
            tname = input.tname;
            type = input.type;
            strvalue = input.strvalue;
            printf("WrappedValue(%p)::WrappedValue(%p/copy) - tname='%s' type='%s' value='%d'\n", this, &input, tname.c_str(), enum_name(type).data(), packed.i_val);
        }

        WrappedValue(WrappedValue &&input) : type(input.type),
                                             tname(std::move(input.tname)),
                                             strvalue(std::move(input.strvalue))
        {
            std::copy_n(&input.packed, 1, &packed); // copy one structure
            std::fill_n((uint8_t *)(&input.packed), sizeof(input.packed), static_cast<uint8_t>(0));
            input.type = WrappedValue::INVALID;
            printf("WrappedValue(%p)::WrappedValue(%p/move) - tname='%s' type='%s' value='%d'\n", this, &input, tname.c_str(), enum_name(type).data(), packed.i_val);
        }

        void reset(void)
        {
            printf("WrappedValue(%p)::~WrappedValue() - tname='%s' type='%s'\n", this, tname.c_str(), enum_name(type).data());
            std::fill_n((uint8_t *)(&packed), sizeof(packed), static_cast<uint8_t>(0));
            type = INVALID;
            tname.clear();
            strvalue.clear();
        }

        template <typename InputType>
        static WrappedValue *wrap(const InputType &value)
        {
            WrappedValue *pWrapped = new WrappedValue(typeid(value).name());
            pWrapped->set(value);
            return pWrapped;
        }

        template <typename InputType>
        static WrappedValue wrapStruct(const InputType &value)
        {
            WrappedValue wrapped = WrappedValue(typeid(value).name());
            wrapped.set(value);
            return wrapped;
        }

        template <>
        static WrappedValue *wrap(const std::string &value)
        {
            return new WrappedValue(value, typeid(value).name());
        }

        template <>
        static WrappedValue wrapStruct(const std::string &value)
        {
            return WrappedValue(value, typeid(value).name());
        }

        static WrappedValue *wrap(const char *value)
        {
            return new WrappedValue(std::string(value), typeid(value).name());
        }

        static WrappedValue wrapStruct(const char *value)
        {
            return WrappedValue(std::string(value), typeid(value).name());
        }

        template <typename T>
        static WrappedValue *external(T **value, uint64_t id)
        {
            return new WrappedValue(typeid(T *).name(), !value ? nullptr : *value, id);
        }

        template <typename T>
        static WrappedValue externalCopy(T **value, uint64_t id)
        {
            return WrappedValue(typeid(T *).name(), !value ? nullptr : *value, id);
        }

        //--------------------------------------------------------------------------------

        // template <typename T>
        // static WrappedValue wrap_struct(const T &value)
        //{
        //     WrappedValue wrapped(sizeof(value), typeid(value).name());
        //     wrapped.set(value);
        //     return wrapped;
        // }
        // template <>
        // static WrappedValue wrap_struct(const std::string &value)
        //{
        //     return WrappedValue(value, typeid(value).name());
        // }
        // template <unsigned int N>
        // static WrappedValue wrap_struct(const char (&value)[N])
        //{
        //     return WrappedValue(std::string(value), typeid(value).name());
        // }

    public:
        template <typename T>
        operator T()
        {
            return get<T>();
        }

        operator std::string &()
        {
            return strvalue;
        }

        operator const char *()
        {
            if (type == STRING)
                return strvalue.c_str();
            return nullptr; // can't cast packed data
        }

        template <typename T>
        operator T *()
        {
            if (type == EXTERNAL)
            {
                void *__data = &packed;
                auto _data = reinterpret_cast<External *>(__data);
                return util::convert<T>::convertToPointer(_data->pointer, _data->handle);
            }
            return nullptr; // nothing else to return
        }

        using Args = std::vector<WrappedValue *>;
    }; //# class WrappedValue
    //#-----------------------------------------------------------------------------------

    WrappedValue::Args &reset_arguments(WrappedValue::Args &args)
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
    //#-----------------------------------------------------------------------------------

    /**
     * @brief
     *
     */
    class BindInfo
    {
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
        using WrappedFunction = std::function<WrappedValue(const WrappedValue::Args &)>;

    protected:
        WrappedFunction wrappedFunction;

    public:
        BindInfoFunctionWrapped(const std::vector<std::string> &names,
                                const std::vector<std::string> &types,
                                std::string_view _returnType) : base_type(names, types, _returnType), wrappedFunction() {}
        virtual ~BindInfoFunctionWrapped() {}

        WrappedValue callWrapped(const WrappedValue::Args &args) const
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

        template <bool is_pointer = std::is_pointer<ReturnType>::value>
        typename std::enable_if<is_pointer == true>::type wrap(DirectFunction function)
        {
            this->wrappedFunction = [function](const WrappedValue::Args &args)
            {
                auto value = unpack_caller(function, args);
                return WrappedValue::externalCopy(&value, value->getHandleBase().getHandle());
            };
        }

        template <bool is_pointer = std::is_pointer<ReturnType>::value>
        typename std::enable_if<is_pointer == false>::type wrap(DirectFunction function)
        {
            this->wrappedFunction = [function](const WrappedValue::Args &args)
            {
                auto value = unpack_caller(function, args);
                return WrappedValue::wrapStruct(value);
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
        }

        BindInfoFunction(std::string_view functionName,
                         DirectFunction function,
                         const std::initializer_list<std::string> &argNames = {})
            : BindInfo(functionName, FUNCTION),
              BindInfoFunctionWrapped(argNames, {std::string(typeid(Args).name())...}, typeid(ReturnType).name()),
              direct(function)
        {
            this->wrap(direct);
        }

        virtual ~BindInfoFunction() {}

        ReturnType call(Args &&...args) const
        {
            if (this->type == FUNCTION && this->direct)
                return this->direct(std::forward<Args>(args)...); // call anyway
            throw std::invalid_argument("Unable to call function - it's not available");
        }

        ReturnType callWithArgs(const WrappedValue::Args &args) const
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
        using WrappedMethod = std::function<WrappedValue(void *, const WrappedValue::Args &)>;

    protected:
        WrappedMethod wrappedMethod;

    public:
        BindInfoMethodWrapped(const std::vector<std::string> &names,
                              const std::vector<std::string> &types,
                              std::string_view _returnType) : base_type(names, types, _returnType), wrappedMethod() {}
        virtual ~BindInfoMethodWrapped() {}

        template <class UserClass>
        WrappedValue callWrapped(UserClass *pObj, const WrappedValue::Args &args) const
        {
            if (wrappedMethod)
                return wrappedMethod(static_cast<void *>(pObj), args);
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
    template <class UserClass, typename ReturnType, typename... Args>
    class BindInfoMethod : public BindInfo, public virtual BindInfoMethodWrapped
    {
        using DirectMethod = ReturnType (UserClass::*)(Args...);

    private:
        DirectMethod direct;

        template <bool is_pointer = std::is_pointer<ReturnType>::value>
        typename std::enable_if<is_pointer == true>::type wrap(DirectMethod methodMember)
        {
            this->wrappedMethod = [methodMember](void *pObj, const WrappedValue::Args &args)
            {
                auto value = unpack_caller(static_cast<UserClass *>(pObj), methodMember, args);
                return WrappedValue::externalCopy(&value, value->getHandleBase().getHandle());
            };
        }

        template <bool is_pointer = std::is_pointer<ReturnType>::value>
        typename std::enable_if<is_pointer == false>::type wrap(DirectMethod methodMember)
        {
            this->wrappedMethod = [methodMember](void *pObj, const WrappedValue::Args &args)
            {
                auto value = unpack_caller(static_cast<UserClass *>(pObj), methodMember, args);
                return WrappedValue::wrapStruct(value);
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

        ReturnType call(UserClass *pObj, Args &&...args) const
        {
            if (this->type == METHOD && this->direct)
            {
                return (pObj->*(this->direct))(std::forward<Args>(args)...); // call anyway
            }
            throw std::invalid_argument("Unable to call member method - it's a property");
        }

        ReturnType callWithArgs(UserClass *pObj, const WrappedValue::Args &args) const
        {
            if (this->type == METHOD && this->direct)
                return unpack_caller(pObj, this->direct, args);
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
            this->wrappedMethod = [function](void *pObj, const WrappedValue::Args &args)
            {
                auto value = unpack_caller(function, args);
                return WrappedValue::externalCopy(&value, value->getHandleBase().getHandle());
            };
        }

        template <bool is_pointer = std::is_pointer<ReturnType>::value>
        typename std::enable_if<is_pointer == false>::type wrap(DirectMethod function)
        {
            this->wrappedMethod = [function](void *pObj, const WrappedValue::Args &args)
            {
                auto value = unpack_caller(function, args);
                return WrappedValue::wrapStruct(value);
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

        ReturnType call(void *pObj, Args &&...args) const
        {
            if (this->type == METHOD && this->direct)
            {
                return this->direct(std::forward<Args>(args)...); // call anyway
            }
            throw std::invalid_argument("Unable to call member method - it's a property");
        }

        ReturnType callWithArgs(void *pObj, const WrappedValue::Args &args) const
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
        using GetterWrappedFunction = std::function<WrappedValue(void *pObj)>;
        using SetterWrappedFunction = std::function<void(void *pObj, const WrappedValue::Args &)>;

    protected:
        GetterWrappedFunction getterWrapped;
        SetterWrappedFunction setterWrapped;
        std::string valueType;

    public:
        BindInfoPropertyWrapped() {}
        virtual ~BindInfoPropertyWrapped() {}

        template <class UserClass>
        WrappedValue getWrapped(UserClass *pObj) const
        {
            if (getterWrapped)
                return getterWrapped(static_cast<void *>(pObj));
            else
                return WrappedValue(); // empty
        }

        template <class UserClass>
        void setWrapped(UserClass *pObj, const WrappedValue::Args &args) const
        {
            if (setterWrapped)
                setterWrapped(static_cast<void *>(pObj), args);
        }

        const std::string &getValueType(void) const noexcept { return valueType; }
    }; //# BindInfoPropertyWrapped
    //#---------------------------------------------------------------------------------------

    template <class UserClass, typename ReturnType>
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
            this->getterWrapped = [_getter](void *pObj)
            {
                auto value = unpack_caller(static_cast<UserClass *>(pObj), _getter, WrappedValue::Args(0));
                return WrappedValue::externalCopy(&value, value->getHandleBase().getHandle());
            };
            if (_setter != nullptr)
            {
                this->setterWrapped = [_setter](void *pObj, const WrappedValue::Args &args)
                {
                    unpack_caller(static_cast<UserClass *>(pObj), _setter, args);
                };
            }
        }

        template <bool is_pointer = std::is_pointer<ReturnType>::value>
        typename std::enable_if<is_pointer == false>::type wrap(GetterMethod _getter, SetterMethod _setter = nullptr)
        {
            this->getterWrapped = [_getter](void *pObj)
            {
                auto value = unpack_caller(static_cast<UserClass *>(pObj), _getter, WrappedValue::Args(0));
                return WrappedValue::wrapStruct(value);
            };
            this->setterWrapped = [_setter](void *pObj, const WrappedValue::Args &args)
            {
                unpack_caller(static_cast<UserClass *>(pObj), _setter, args);
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

        ReturnType get(UserClass *pObj) const
        {
            if (this->type == PROPERTY && this->getter)
                return ((pObj)->*(this->getter))();
            throw std::invalid_argument("Unable to use getter on a property");
        }

        template <bool is_void = std::is_void<ReturnType>::value, class InputType>
        typename std::enable_if<is_void == true>::type set(UserClass *pObj, const InputType &value) const {}

        template <bool is_void = std::is_void<ReturnType>::value, class InputType>
        typename std::enable_if<is_void == false>::type set(UserClass *pObj, const InputType &value) const
        {
            if (this->type == PROPERTY && this->setter)
                ((pObj)->*(this->setter))(value);
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

    template <class UserClass, typename VarType>
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
            this->getterWrapped = [pThis](void *pObj)
            {
                auto value = pThis->get(static_cast<UserClass *>(pObj));
                return WrappedValue::externalCopy(&value, value->getHandleBase().getHandle());
            };

            this->setterWrapped = [pThis](void *pObj, const WrappedValue::Args &args)
            {
                pThis->set(static_cast<UserClass *>(pObj), *(args[0]));
            };
        }

        template <bool is_pointer = std::is_pointer<VarType>::value>
        typename std::enable_if<is_pointer == false>::type wrap(BindInfoVariable *pThis)
        {
            this->getterWrapped = [pThis](void *pObj)
            {
                auto value = pThis->get(static_cast<UserClass *>(pObj));
                return WrappedValue::wrapStruct(value); // struct
            };
            this->setterWrapped = [pThis](void *pObj, const WrappedValue::Args &args)
            {
                pThis->set(static_cast<UserClass *>(pObj), *(args[0]));
            };
        }

    public:
        BindInfoVariable(std::string_view variableName, DecayedVariable _var) : BindInfo(variableName, VARIABLE), BindInfoVariableWrapped(), var(_var)
        {
            this->valueType.assign(typeid(VarType).name());
            this->wrap(this);
        }

        ~BindInfoVariable() {}

        VarType get(UserClass *pObj) const
        {
            if (this->type == VARIABLE && this->var)
                return pObj->*(this->var);
            throw std::invalid_argument("Unable to get value of a variable");
        }

        template <bool is_void = std::is_void<VarType>::value>
        typename std::enable_if<is_void == true>::type set(UserClass *pObj, const DecayedVarType &value) const {}

        template <bool is_void = std::is_void<VarType>::value>
        typename std::enable_if<is_void == false>::type set(UserClass *pObj, const DecayedVarType &value) const
        {
            if (this->type == VARIABLE && this->var)
                pObj->*(this->var) = value;
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
        WrappedValue::Args args;
        WrappedValue result;
        std::string nonce;

        WrappedAction() : type(UNSET), targetType(BindInfo::METHOD), targetName(), objectId(0), objectTypeName(), args(), result() {}

        WrappedAction(WrappedAction &&in) : type(std::exchange(in.type, UNSET)),
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

        template <class UserClass, typename ReturnType, typename... Args>
        self_type &function(std::string_view functionName, ReturnType (*function)(Args...), const std::initializer_list<std::string> &argNames = {})
        {
            bindings.push_back(new BindInfoFunction<ReturnType, Args...>(functionName, function, argNames));
            return *this;
        }

        template <class UserClass, typename ReturnType, typename... Args>
        self_type &method(std::string_view methodName, ReturnType (UserClass::*methodMember)(Args...), const std::initializer_list<std::string> &argNames = {})
        {
            bindings.push_back(new BindInfoMethod<UserClass, ReturnType, Args...>(methodName, methodMember, argNames));
            return *this;
        }

        template <class UserClass, typename ReturnType>
        self_type &property(std::string_view propertyName, ReturnType (UserClass::*getter)() const, void (UserClass::*setter)(ReturnType))
        {
            bindings.push_back(new BindInfoProperty<UserClass, ReturnType>(propertyName, getter, setter));
            return *this;
        }

        template <class UserClass, typename ReturnType>
        self_type &property(std::string_view propertyName, ReturnType (UserClass::*getter)() const)
        {
            bindings.push_back(new BindInfoProperty<UserClass, ReturnType>(propertyName, getter));
            return *this;
        }

        template <class UserClass, typename VarType>
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

    template <class UserClass>
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

        virtual WrappedValue callWrapped(std::string_view name, const WrappedValue::Args &args) = 0;

        virtual WrappedValue getWrapped(std::string_view name) = 0;

        virtual void setWrapped(std::string_view name, const WrappedValue::Args &args) = 0;

        virtual bool executeAction(WrappedAction &action) = 0;
    }; //# class BindingsInvokerBase
    //#-----------------------------------------------------------------------------------

    struct BindingHelper
    {
        BindingHelper() = delete;
        BindingHelper(const BindingHelper &other) = delete;
        //#-------------------------------------------------------------------------------

        template <typename ReturnType, typename UserClass, typename... Args>
        static ReturnType call(UserClass *pObject, const BindInfo *pBinding, Args &&...args)
        {
            if (pBinding->isFunction())
                return BindingHelper::callFunction<ReturnType, Args...>(pBinding, std::forward<Args>(args)...);
            if (pBinding->isMethod())
                return BindingHelper::callMethod<ReturnType, UserClass, Args...>(pObject, pBinding, std::forward<Args>(args)...);
            throw std::invalid_argument("Unable to call binding - not a function / method");
        }
        //#-------------------------------------------------------------------------------

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
        //#-------------------------------------------------------------------------------

        template <typename ReturnType, typename UserClass, typename... Args>
        static ReturnType callMethod(UserClass *pObject, const BindInfo *pBinding, Args &&...args)
        {
            if (pBinding->isMethod())
            {
                auto pBindingCast = static_cast<const BindInfoMethod<UserClass, ReturnType, Args...> *>(pBinding);
                return pBindingCast->call(pObject, std::forward<Args>(args)...); // direct
            }
            throw std::invalid_argument("Unable to call binding - not a method");
        }
        //#-------------------------------------------------------------------------------

        template <typename ReturnType, typename UserClass>
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
        //#-------------------------------------------------------------------------------

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
        //#-------------------------------------------------------------------------------

        template <typename UserClass>
        static WrappedValue callWrapped(UserClass *pObject, const BindInfo *pBinding, const WrappedValue::Args &args)
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
        //#-------------------------------------------------------------------------------

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
        //#-------------------------------------------------------------------------------

        template <typename UserClass>
        static void setWrapped(UserClass *pObject, const BindInfo *pBinding, const WrappedValue::Args &args)
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
        //#-------------------------------------------------------------------------------
    }; //# struct BindingHelper

    template <class UserClass>
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
                return BindingHelper::callMethod<ReturnType, UserClass, Args...>(pObject, pBinding, std::forward<Args>(args)...); // direct
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
                return BindingHelper::get<ReturnType, UserClass>(pObject, pBinding);
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
        //         return util::bound_set(pBinding, pObj, value);
        //     throw std::invalid_argument("Unable to call set() - not a variable / property");
        // }

        WrappedValue callWrapped(std::string_view name, const WrappedValue::Args &args) override
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

        void setWrapped(std::string_view name, const WrappedValue::Args &args) override
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