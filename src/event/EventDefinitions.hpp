#ifndef FG_INC_EVENT_DEFINITIONS
#define FG_INC_EVENT_DEFINITIONS

#include <util/Timesys.hpp>
#include <event/KeyVirtualCodes.hpp>
#include <util/Handle.hpp>

//
// This file will contain all basic events occurring in the game engine
// also defines standard event structures holding info about the event
//
namespace resource
{
    class Resource;
    using ResourceTag = ::util::Tag<Resource>;
    using ResourceHandle = ::util::Handle<ResourceTag>;
}

/*namespace gui {
    class CMenu;
    class CWidget;
}*/

namespace event
{
    enum class Type : unsigned int
    {
        /// Invalid event code - never thrown
        Invalid = 0,

        /// Touch event - finger pressed against the screen
        TouchPressed = 1,
        /// Finger released
        TouchReleased = 2,
        /// Touch/finger motion
        TouchMotion = 3,
        /// Touch tap event
        TouchTapFinished = 4,

        /// Mouse button pressed
        MousePressed = 6,
        /// Mouse button released
        MouseReleased = 7,
        /// Mouse pointer motion
        MouseMotion = 8,
        /// Mouse tap event
        MouseTapFinished = TouchTapFinished,

        /// Swipe event - horizontal
        SwipeX = 10,
        /// Swipe event - vertical
        SwipeY = 11,
        /// Swipe event - mix/angle
        SwipeXY = 12,
        /// Swipe event - mix/angle
        SwipeMixed = SwipeXY,

        /// Swipe pinch event
        SwipePinch = 14,

        /// Swipe rotation event
        MultiSwipeRotate = 15,

        /// Key is being held down
        KeyDown = 16,
        /// Key is released
        KeyUp = 17,
        /// Key is just being pressed (changing state from up to down)
        KeyPressed = 18,
        /// Key is released
        KeyReleased = KeyUp,

        /// Resource was just created
        ResourceCreated = 30,
        /// Resource was removed from Resource Manager
        ResourceRemoved = 31,
        /// Resource was disposed (memory freed)
        ResourceDisposed = 32,
        /// Resource was destroyed - it is no longer available
        ResourceDestroyed = 33,
        /// Resource was requested (first use)
        ResourceRequested = 34,

        /// Event thrown when the program finishes initializing
        ProgramInit = 40,
        /// Event thrown on quit
        ProgramQuit = 41,
        /// Event thrown when program is suspended (outside pause, mostly mobile)
        ProgramSuspend = 42,
        /// Event thrown when program resumes (un-pause)
        ProgramResume = 43,

        ///
        LoadingBegin = 50,
        ///
        LoadingFinished = 51,
        ///
        SplashScreenShown = 52,
        /// The buffers are being swapped
        SwapBuffers = 53,
        /// Event called on every update function (per frame)
        UpdateShot = 54,
        /// Event called on every pre-render function (per frame)
        PreRenderShot = 55,
        /// Event called on every render function (per frame)
        RenderShot = 56,

        /// Event called when the frame freezes (special pause)
        FrameFrozen = 57,
        /// Event called when the frame resumes
        FrameUnfrozen = 58,

        ///
        VertexStreamReady = 60,
        ///
        VertexBufferReady = 61,
        ///
        CameraChanged = 62,

        /// Event thrown when the sound is starting to play
        SoundPlayed = 70,

        ///
        MenuChanged = 80,
        ///
        WidgetStateChanged = 81,
        ///
        GuiAction = 82,

        /// Event thrown on sensors change (gyro/accel)
        SensorsChanged = 90,

        /// Game controller (joystick) added
        GameControllerAdded = 100,
        /// Game controller (joystick) removed
        GameControllerRemoved = 101,
        /// Game controller (joystick) button event
        GameControllerButton = 102,
        /// Game controller (joystick) axis event
        GameControllerAxis = 103,

        CustomEvent = 105,

        /// Reserved event code
        Reserved1 = 110,
        /// Reserved event code
        Reserved2 = 111,
        /// Reserved event code
        Reserved3 = 112,

        /// Special id - last valid standard event code
        LastStandardEventCode = Reserved3
    };

    struct EventBase : public util::ObjectWithIdentifier
    {
    private:
        inline static uint64_t s_autoid = 0;

    public:
        Type eventType;
        int64_t timeStamp;
        uint64_t identifier;

        EventBase(Type _type = Type::Invalid) : eventType(_type), timeStamp(timesys::ticks()), identifier(EventBase::autoid()) {}
        inline uint64_t getIdentifier(void) const override { return identifier; }
        inline static uint64_t autoid(void) { return ++s_autoid; }
    };

    struct EventControllerDevice : EventBase
    {
        int which; // The joystick device index
    };

    struct EventControllerButton : EventBase
    {
        int which; // The joystick instance id
        unsigned short button;
        unsigned short state;
    };

    struct EventControllerAxis : EventBase
    {
        int which;           // The joystick instance id
        unsigned short axis; // Controller axis
        short int value;
    };

    struct EventTouch : EventBase
    {
        int x;
        int y;
        int relX;
        int relY;

        union
        {
            unsigned int touchID;
            unsigned int buttonID;
            unsigned int pointerID;
        };
        bool pressed;
    };

    struct EventMouse : EventBase
    {
        int x;
        int y;
        int relX;
        int relY;

        union
        {
            unsigned int touchID;
            unsigned int buttonID;
            unsigned int pointerID;
        };
        bool pressed;
    };

    struct EventSwipe : EventBase
    {

        enum Direction
        {
            Invalid,
            Left,
            Right,
            Up,
            Down,
            Angle
        } swipeDirection;
        int xStart;
        int yStart;
        int xEnd;
        int yEnd;
        int swipeXOffset;
        int swipeYOffset;
        int swipeXSteps;
        int swipeYSteps;

        const char *getDirectionAsString(void) const
        {
            auto name = magic_enum::enum_name<Direction>(swipeDirection);
            return name.data();
        }
    };

    struct EventSwipePinch : EventBase
    {
        enum Direction
        {
            Invalid,
            In,
            Out
        } pinchDirection;
        int x;
        int y;
        int x2;
        int y2;
        int pinchXOffset;
        int pinchYOffset;
        int pinchSize;

        const char *getDirectionAsString(void) const
        {
            auto name = magic_enum::enum_name<Direction>(pinchDirection);
            return name.data();
        }
    };

    struct EventSwipeRotate : EventBase
    { // Should extend SwipeEvent?
        int x;
        int y;
        int x2;
        int y2;
        float angle;
    };

    struct EventKey : EventBase
    {
        KeyCode keyCode;
        bool pressed;
        int repeats;
        KeyMod mod;

        inline bool isMod(void) const
        {
            if (keyCode == KeyCode::LSHIFT ||
                keyCode == KeyCode::RSHIFT ||
                keyCode == KeyCode::LCTRL ||
                keyCode == KeyCode::RCTRL ||
                keyCode == KeyCode::LALT ||
                keyCode == KeyCode::RALT ||
                keyCode == KeyCode::LGUI ||
                keyCode == KeyCode::RGUI)
            {
                return true;
            }
            return false;
        }

        inline bool isAlpha(void) const
        {
            if (keyCode >= KeyCode::A && keyCode <= KeyCode::Z)
            {
                return true;
            }
            return false;
        }

        inline bool isDigit(void) const
        {
            if ((keyCode >= KeyCode::N0 && keyCode <= KeyCode::N9) ||
                (keyCode >= KeyCode::NUM_0 && keyCode <= KeyCode::NUM_9))
            {
                return true;
            }
            return false;
        }

        inline bool isAlphaNum(void) const
        {
            return (bool)!!(isAlpha() || isDigit());
        }

        inline bool isSpace(void) const
        {
            if (keyCode == KeyCode::FF ||
                keyCode == KeyCode::LINEFEED ||
                keyCode == KeyCode::CR ||
                keyCode == KeyCode::HORIZONTAL_TAB ||
                keyCode == KeyCode::VERTICAL_TAB)
            {
                return true;
            }
            return false;
        }

        inline bool isShiftDown(void) const
        {
            if ((bool)!!(mod & KeyMod::LSHIFT))
                return true;
            if ((bool)!!(mod & KeyMod::RSHIFT))
                return true;
            return false;
        }

        inline bool isControlDown(void) const
        {
            if ((bool)!!(mod & KeyMod::LCTRL))
                return true;
            if ((bool)!!(mod & KeyMod::RCTRL))
                return true;
            return false;
        }

        inline bool isAltDown(void) const
        {
            if ((bool)!!(mod & KeyMod::LALT))
                return true;
            if ((bool)!!(mod & KeyMod::RALT))
                return true;
            return false;
        }

        inline bool isGuiDown(void) const
        {
            if ((bool)!!(mod & KeyMod::LGUI))
                return true;
            if ((bool)!!(mod & KeyMod::RGUI))
                return true;
            return false;
        }
    }; // struct EventKey

    struct EventResource : EventBase
    {
        enum Status
        {
            Created,
            Removed,
            Disposed,
            Destroyed,
            Requested
        } status;
        resource::ResourceHandle handle;

        const char *getStatusAsString(void) const
        {
            auto name = magic_enum::enum_name<Status>(status);
            return name.data();
        }
    };

    struct EventProgram : EventBase
    {
        bool isSuccess;
        bool isOriginGfx;
    };

    struct EventLoading : EventBase
    {
        enum Status
        {
            BEGIN = 0,
            CONTINUE = 1,
            FINISH = 2
        } status;
    };

    struct EventSplashScreen : EventBase
    {
        /// Status of the splashscreen display - no additional data is required?
        /// If true - splashscreen is no longer displayed
        /// If false - splashscreen just started or in the middle
        bool finish;
    };

    struct EventVertexStream : EventBase
    {
        // FG_GFXHANDLE vertexStreamHandle;
        // fgVertexStream *vertexStreamHolder;
    };

    struct EventCamera : EventBase
    {
        // GfxCamera *cameraHolder;
        // FG_GFXHANDLE cameraHandle;
    };

    struct EventSound : EventBase
    {
        // FG_RHANDLE soundHandle;
        // fgResource *soundHolder;
    };

    /**
     * Event structure for changing GUI menu events
     */
    struct EventMenuChanged : EventBase
    {
        /// Pointer to previous menu (can be NULL)
        // fg::gui::CWidget* prevMenu;
        /// Pointer to the next menu (current)
        // fg::gui::CWidget* nextMenu;
        /// Previous menu name
        const char *prevMenuName;
        /// Next menu name
        const char *nextMenuName;
        /// Set to TRUE if the changing did finished or to FALSE if the
        // changing is already in motion (animated)
        bool didChange;
    };

    struct EventWidget : EventBase
    {
        // FG_GUIHANDLE widgetHandle;
        // fgWidget
        // fgResource
    };

    struct EventGuiAction : EventBase
    {
        const char *action;
        bool isFinished;
    };

    struct EventSensors : EventBase
    {
        int sensorType;
        union
        {

            struct
            {
                float x, y, z;
            };
            float data[3];
        };
    };

    struct EventCustom : EventBase
    {
        void *ptr;
        unsigned long int customId;
    };

    struct EventReserved : EventBase
    {
        void *data1;
        void *data2;
        void *data3;
        int n_data;
    };

    struct EventCombined : public util::ObjectWithIdentifier
    {
        EventCombined(Type type) : eventType(type), timeStamp(timesys::ticks()), identifier(EventCombined::autoid())
        {
            swipe.xStart = 0;
            swipe.yStart = 0;
            swipe.xEnd = 0;
            swipe.yEnd = 0;
            swipe.swipeXOffset = 0;
            swipe.swipeYOffset = 0;
            swipe.swipeXSteps = 0;
            swipe.swipeYSteps = 0;
        }
        ~EventCombined()
        {
            eventType = Type::Invalid;
            timeStamp = 0;
            identifier = 0;
        }
        inline void setup(Type type)
        {
            eventType = type;
            timeStamp = timesys::ticks();
            identifier = event::EventCombined::autoid();
            swipe.xStart = 0;
            swipe.yStart = 0;
            swipe.xEnd = 0;
            swipe.yEnd = 0;
            swipe.swipeXOffset = 0;
            swipe.swipeYOffset = 0;
            swipe.swipeXSteps = 0;
            swipe.swipeYSteps = 0;
        }
        inline uint64_t getIdentifier(void) const override { return identifier; }
        inline static uint64_t autoid(void) { return EventBase::autoid(); }
        union
        {
            struct
            {
                Type eventType;
                int64_t timeStamp;
                uint64_t identifier;
            };
            EventTouch touch;
            EventMouse mouse;
            EventSwipe swipe;
            EventSwipePinch swipePinch;
            EventSwipeRotate swipeRotate;
            EventKey key;
            EventResource resource;
            EventVertexStream vertexStream;
            EventCamera camera;
            EventSound sound;
            EventMenuChanged menuChanged;
            EventWidget widget;
            EventGuiAction guiAction;
            EventSensors sensors;
            EventSplashScreen splash;
            EventLoading loading;
            EventProgram program;
            EventControllerDevice controller;
            EventControllerButton controllerButton;
            EventControllerAxis controllerAxis;

            EventCustom custom;
            EventReserved reserved;
        };
    }; //> struct EventCombined

} //> namespace event

#endif //> FG_INC_EVENT_DEFINITIONS