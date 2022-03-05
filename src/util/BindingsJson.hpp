#pragma once
#ifndef FG_INC_UTIL_BINDINGS_JSON
#define FG_INC_UTIL_BINDINGS_JSON

#include "Bindings.hpp"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace util
{
    struct json_helper
    {
        /**
         * @brief
         *
         * @param output
         * @param key
         * @param input
         */
        static void unwrap_value(json &output, const std::string &key, const WrappedValue &input)
        {
            auto type = input.getType();
            if (type == WrappedValue::CHAR)
                output["value"] = input.get<char>();
            else if (type == WrappedValue::SIGNED_CHAR)
                output["value"] = input.get<signed char>();
            else if (type == WrappedValue::UNSIGNED_CHAR)
                output["value"] = input.get<unsigned char>();
            else if (type == WrappedValue::SHORT)
                output["value"] = input.get<short>();
            else if (type == WrappedValue::SIGNED_SHORT)
                output["value"] = input.get<signed short>();
            else if (type == WrappedValue::UNSIGNED_SHORT)
                output["value"] = input.get<unsigned short>();
            else if (type == WrappedValue::INT)
                output["value"] = input.get<int>();
            else if (type == WrappedValue::UNSIGNED_INT)
                output["value"] = input.get<unsigned int>();
            else if (type == WrappedValue::LONG)
                output["value"] = input.get<long>();
            else if (type == WrappedValue::UNSIGNED_LONG)
                output["value"] = input.get<unsigned long>();
            else if (type == WrappedValue::LONG_LONG)
                output["value"] = input.get<long long>();
            else if (type == WrappedValue::UNSIGNED_LONG_LONG)
                output["value"] = input.get<unsigned long long>();
            else if (type == WrappedValue::FLOAT)
                output["value"] = input.get<float>();
            else if (type == WrappedValue::DOUBLE)
                output["value"] = input.get<double>();
            else if (type == WrappedValue::BOOL)
                output["value"] = input.get<bool>();
            else if (type == WrappedValue::STRING)
                output["value"] = input.get<std::string>();
        }
        /**
         * @brief
         *
         * @tparam SourceType
         * @param output
         * @param input
         * @param type
         */
        template <typename SourceType>
        static void wrap_value(WrappedValue &output, const json &input, WrappedValue::Type type)
        {
            if (input.is_null() || input.is_discarded() || input.is_structured())
                return; // skip completely
            SourceType _value = input.get<SourceType>();
            if (type == WrappedValue::CHAR)
                output.set<char>(static_cast<char>(_value));
            else if (type == WrappedValue::SIGNED_CHAR)
                output.set<signed char>(static_cast<signed char>(_value));
            else if (type == WrappedValue::UNSIGNED_CHAR)
                output.set<unsigned char>(static_cast<unsigned char>(_value));
            else if (type == WrappedValue::SHORT)
                output.set<short>(static_cast<short>(_value));
            else if (type == WrappedValue::SIGNED_SHORT)
                output.set<signed short>(static_cast<signed short>(_value));
            else if (type == WrappedValue::UNSIGNED_SHORT)
                output.set<unsigned short>(static_cast<unsigned short>(_value));
            else if (type == WrappedValue::INT)
                output.set<int>(static_cast<int>(_value));
            else if (type == WrappedValue::UNSIGNED_INT)
                output.set<unsigned int>(static_cast<unsigned int>(_value));
            else if (type == WrappedValue::LONG)
                output.set<long>(static_cast<long>(_value));
            else if (type == WrappedValue::UNSIGNED_LONG)
                output.set<unsigned long>(static_cast<unsigned long>(_value));
            else if (type == WrappedValue::LONG_LONG)
                output.set<long long>(static_cast<long long>(_value));
            else if (type == WrappedValue::UNSIGNED_LONG_LONG)
                output.set<unsigned long long>(static_cast<unsigned long long>(_value));
            else if (type == WrappedValue::FLOAT)
                output.set<float>(static_cast<float>(_value));
            else if (type == WrappedValue::DOUBLE)
                output.set<double>(static_cast<double>(_value));
            else if (type == WrappedValue::BOOL)
                output.set<bool>(static_cast<bool>(_value));
        }
    };
    //#-----------------------------------------------------------------------------------

    void to_json(json &output, const WrappedValue &input)
    {
        std::string typeName = input.getTypeName();
        output["typeName"] = typeName;
        output["type"] = WrappedValue::enum_name(input.getType());
        output["external"] = {{"handle", input.getExternalHandle()}};
        // WrappedValue::external_struct
        if (!input.isExternal())
        {
            // Now need explicit conversion based on the string type name (which is quite dangerous and limited)
            // WrappedValue also does not support big pieces of data (binary), might be possible to add later on.
            // Honestly there is no other way, however at this moment type is determined from string (instead of template).
            ::util::json_helper::unwrap_value(output, "value", input);
        }
    } //# to_json WrappedValue
    //#-----------------------------------------------------------------------------------

    void from_json(const json &input, WrappedValue &output)
    {
        WrappedValue::Type type = WrappedValue::INVALID;
        std::string typeName;
        std::string typestr;
        uint64_t handle = 0;
        json value;
        if (input.contains("typeName"))
            input.at("typeName").get_to(typeName);
        if (input.contains("type"))
        {
            input.at("type").get_to(typestr);
            type = WrappedValue::enum_value(typestr);
        }
        if (type == WrappedValue::EXTERNAL)
        {
            if (input.contains("external"))
            {
                auto external = input.at("external");
                if (external.contains("handle"))
                {
                    external.at("handle").get_to(handle);
                    output = WrappedValue(typeName.c_str(), nullptr, handle);
                }
            }
        }
        else if (type == WrappedValue::STRING)
        {
            if (input.contains("value"))
                value = input.at("value");
            if (value.is_string())
                output = WrappedValue(value.get<std::string>(), typeName.c_str());
        }
        else if (type != WrappedValue::INVALID)
        {
            if (input.contains("value"))
                value = input.at("value");
            else
                return;
            if (value.is_boolean())
            {
                output = WrappedValue(typeName.c_str());
                ::util::json_helper::wrap_value<bool>(output, value, type);
            }
            else if (value.is_number())
            {
                output = WrappedValue(typeName.c_str());
                ::util::json_helper::wrap_value<double>(output, value, type);
            }
            else if (value.is_number_float())
            {
                output = WrappedValue(typeName.c_str());
                ::util::json_helper::wrap_value<float>(output, value, type);
            }
            else if (value.is_number_integer())
            {
                output = WrappedValue(typeName.c_str());
                ::util::json_helper::wrap_value<int>(output, value, type);
            }
            else if (value.is_number_unsigned())
            {
                output = WrappedValue(typeName.c_str());
                ::util::json_helper::wrap_value<unsigned int>(output, value, type);
            }
        }
    } //# from_json WrappedValue
    //#-----------------------------------------------------------------------------------

    void to_json(json &output, const WrappedValue::Args &input)
    {
        output = json::array(); // force array type ?
        for (auto &arg : input)
        {
            const WrappedValue &value = *arg;
            output.push_back(value); // append to array
        }
    } //# to_json WrappedValue::Args
    //#-----------------------------------------------------------------------------------

    void from_json(const json &input, WrappedValue::Args &output)
    {
        if (!input.is_array())
            return; // cannot do anything without an array
        for (auto &element : input)
        {
            WrappedValue *pWrapped = new WrappedValue();
            from_json(element, *pWrapped);
            output.push_back(pWrapped);
        }
    } //# from_json WrappedValue::Args
    //#-----------------------------------------------------------------------------------

    void to_json(json &output, const WrappedAction &input)
    {
        output = {
            {"type", WrappedAction::enum_name(input.type)},
            {"targetType", BindInfo::enum_name(input.targetType)},
            {"targetName", input.targetName},
            {"objectId", input.objectId},
            {"objectTypeName", input.objectTypeName},
            {"args", input.args},
            {"result", input.result},
            {"nonce", input.nonce}};
    } //# to_json WrappedAction
    //#-----------------------------------------------------------------------------------

    void from_json(const json &input, WrappedAction &output)
    {
        if (input.contains("type"))
            output.type = WrappedAction::enum_value(input.at("type").get<std::string>());
        if (input.contains("targetType"))
            output.targetType = BindInfo::enum_value(input.at("targetType").get<std::string>());
        if (input.contains("targetName"))
            input.at("targetName").get_to(output.targetName);
        if (input.contains("objectId"))
            input.at("objectId").get_to(output.objectId);
        if (input.contains("objectTypeName"))
            input.at("objectTypeName").get_to(output.objectTypeName);
        if (input.contains("args"))
            from_json(input.at("args"), output.args);
        if (input.contains("result"))
            from_json(input.at("result"), output.result);
        if (input.contains("nonce"))
            input.at("nonce").get_to(output.nonce);
    } //# from_json WrappedAction
    //#-----------------------------------------------------------------------------------

    void to_json(json &output, const BindInfoMethodWrapped::ParameterInfo &input)
    {
        output = {{"name", input.name}, {"typeName", input.type}, {"type", WrappedValue::enum_name(WrappedValue::determineInternalType(input.type))}};
    } //# to_json BindInfoMethodWrapped::ParameterInfo
    //#-----------------------------------------------------------------------------------

    void to_json(json &output, const BindInfo &input)
    {
        output["name"] = input.getName();
        output["type"] = BindInfo::enum_name(input.getType());
        if (input.isMethod())
        {
            const BindInfo *pBinding = &input;
            const BindInfoMethodWrapped *pBindingCast = dynamic_cast<const BindInfoMethodWrapped *>(pBinding);
            if (pBindingCast != nullptr)
            {
                output["returnTypeName"] = pBindingCast->getReturnType();
                output["returnType"] = WrappedValue::enum_name(WrappedValue::determineInternalType(pBindingCast->getReturnType()));
                output["parameters"] = json::array();
                auto params = pBindingCast->getParameters();
                for (auto &param : params)
                    output["parameters"].push_back(param);
            }
        }
        else if (input.isProperty())
        {
            const BindInfo *pBinding = &input;
            const BindInfoPropertyWrapped *pBindingCast = dynamic_cast<const BindInfoPropertyWrapped *>(pBinding);
            if (pBindingCast != nullptr)
            {
                output["valueTypeName"] = pBindingCast->getValueType();
                output["valueType"] = WrappedValue::enum_name(WrappedValue::determineInternalType(pBindingCast->getValueType()));
            }
        }
        else if (input.isVariable())
        {
            const BindInfo *pBinding = &input;
            const BindInfoVariableWrapped *pBindingCast = dynamic_cast<const BindInfoVariableWrapped *>(pBinding);
            if (pBindingCast != nullptr)
            {
                output["valueTypeName"] = pBindingCast->getValueType();
                output["valueType"] = WrappedValue::enum_name(WrappedValue::determineInternalType(pBindingCast->getValueType()));
            }
        }
    } //# to_json BindInfo
    //#-----------------------------------------------------------------------------------

    void to_json(json &output, const BindInfo::Bindings &input)
    {
        output = json::array();
        for (auto &binding : input)
        {
            output.push_back(*binding);
        }
    } //# to_json BindInfo::Bindings
    //#-----------------------------------------------------------------------------------

    void to_json(json &output, const MetadataBindings &input)
    {
        output = {{"typeName", input.getTypeName()}, {"bindings", input.getBindings()}};
    } //# to_json MetadataBindings
    //#-----------------------------------------------------------------------------------

} //> namespace util

#endif //> FG_INC_UTIL_BINDINGS_JSON