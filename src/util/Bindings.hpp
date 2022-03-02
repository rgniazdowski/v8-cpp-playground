#pragma once
#ifndef FG_INC_UTIL_BINDINGS
#define FG_INC_UTIL_BINDINGS

#include <typeinfo>
#include <functional>
#include <string>
#include <util/Vector.hpp>
#include <util/EnumName.hpp>
#include <util/UnpackCaller.hpp>

namespace util
{
    class WrappedValue
    {
    public:
        using Args = std::vector<WrappedValue *>;

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

        static const char *enum_name(Type type)
        {
            return ::util::id_to_name<Type>(type, s_typeEnumNames);
        }

        static Type enum_value(const std::string &name)
        {
            return ::util::name_to_id<Type>(name, s_typeEnumNames);
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
        inline static ::util::IdAndName<Type> s_typeEnumNames[] = {
            ID_NAME(INVALID),
            ID_NAME(CHAR), ID_NAME(SIGNED_CHAR), ID_NAME(UNSIGNED_CHAR),
            ID_NAME(SHORT), ID_NAME(SIGNED_SHORT), ID_NAME(UNSIGNED_SHORT),
            ID_NAME(INT), ID_NAME(UNSIGNED_INT),
            ID_NAME(LONG), ID_NAME(UNSIGNED_LONG),
            ID_NAME(LONG_LONG), ID_NAME(UNSIGNED_LONG_LONG),
            ID_NAME(FLOAT), ID_NAME(DOUBLE),
            ID_NAME(BOOL), ID_NAME(STRING), ID_NAME(EXTERNAL)};

        struct External
        {
            uint64_t identifier;
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

        constexpr Type getType(void) const noexcept { return type; }

        constexpr uint64_t getExternalIdentifier(void) const
        {
            if (type == EXTERNAL)
                return packed.external.identifier;
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
            memset(&packed, 0, sizeof(packed));
        }

        WrappedValue(const char *_tname) : tname(_tname), type(WrappedValue::determineInternalType(_tname))
        {
            memset(&packed, 0, sizeof(packed));
        }

        WrappedValue(const char *_tname, void *_external, uint64_t _id) : tname(_tname), type(EXTERNAL)
        {
            memset(&packed, 0, sizeof(packed));
            packed.external.pointer = _external;
            packed.external.identifier = _id;
        }

        WrappedValue(const std::string &str, const char *_tname) : tname(_tname), type(STRING), strvalue(str)
        {
            memset(&packed, 0, sizeof(packed));
        }

        ~WrappedValue() { reset(); }

        WrappedValue &operator=(const WrappedValue &input)
        {
            memset(&packed, 0, sizeof(packed));
            memcpy(&packed, &input.packed, sizeof(packed));
            tname = input.tname;
            type = input.type;
            strvalue = input.strvalue;
            return *this;
        }

        WrappedValue(const WrappedValue &input)
        {
            memset(&packed, 0, sizeof(packed));
            memcpy(&packed, &input.packed, sizeof(packed));
            tname = input.tname;
            type = input.type;
            strvalue = input.strvalue;
        }

        WrappedValue(WrappedValue &&input) : type(std::exchange(input.type, WrappedValue::INVALID)),
                                             tname(std::move(tname)),
                                             strvalue(std::move(strvalue))
        {
            input.packed.external.identifier = 0;
            input.packed.external.pointer = nullptr;
        }

        void reset(void)
        {
            memset(&packed, 0, sizeof(packed));
            type = INVALID;
            tname.clear();
            strvalue.clear();
        }

        template <typename InputType>
        static WrappedValue *wrap(const InputType &value)
        {
            WrappedValue *wrapped = new WrappedValue(typeid(value).name());
            wrapped->set(value);
            return wrapped;
        }

        template <>
        static WrappedValue *wrap(const std::string &value)
        {
            return new WrappedValue(value, typeid(value).name());
        }

        // template <unsigned int N>
        // static WrappedValue *wrap(const char (&value)[N])
        //{
        //     /* outside of template */
        //     return new WrappedValue(std::string(value), typeid(value).name());
        // }

        static WrappedValue *wrap(const char *value)
        {
            /* outside of template */
            return new WrappedValue(std::string(value), typeid(value).name());
        }

        template <typename T>
        static WrappedValue *external(T **value, uint64_t id)
        {
            return new WrappedValue(typeid(T *).name(), !value ? nullptr : *value, id);
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
                return util::convert<T>::convertToPointer(_data->pointer, _data->identifier);
            }
            return nullptr; // nothing else to return
        }
    }; //# class WrappedValue
    //#-----------------------------------------------------------------------------------

    void reset_arguments(WrappedValue::Args &args)
    {
        for (auto idx = 0; idx < args.size(); idx++)
        {
            auto arg = args[idx];
            delete arg;
            args[idx] = nullptr;
        }
        args.clear();
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
            METHOD = 0x1,
            PROPERTY = 0x2,
            VARIABLE = 0x4
        };

        static const char *enum_name(Type type)
        {
            return ::util::id_to_name<Type>(type, s_typeEnumNames);
        }

        static Type enum_value(const std::string &name)
        {
            return ::util::name_to_id<Type>(name, s_typeEnumNames);
        }

    protected:
        Type type;
        std::string name;
        inline static ::util::IdAndName<Type> s_typeEnumNames[] = {ID_NAME(METHOD), ID_NAME(PROPERTY), ID_NAME(VARIABLE)};

    protected:
        BindInfo(const std::string &_name, Type _type) : name(_name), type(_type) {}

        virtual ~BindInfo() {}

    public:
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
    class BindInfoMethodWrapped
    {
    public:
        using WrappedFunction = std::function<WrappedValue *(void *, const WrappedValue::Args &)>;
        struct ParameterInfo
        {
            std::string name;
            std::string type;
        };

    protected:
        WrappedFunction wrappedFunction;
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

    public:
        BindInfoMethodWrapped() : wrappedFunction(), parameters(), returnType() {}
        virtual ~BindInfoMethodWrapped() {}

        template <class UserClass>
        WrappedValue *callWrapped(UserClass *pObj, const WrappedValue::Args &args)
        {
            if (wrappedFunction)
                return wrappedFunction(static_cast<void *>(pObj), args);
            else
                return nullptr;
        }

        const std::vector<ParameterInfo> &getParameters(void) const { return parameters; }

        const std::string &getReturnType(void) const { return returnType; }
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
            this->wrappedFunction = [methodMember](void *pObj, const WrappedValue::Args &args)
            {
                auto value = unpack_caller(static_cast<UserClass *>(pObj), methodMember, args);
                return WrappedValue::external(&value, value->getIdentifier());
            };
        }

        template <bool is_pointer = std::is_pointer<ReturnType>::value>
        typename std::enable_if<is_pointer == false>::type wrap(DirectMethod methodMember)
        {
            this->wrappedFunction = [methodMember](void *pObj, const WrappedValue::Args &args)
            {
                auto value = unpack_caller(static_cast<UserClass *>(pObj), methodMember, args);
                return WrappedValue::wrap(value);
            };
        }

    public:
        BindInfoMethod(const std::string &methodName, DirectMethod methodMember, const std::initializer_list<std::string> &argNames = {}) : BindInfo(methodName, METHOD), BindInfoMethodWrapped()
        {
            this->setupParameters(argNames, {std::string(typeid(Args).name())...});
            this->returnType.assign(typeid(ReturnType).name());
            this->direct = methodMember;
            this->wrap(methodMember);
        }

        ~BindInfoMethod() {}

        ReturnType call(UserClass *pObj, Args &&...args)
        {
            if (this->type == METHOD && this->direct)
            {
                return ((pObj)->*(this->direct))(std::forward<Args>(args)...); // call anyway
            }
            throw std::invalid_argument("Unable to call member method - it's a property");
        }

        ReturnType callWithArgs(UserClass *pObj, WrappedValue::Args &args)
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
     */
    class BindInfoPropertyWrapped
    {
    public:
        using GetterWrappedFunction = std::function<WrappedValue *(void *pObj)>;
        using SetterWrappedFunction = std::function<void(void *pObj, const WrappedValue::Args &)>;

    protected:
        GetterWrappedFunction getterWrapped;
        SetterWrappedFunction setterWrapped;
        std::string valueType;

    public:
        BindInfoPropertyWrapped() {}
        virtual ~BindInfoPropertyWrapped() {}

        template <class UserClass>
        WrappedValue *getWrapped(UserClass *pObj) const
        {
            if (getterWrapped)
                return getterWrapped(static_cast<void *>(pObj));
            else
                return nullptr;
        }

        template <class UserClass>
        void setWrapped(UserClass *pObj, const WrappedValue::Args &args)
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
                return WrappedValue::external(&value, value->getIdentifier());
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
                return WrappedValue::wrap(value);
            };
            this->setterWrapped = [_setter](void *pObj, const WrappedValue::Args &args)
            {
                unpack_caller(static_cast<UserClass *>(pObj), _setter, args);
            };
        }

    public:
        BindInfoProperty(const std::string &propertyName, GetterMethod _getter, SetterMethod _setter) : BindInfo(propertyName, PROPERTY), BindInfoPropertyWrapped(), getter(_getter), setter(_setter)
        {
            this->valueType.assign(typeid(ReturnType).name());
            this->wrap(_getter, _setter);
        }

        BindInfoProperty(const std::string &propertyName, GetterMethod _getter) : BindInfo(propertyName, PROPERTY), BindInfoPropertyWrapped(), getter(_getter), setter(nullptr)
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
        typename std::enable_if<is_void == true>::type set(UserClass *pObj, const InputType &value) {}

        template <bool is_void = std::is_void<ReturnType>::value, class InputType>
        typename std::enable_if<is_void == false>::type set(UserClass *pObj, const InputType &value)
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
                return WrappedValue::external(&value, value->getIdentifier());
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
                return WrappedValue::wrap(value); // ptr
            };
            this->setterWrapped = [pThis](void *pObj, const WrappedValue::Args &args)
            {
                pThis->set(static_cast<UserClass *>(pObj), *(args[0]));
            };
        }

    public:
        BindInfoVariable(const std::string &variableName, DecayedVariable _var) : BindInfo(variableName, VARIABLE), BindInfoVariableWrapped(), var(_var)
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
        typename std::enable_if<is_void == true>::type set(UserClass *pObj, const DecayedVarType &value) {}

        template <bool is_void = std::is_void<VarType>::value>
        typename std::enable_if<is_void == false>::type set(UserClass *pObj, const DecayedVarType &value)
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

    private:
        inline static ::util::IdAndName<Type> s_typeEnumNames[] = {ID_NAME(UNSET), ID_NAME(CALL), ID_NAME(SET), ID_NAME(GET)};

    public:
        static const char *enum_name(Type type)
        {
            return ::util::id_to_name<Type>(type, s_typeEnumNames);
        }
        static Type enum_value(const std::string &name)
        {
            return ::util::name_to_id<Type>(name, s_typeEnumNames);
        }

        BindInfo::Type targetType;
        std::string targetName;
        int64_t objectId;
        std::string objectTypeName;
        WrappedValue::Args args;
        WrappedValue result;
        std::string nonce;

        WrappedAction() : type(UNSET), targetType(BindInfo::METHOD), targetName(), objectId(0), objectTypeName(), args(), result() {}

        // WrappedAction(const WrappedAction &in)
        //{
        //     int xd = 0;
        // }

        WrappedAction(WrappedAction &&in) : type(std::exchange(in.type, UNSET)),
                                            targetType(std::exchange(in.targetType, BindInfo::METHOD)),
                                            targetName(std::move(in.targetName)),
                                            objectId(std::exchange(in.objectId, 0)),
                                            objectTypeName(std::move(in.objectTypeName)),
                                            args(std::move(in.args)),
                                            result(std::move(in.result)),
                                            nonce(std::move(in.nonce))
        {
            int xyz = 123;
        }

        ~WrappedAction()
        {
            type = UNSET;
            objectId = 0;
            targetName.clear();
            objectTypeName.clear();
            nonce.clear();
            reset_arguments(args);
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
        MetadataBindings(const std::string &_typeName) : typeName(_typeName), bindings() {}

        const std::string &getTypeName(void) const noexcept { return typeName; }

        const BindInfo::Bindings &getBindings(void) const noexcept { return bindings; }

        template <class UserClass, typename ReturnType, typename... Args>
        self_type &method(const std::string &methodName, ReturnType (UserClass::*methodMember)(Args...), const std::initializer_list<std::string> &argNames = {})
        {
            bindings.push_back(new BindInfoMethod<UserClass, ReturnType, Args...>(methodName, methodMember, argNames));
            return *this;
        }
        template <class UserClass, typename ReturnType>
        self_type &property(const std::string &propertyName, ReturnType (UserClass::*getter)() const, void (UserClass::*setter)(ReturnType))
        {
            bindings.push_back(new BindInfoProperty<UserClass, ReturnType>(propertyName, getter, setter));
            return *this;
        }
        template <class UserClass, typename ReturnType>
        self_type &property(const std::string &propertyName, ReturnType (UserClass::*getter)() const)
        {
            bindings.push_back(new BindInfoProperty<UserClass, ReturnType>(propertyName, getter));
            return *this;
        }
        template <class UserClass, typename VarType>
        self_type &variable(const std::string &variableName, VarType UserClass::*var)
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

        bool has(BindInfo::Type type, const std::string &name) const
        {
            return (get(type, name) != nullptr);
        }

        BindInfo *get(BindInfo::Type type, const std::string &name) const
        {
            auto found = std::find_if(bindings.begin(), bindings.end(), [&type, &name](BindInfo *const &binding)
                                      { return binding->getType() == type && binding->getName().compare(name) == 0; });
            if (found != bindings.end())
                return *found;
            else
                return nullptr;
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

        virtual WrappedValue *callWrapped(const std::string &name, WrappedValue::Args &args) = 0;

        virtual WrappedValue *getWrapped(const std::string &name) = 0;

        virtual void setWrapped(const std::string &name, WrappedValue::Args &args) = 0;

        virtual bool executeAction(WrappedAction &action) = 0;
    }; //# class BindingsInvokerBase
    //#-----------------------------------------------------------------------------------

    template <class UserClass>
    class BindingsInvoker : public BindingsInvokerBase
    {
        UserClass *pObj;

    public:
        BindingsInvoker(UserClass *object, const MetadataBindings &metadata) : BindingsInvokerBase(metadata), pObj(object) {}
        BindingsInvoker(UserClass *object, const MetadataBindings *metadata) : BindingsInvokerBase(metadata), pObj(object) {}
        virtual ~BindingsInvoker()
        {
            pMetadata = nullptr;
            pObj = nullptr;
        }

        void setObject(UserClass *object) { pObj = object; }

    public:
        template <typename ReturnType, typename... Args>
        ReturnType call(const std::string &name, Args &&...args)
        {
            auto pBinding = pMetadata->get(BindInfo::METHOD, name);
            if (pBinding != nullptr)
            {
                auto pBindingCast = static_cast<BindInfoMethod<UserClass, ReturnType, Args...> *>(pBinding);
                return pBindingCast->call(pObj, std::forward<Args>(args)...); // direct
                // return util::bound_call<UserClass, ReturnType, Args...>(pBinding, pObj, std::forward<Args>(args)...);
            }
            throw std::invalid_argument("Unable to find bound method");
        }

        template <typename ReturnType>
        ReturnType get(const std::string &name)
        {
            auto pBinding = pMetadata->get(BindInfo::PROPERTY, name);
            if (pBinding != nullptr)
            {
                auto pBindingCast = static_cast<BindInfoProperty<UserClass, ReturnType> *>(pBinding); // PROPERTY
                return pBindingCast->get(pObj);                                                       // direct
            }
            pBinding = pMetadata->get(BindInfo::VARIABLE, name);
            if (pBinding != nullptr)
            {
                auto pBindingCast = static_cast<BindInfoVariable<UserClass, ReturnType> *>(pBinding); // VARIABLE
                return pBindingCast->get(pObj);                                                       // direct
            }
            //    return util::bound_get<UserClass, ReturnType>(pBinding, pObj);
            throw std::invalid_argument("Unable to call get() - not a variable / property");
        }

        // template <typename InputType, bool is_pointer = std::is_pointer<InputType>::value, bool is_reference = std::is_reference<InputType>::value>
        // typename std::enable_if<is_pointer == true || is_reference == false>::type set(const std::string &name, InputType value)
        template <typename InputType>
        void set(const std::string &name, const InputType &value)
        {
            auto pBinding = pMetadata->get(BindInfo::PROPERTY, name);
            if (pBinding != nullptr)
            {
                auto pBindingCast = static_cast<BindInfoProperty<UserClass, InputType> *>(pBinding); // PROPERTY
                pBindingCast->set(pObj, value);                                                      // direct
                return;
            }
            pBinding = pMetadata->get(BindInfo::VARIABLE, name);
            if (pBinding != nullptr)
            {
                auto pBindingCast = static_cast<BindInfoVariable<UserClass, InputType> *>(pBinding); // VARIABLE
                pBindingCast->set(pObj, value);                                                      // direct
                return;
            }
            // return util::bound_set(pBinding, pObj, value);
            throw std::invalid_argument("Unable to call set() - not a variable / property");
        }

        // template <typename InputType, bool is_pointer = std::is_pointer<InputType>::value, bool is_reference = std::is_reference<InputType>::value>
        // typename std::enable_if<is_pointer == false && is_reference == true>::type set(const std::string &name, const InputType &value)
        //{
        //     auto pBinding = pMetadata->get(BindInfo::PROPERTY, name);
        //     if (pBinding == nullptr)
        //         pBinding = pMetadata->get(BindInfo::VARIABLE, name);
        //     if (pBinding != nullptr)
        //         return util::bound_set(pBinding, pObj, value);
        //     throw std::invalid_argument("Unable to call set() - not a variable / property");
        // }

        WrappedValue *callWrapped(const std::string &name, WrappedValue::Args &args) override
        {
            auto pBinding = pMetadata->get(BindInfo::METHOD, name);
            if (pBinding != nullptr)
            {
                BindInfoMethodWrapped *pBindingCast = dynamic_cast<BindInfoMethodWrapped *>(pBinding);
                return pBindingCast->callWrapped(pObj, args); // pointer - allocated
                // return util::bound_call_wrapped(pBinding, pObj, args);
            }
            throw std::invalid_argument("Unable to find bound method");
        }

        WrappedValue *getWrapped(const std::string &name) override
        {
            auto pBinding = pMetadata->get(BindInfo::PROPERTY, name);
            if (pBinding != nullptr)
            {
                BindInfoPropertyWrapped *pBindingCast = dynamic_cast<BindInfoPropertyWrapped *>(pBinding); // PROPERTY
                return pBindingCast->getWrapped(pObj);                                                     // pointer - allocated
            }
            pBinding = pMetadata->get(BindInfo::VARIABLE, name);
            if (pBinding != nullptr)
            {
                BindInfoVariableWrapped *pBindingCast = dynamic_cast<BindInfoVariableWrapped *>(pBinding); // VARIABLE
                return pBindingCast->getWrapped(pObj);                                                     // pointer - allocated
            }
            // return util::bound_get_wrapped(pBinding, pObj);
            throw std::invalid_argument("Unable to call get() - not a variable / property");
        }

        void setWrapped(const std::string &name, WrappedValue::Args &args) override
        {
            auto pBinding = pMetadata->get(BindInfo::PROPERTY, name);
            if (pBinding != nullptr)
            {
                BindInfoPropertyWrapped *pBindingCast = dynamic_cast<BindInfoPropertyWrapped *>(pBinding);
                pBindingCast->setWrapped(pObj, args);
                return;
            }
            pBinding = pMetadata->get(BindInfo::VARIABLE, name);
            if (pBinding != nullptr)
            {
                BindInfoVariableWrapped *pBindingCast = dynamic_cast<BindInfoVariableWrapped *>(pBinding);
                pBindingCast->setWrapped(pObj, args); // wrapped args - no result
                return;
            }
            // return util::bound_set_wrapped(pBinding, pObj, args);
            throw std::invalid_argument("Unable to call set() - not a variable / property");
        }

        bool executeAction(WrappedAction &action) override
        {
            if (!action.isValid())
            {
                action.result.reset();
                return false; // skip
            }
            if (!this->pObj)
                return false;
            // need to assume that this object (invoker is already prepared and object pointer
            // was externally updated before calling action.
            // Wrapped action contains unique object identification, it allows to retrieve pointer
            // to any type of registered object (it has to be kept in map to be found).
            //
            // action contains wrapped value for result - before executing it should be cleared
            action.result.reset();
            WrappedValue *pResult = nullptr;
            if (action.type == WrappedAction::CALL)
                pResult = callWrapped(action.targetName, action.args);
            else if (action.type == WrappedAction::GET)
                pResult = getWrapped(action.targetName);
            else if (action.type == WrappedAction::SET)
                setWrapped(action.targetName, action.args);
            if (pResult != nullptr)
            {
                action.result = *pResult; // copy result to target
                delete pResult;           // clear allocated result
            }
            return true;
        }
    }; //# class BindingsInvoker
    //#-----------------------------------------------------------------------------------
} //> namespace util

#endif //> FG_INC_UTIL_BINDINGS