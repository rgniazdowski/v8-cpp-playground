#pragma once
#ifndef FG_INC_SCRIPT_INTERNAL_MODULE
#define FG_INC_SCRIPT_INTERNAL_MODULE

#include <string>

#include <script/Module.hpp>

namespace script
{
    /**
     * Internal module is a special kind of module - without a source code, it is made of
     * custom C++ bindings mostly done via V8PP library. The idea here is to have a fast
     * way of populating global scope of a selected script context (specified on input).
     *
     * Registration of object templates and function templates is done in the first step,
     * only once - this is heavily bound to the lifetime of this object, all destructors
     * are called in the end for any registered objects and classes.
     *
     * Second step (which can happen more than once) is to create objects from templates
     * and place them in global scope of a given context and also register exports;
     */
    class InternalModule : public Module
    {
    public:
        InternalModule(v8::Isolate* isolate, std::string_view name) : Module(isolate, name)
        {
            setMode(BuiltinGlobalsOnly); // in most cases it will not be loadable
        }
        virtual ~InternalModule() {}
    }; //# class InternalModule
} //> namespace script

#endif //> FG_INC_SCRIPT_INTERNAL_MODULE
