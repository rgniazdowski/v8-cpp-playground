#pragma once
#ifndef FG_INC_SCRIPT_MODULE
#define FG_INC_SCRIPT_MODULE

#include <string>
#include <script/V8.hpp>
#include <util/Tag.hpp>

namespace script
{
    class Module
    {
    public:
        enum Mode
        {
            /// Automatically registered with every context, this is not a loadable module
            BuiltinGlobalsOnly,
            /// Loadable internal module (on demand) that can possible pollute globals and setup exports (C++ bindings)
            InternalModule,
            /// Loadable module using custom methods (explicit require) and exports objects within (contains)
            SimpleModule,
            /// ES module / ECMAScript module that uses import/export keywords, using internal V8 mechanism to process the module
            ScriptModule
        };

    public:
        Module(v8::Isolate *isolate, std::string_view name) : m_init(false), m_mode(BuiltinGlobalsOnly), m_name(name), m_globals() {}

        Module(const Module &other) = delete;
        Module(Module &&other) = delete;

        virtual ~Module() { m_globals.Reset(); }

        inline bool isInitialized(void) const noexcept { return m_init; }
        inline const std::string &getName(void) const noexcept { return m_name; }
        inline Mode getMode(void) const noexcept { return m_mode; }
        inline bool isLoadable(void) const noexcept { return m_mode != BuiltinGlobalsOnly; }

    public:
        /**
         * Create object/function templates or compile the code and extract exports/globals from it.
         */
        virtual bool initialize(void) = 0;

        /**
         * This transfers objects from a modified 'global' object into the target context.
         * Can just return false and do nothing if there are no globals to transfer.
         */
        virtual bool instantiateGlobals(LocalContext &context) = 0;

        /**
         * Registering this module - set bindings/exports as parameter exports object prototype.
         * This can do nothing and return false if there are not exports / this is not that kind of module.
         */
        virtual bool registerModule(LocalObject &exports) = 0;

    protected:
        inline void setMode(Mode mode) { m_mode = mode; }

    protected:
        bool m_init;
        Mode m_mode;
        std::string m_name;
        PersistentObject m_globals;
    }; //# class InternalModule
} //> namespace script

#endif //> FG_INC_SCRIPT_MODULE
