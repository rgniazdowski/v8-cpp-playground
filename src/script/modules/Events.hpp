#pragma once
#ifndef FG_INC_SCRIPT_MODULES_EVENTS
#define FG_INC_SCRIPT_MODULES_EVENTS

#include <script/modules/Timers.hpp>
#include <event/EventDefinitions.hpp>
#include <util/Logger.hpp>
#include <util/Callbacks.hpp>

#include <v8pp/module.hpp>
#include <v8pp/class.hpp>

namespace script::modules
{
    class Events : public Timers
    {
        using base_type = Timers;
        friend class ::script::ScriptManager;

    public:
        Events(v8::Isolate *isolate);
        virtual ~Events();

    public:
        bool initialize(void) override;
        bool instantiateGlobals(LocalContext &context) override;

        static void addCallback(FunctionCallbackInfo const &args);
        static void deleteCallback(FunctionCallbackInfo const &args);

    protected:
        v8pp::module m_module;
        v8pp::module m_eventTypes;
        v8pp::module m_keyCodes;
        v8pp::module m_keyMods;
        v8pp::class_<util::Callback> m_class_callback;

        v8pp::class_<event::EventBase> m_class_base;
        v8pp::class_<event::EventTouch> m_class_touch;
        v8pp::class_<event::EventMouse> m_class_mouse;
        v8pp::class_<event::EventSwipe> m_class_swipe;
        v8pp::class_<event::EventSwipePinch> m_class_swipePinch;
        v8pp::class_<event::EventSwipeRotate> m_class_swipeRotate;
        v8pp::class_<event::EventKey> m_class_key;
        v8pp::class_<event::EventResource> m_class_resource;
        v8pp::class_<event::EventVertexStream> m_class_vertexStream;
        v8pp::class_<event::EventCamera> m_class_camera;
        v8pp::class_<event::EventSound> m_class_sound;
        v8pp::class_<event::EventMenuChanged> m_class_menuChanged;
        v8pp::class_<event::EventWidget> m_class_widget;
        v8pp::class_<event::EventGuiAction> m_class_guiAction;
        v8pp::class_<event::EventSensors> m_class_sensors;
        v8pp::class_<event::EventSplashScreen> m_class_splash;
        v8pp::class_<event::EventLoading> m_class_loading;
        v8pp::class_<event::EventProgram> m_class_program;
        v8pp::class_<event::EventControllerDevice> m_class_controller;
        v8pp::class_<event::EventControllerButton> m_class_controllerButton;
        v8pp::class_<event::EventControllerAxis> m_class_controllerAxis;
    }; //# class Timers
} //> namespace script::modules

#endif //> FG_INC_SCRIPT_MODULES_EVENTS