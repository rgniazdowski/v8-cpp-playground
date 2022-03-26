#include <script/modules/Events.hpp>
#include <util/Logger.hpp>
#include <event/EventManager.hpp>
#include <script/ScriptCallback.hpp>
#include <script/ScriptManager.hpp>

#include <magic_enum.hpp>
#include <iostream>

script::modules::Events::Events(v8::Isolate *isolate)
    : base_type(isolate), m_module(isolate),
      m_eventTypes(isolate), m_keyCodes(isolate), m_keyMods(isolate),
      m_class_callback(isolate),
      m_class_base(isolate),
      m_class_touch(isolate), m_class_mouse(isolate),
      m_class_swipe(isolate), m_class_swipePinch(isolate),
      m_class_swipeRotate(isolate), m_class_key(isolate),
      m_class_resource(isolate), m_class_vertexStream(isolate),
      m_class_camera(isolate), m_class_sound(isolate),
      m_class_menuChanged(isolate), m_class_widget(isolate),
      m_class_guiAction(isolate), m_class_sensors(isolate),
      m_class_splash(isolate), m_class_loading(isolate),
      m_class_program(isolate), m_class_controller(isolate),
      m_class_controllerButton(isolate), m_class_controllerAxis(isolate)
{
    m_name = "events-internal";
    setMode(BuiltinGlobalsOnly);
}
//>---------------------------------------------------------------------------------------

script::modules::Events::~Events()
{
    auto isolate = m_class_callback.isolate();
    removeClassObjects<util::Callback>(isolate);
    removeClassObjects<event::EventTouch, event::EventMouse, event::EventSwipe,
                       event::EventSwipePinch, event::EventSwipeRotate,
                       event::EventKey, event::EventResource,
                       event::EventVertexStream,
                       event::EventCamera, event::EventSound,
                       event::EventMenuChanged, event::EventWidget, event::EventGuiAction,
                       event::EventSensors,
                       event::EventSplashScreen,
                       event::EventLoading, event::EventProgram,
                       event::EventControllerDevice,
                       event::EventControllerButton,
                       event::EventControllerAxis>(isolate);
}
//>---------------------------------------------------------------------------------------

bool script::modules::Events::initialize(void)
{
    base_type::initialize();
    auto isolate = m_module.isolate();
    {
        auto entries = magic_enum::enum_entries<event::Type>();
        for (auto &it : entries)
            m_eventTypes.const_(it.second, it.first);
    }
    {
        auto entries = magic_enum::enum_entries<event::KeyCode>();
        for (auto &it : entries)
            m_keyCodes.const_(it.second, it.first);
    }
    {
        auto entries = magic_enum::enum_entries<event::KeyMod>();
        for (auto &it : entries)
            m_keyMods.const_(it.second, it.first);
    }
    // #FIXME - need to use wrapped type for manager and register it as a class (?)
    m_module.function("setInterval", &base_type::setInterval);
    m_module.function("setTimeout", &base_type::setTimeout);
    m_module.function("clearInterval", &base_type::clearTimeout);
    m_module.function("clearTimeout", &base_type::clearTimeout);
    m_module.function("addCallback", &Events::addCallback);
    m_module.function("deleteCallback", &Events::deleteCallback);

    setClassName(isolate, m_class_callback, "Callback");
    setClassName(isolate, m_class_base, "EventBase");
    setClassName(isolate, m_class_touch, "EventTouch");
    setClassName(isolate, m_class_mouse, "EventMouse");
    setClassName(isolate, m_class_swipe, "EventSwipe");
    setClassName(isolate, m_class_swipePinch, "EventSwipePinch");
    setClassName(isolate, m_class_swipeRotate, "EventSwipeRotate");
    setClassName(isolate, m_class_key, "EventKey");
    setClassName(isolate, m_class_resource, "EventResource");
    setClassName(isolate, m_class_vertexStream, "EventVertexStream");
    setClassName(isolate, m_class_camera, "EventCamera");
    setClassName(isolate, m_class_sound, "EventSound");
    setClassName(isolate, m_class_menuChanged, "EventMenuChanged");
    setClassName(isolate, m_class_widget, "EventWidget");
    setClassName(isolate, m_class_guiAction, "EventGuiAction");
    setClassName(isolate, m_class_sensors, "EventSensors");
    setClassName(isolate, m_class_splash, "EventSplashScreen");
    setClassName(isolate, m_class_loading, "EventLoading");
    setClassName(isolate, m_class_program, "EventProgram");
    setClassName(isolate, m_class_controller, "EventControllerDevice");
    setClassName(isolate, m_class_controllerButton, "EventControllerButton");
    setClassName(isolate, m_class_controllerAxis, "EventControllerAxis");

    m_class_base.var("eventType", &event::EventBase::eventType)
        .var("timeStamp", &event::EventBase::timeStamp)
        .function("getIdentifier", &event::EventBase::getIdentifier);

    m_class_touch.inherit<event::EventBase>()
        .var("x", &event::EventTouch::x)
        .var("y", &event::EventTouch::y)
        .var("relX", &event::EventTouch::relX)
        .var("relY", &event::EventTouch::relY)
        .var("touchID", &event::EventTouch::touchID)
        .var("buttonID", &event::EventTouch::buttonID)
        .var("pointerID", &event::EventTouch::pointerID)
        .var("pressed", &event::EventTouch::pressed);

    m_class_mouse.inherit<event::EventBase>()
        .var("x", &event::EventMouse::x)
        .var("y", &event::EventMouse::y)
        .var("relX", &event::EventMouse::relX)
        .var("relY", &event::EventMouse::relY)
        .var("touchID", &event::EventMouse::touchID)
        .var("buttonID", &event::EventMouse::buttonID)
        .var("pointerID", &event::EventMouse::pointerID)
        .var("pressed", &event::EventMouse::pressed);

    m_class_swipe.inherit<event::EventBase>()
        .var("swipeDirection", &event::EventSwipe::swipeDirection)
        .var("xStart", &event::EventSwipe::xStart)
        .var("yStart", &event::EventSwipe::yStart)
        .var("xEnd", &event::EventSwipe::xEnd)
        .var("yEnd", &event::EventSwipe::yEnd)
        .var("swipeXOffset", &event::EventSwipe::swipeXOffset)
        .var("swipeYOffset", &event::EventSwipe::swipeYOffset)
        .var("swipeXSteps", &event::EventSwipe::swipeXSteps)
        .var("swipeYSteps", &event::EventSwipe::swipeYSteps)
        .function("getDirectionAsString", &event::EventSwipe::getDirectionAsString);

    m_class_swipePinch.inherit<event::EventBase>()
        .var("pinchDirection", &event::EventSwipePinch::pinchDirection)
        .var("x", &event::EventSwipePinch::x)
        .var("y", &event::EventSwipePinch::y)
        .var("x2", &event::EventSwipePinch::x2)
        .var("y2", &event::EventSwipePinch::y2)
        .var("pinchXOffset", &event::EventSwipePinch::pinchXOffset)
        .var("pinchYOffset", &event::EventSwipePinch::pinchYOffset)
        .var("pinchSize", &event::EventSwipePinch::pinchSize)
        .function("getDirectionAsString", &event::EventSwipePinch::getDirectionAsString);

    m_class_swipeRotate.inherit<event::EventBase>()
        .var("x", &event::EventSwipeRotate::x)
        .var("y", &event::EventSwipeRotate::y)
        .var("x2", &event::EventSwipeRotate::x2)
        .var("y2", &event::EventSwipeRotate::y2)
        .var("angle", &event::EventSwipeRotate::angle);

    m_class_key.inherit<event::EventBase>()
        .var("keyCode", &event::EventKey::keyCode)
        .var("pressed", &event::EventKey::pressed)
        .var("repeats", &event::EventKey::repeats)
        .var("mod", &event::EventKey::mod)
        .function("isMod", &event::EventKey::isMod)
        .function("isAlpha", &event::EventKey::isAlpha)
        .function("isDigit", &event::EventKey::isDigit)
        .function("isAlphaNum", &event::EventKey::isAlphaNum)
        .function("isSpace", &event::EventKey::isSpace)
        .function("isShiftDown", &event::EventKey::isShiftDown)
        .function("isControlDown", &event::EventKey::isControlDown)
        .function("isAltDown", &event::EventKey::isAltDown)
        .function("isGuiDown", &event::EventKey::isGuiDown);

    m_class_resource.inherit<event::EventBase>()
        .var("status", &event::EventResource::status)
        .function("getStatusAsString", &event::EventResource::getStatusAsString);

    m_class_vertexStream.inherit<event::EventBase>();

    m_class_camera.inherit<event::EventBase>();

    m_class_sound.inherit<event::EventBase>();

    m_class_menuChanged.inherit<event::EventBase>();

    m_class_widget.inherit<event::EventBase>();

    m_class_guiAction.inherit<event::EventBase>();

    m_class_sensors.inherit<event::EventBase>()
        .var("sensorType", &event::EventSensors::sensorType)
        .var("x", &event::EventSensors::x)
        .var("y", &event::EventSensors::y)
        .var("z", &event::EventSensors::z);

    m_class_splash.inherit<event::EventBase>()
        .var("finish", &event::EventSplashScreen::finish);

    m_class_loading.inherit<event::EventBase>()
        .var("status", &event::EventLoading::status);

    m_class_program.inherit<event::EventBase>()
        .var("isSuccess", &event::EventProgram::isSuccess)
        .var("isOriginGfx", &event::EventProgram::isOriginGfx);

    m_class_controller.inherit<event::EventBase>();

    m_class_controllerButton.inherit<event::EventBase>();

    m_class_controllerAxis.inherit<event::EventBase>();

    registerExternalConverters<event::EventTouch, event::EventMouse, event::EventSwipe,
                               event::EventSwipePinch, event::EventSwipeRotate,
                               event::EventKey, event::EventResource,
                               event::EventVertexStream,
                               event::EventCamera, event::EventSound,
                               event::EventMenuChanged, event::EventWidget, event::EventGuiAction,
                               event::EventSensors,
                               event::EventSplashScreen,
                               event::EventLoading, event::EventProgram,
                               event::EventControllerDevice,
                               event::EventControllerButton,
                               event::EventControllerAxis>();

    m_init = true;
    return true;
} //> initialize()
//>---------------------------------------------------------------------------------------

bool script::modules::Events::instantiateGlobals(LocalContext &context)
{
    auto isolate = context->GetIsolate();
    auto global = context->Global();
    global->Set(context, v8pp::to_v8(isolate, "EventManager"), m_module.new_instance());
    global->Set(context, v8pp::to_v8(isolate, "EventType"), m_eventTypes.new_instance());
    global->Set(context, v8pp::to_v8(isolate, "KeyCode"), m_keyCodes.new_instance());
    global->Set(context, v8pp::to_v8(isolate, "KeyMod"), m_keyMods.new_instance());
    return true;
} //> instantiateGlobals(...)
//>---------------------------------------------------------------------------------------

static event::Type getEventTypeFromArgument(v8::Isolate *isolate, script::LocalValue value)
{
    auto context = isolate->GetCurrentContext();
    auto &eventType = value;
    if (eventType->IsString())
    {
        std::string stringEventType = v8pp::from_v8<std::string>(isolate, eventType);
        auto castEventType = magic_enum::enum_cast<event::Type>(stringEventType);
        if (castEventType.has_value())
            return castEventType.value();
    }
    else if (eventType->IsInt32() || eventType->IsUint32() || eventType->IsNumber())
    {
        auto castMaybeValue = eventType->Uint32Value(context);
        if (!castMaybeValue.IsNothing())
        {
            auto castValue = castMaybeValue.ToChecked();
            if (magic_enum::enum_contains<event::Type>(castValue))
                return magic_enum::enum_cast<event::Type>(castValue).value();
        }
    }
    return event::Type::Invalid;
} //> getEventTypeFromArgument(...)
//>---------------------------------------------------------------------------------------

void script::modules::Events::addCallback(FunctionCallbackInfo const &args)
{
    if (args.Length() < 2 || !s_pScriptMgr)
    {
        args.GetReturnValue().SetNull();
        return;
    }
    auto isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);
    auto context = isolate->GetCurrentContext();
    auto &eventType = args[0];
    auto &handler = args[1];
    if (!handler->IsFunction() && !handler->IsString())
    {
        args.GetReturnValue().SetNull();
        return;
    }
    event::Type nativeEventType = getEventTypeFromArgument(isolate, eventType);
    if (nativeEventType == event::Type::Invalid)
    {
        args.GetReturnValue().SetNull();
        return;
    }
    ScriptCallback *callback;
    if (handler->IsFunction())
    {
        auto function = handler.As<v8::Function>();
        callback = s_pScriptMgr->createScriptCallback(function);
    }
    else if (handler->IsString())
    {
        auto expression = handler.As<v8::String>();
        callback = s_pScriptMgr->createScriptCallback(expression);
    }
    auto managerRegistry = base::ManagerRegistry::instance();
    auto eventMgr = managerRegistry->get<event::EventManager>();
    eventMgr->addCallback(nativeEventType, callback);
    auto callbackObject = v8pp::class_<util::Callback>::reference_external(isolate, callback);
    args.GetReturnValue().Set(callbackObject);
}
//>---------------------------------------------------------------------------------------

void script::modules::Events::deleteCallback(FunctionCallbackInfo const &args)
{
    if (args.Length() < 2 || !s_pScriptMgr)
    {
        args.GetReturnValue().Set(false);
        return;
    }
    auto isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);
    auto context = isolate->GetCurrentContext();
    auto eventType = args[0];
    auto callbackValue = args[1];
    event::Type nativeEventType = getEventTypeFromArgument(isolate, eventType);
    if (nativeEventType == event::Type::Invalid || !callbackValue->IsObject())
    {
        args.GetReturnValue().Set(false);
        return;
    }
    auto callback = v8pp::class_<util::Callback>::unwrap_object(isolate, callbackValue);
    if (!callback)
    {
        args.GetReturnValue().Set(false);
        return;
    }
    bool status = false;
    auto managerRegistry = base::ManagerRegistry::instance();
    auto eventMgr = managerRegistry->get<event::EventManager>();
    if (eventMgr->removeCallback(nativeEventType, callback))
    {
        v8pp::class_<util::Callback>::unreference_external(isolate, callback);
        delete callback;
        status = true;
    }
    args.GetReturnValue().Set(status);
}
//>---------------------------------------------------------------------------------------
