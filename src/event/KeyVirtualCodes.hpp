#pragma once
#ifndef FG_INC_KEYBOARD
#define FG_INC_KEYBOARD

#include <util/EnumFlags.hpp>

namespace event
{
    enum class KeyCode : unsigned int
    {
        UNKNOWN = 0,
        NONE = 0,

        SOH = 1,             // SOH // Start of heading
        STX = 2,             // STX // Start of text
        ETX = 3,             // ETX // End of text
        EOT = 4,             // EOT // End of trans.
        ENQ = 5,             // ENQ // Enquiry
        ENQUIRY = ENQ,       // ENQ
        ACK = 6,             // ACK // Ack.
        BELL = 7,            // BEL // Bell \a
        BS = 8,              // BS // Back space \b
        BACKSPACE = BS,      // Back space \b
        HT = 9,              // HT // Horizontal tab \t
        HORIZONTAL_TAB = HT, // HT \t
        TAB = HT,            // TAB \t
        LF = 10,             // LF // Line feed \n
        LINEFEED = LF,       // LF \n
        VT = 11,             // VT // Vertical tab \v
        VERTICAL_TAB = VT,   // Vertical tab \v
        FF = 12,             // FF // Form feed \f
        CR = 13,             // CR // Carriage return \r
        RETURN = CR,         // Carriage return \r
        SO = 14,             // SO // Shift out
        SI = 15,             // SI // Shift in
        DLE = 16,            // DLE // Data line escape
        DC1 = 17,            // DC1 // Device control 1
        DC2 = 18,            // DC2 // Device control 2
        DC3 = 19,            // DC3 // Device control 3
        DC4 = 20,            // DC4 // Device control 4
        NAK = 21,            // NAK // Negative ack.
        SYN = 22,            // SYN // Synchronous idle
        ETB = 23,            // ETB // End of block
        CANCEL = 24,         // CAN // Cancel
        EM = 25,             // EM // End of medium
        SUB = 26,            // SUB // Substitute
        ESC = 27,            // ESC // Escape
        ESCAPE = ESC,        // Escape
        FS = 28,             // FS // File separator
        GS = 29,             // GS // Group separator
        RS = 30,             // RS // Record separator
        US = 31,             // US // Unit separator

        SPACE = 32,              // Space
        EXCLAIM = 33,            // !
        QUOTEDBL = 34,           // " 	&quot;
        QUOTE_DOUBLE = QUOTEDBL, // "
        HASH = 35,               // #
        DOLLAR = 36,             // $
        PERCENT = 37,            // %
        AMPERSAND = 38,          // & &amp;
        QUOTE = 39,              // '
        QUOTE_SINGLE = QUOTE,    // '
        LEFTPAREN = 40,          // (
        RIGHTPAREN = 41,         // )
        ASTERISK = 42,           // *
        PLUS = 43,               // +
        COMMA = 44,              // ,
        MINUS = 45,              // -
        PERIOD = 46,             // .
        SLASH = 47,              // /
        N0 = 48,                 // 0
        N1 = 49,                 // 1
        N2 = 50,                 // 2
        N3 = 51,                 // 3
        N4 = 52,                 // 4
        N5 = 53,                 // 5
        N6 = 54,                 // 6
        N7 = 55,                 // 7
        N8 = 56,                 // 8
        N9 = 57,                 // 9
        COLON = 58,              // :
        SEMICOLON = 59,          // ;
        LESS = 60,               // < 	&lt;
        EQUALS = 61,             // =
        GREATER = 62,            // > 	&gt;
        QUESTION = 63,           // ?
        AT = 64,                 // @
        // 65 - 90 - uppercase letters

        LEFTBRACKET = 91,   // [
        BACKSLASH = 92,     // Backslash
        RIGHTBRACKET = 93,  // ]
        CARET = 94,         // ^
        UNDERSCORE = 95,    // _
        BACKQUOTE = 96,     // `
        A = 97,             // a
        B = 98,             // b
        C = 99,             // c
        D = 100,            // d
        E = 101,            // e
        F = 102,            // f
        G = 103,            // g
        H = 104,            // h
        I = 105,            // i
        J = 106,            // j
        K = 107,            // k
        L = 108,            // l
        M = 109,            // m
        N = 110,            // n
        O = 111,            // o
        P = 112,            // p
        Q = 113,            // q
        R = 114,            // r
        S = 115,            // s
        T = 116,            // t
        U = 117,            // u
        V = 118,            // v
        W = 119,            // w
        X = 120,            // x
        Y = 121,            // y
        Z = 122,            // z
        LEFTBRACE = 123,    // {
        VERTICAL_BAR = 124, // |
        RIGHTBRACE = 125,   // }
        TILDE = 126,        // ~
        DELETE = 127,       // Delete

        CAPSLOCK,

        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,

        F13,
        F14,
        F15,
        F16,
        F17,
        F18,
        F19,
        F20,
        F21,
        F22,
        F23,
        F24,

        PRINTSCREEN,
        SCROLL_LOCK,
        PAUSE,
        INSERT,
        HOME,
        END,
        PAGEUP,
        PAGEDOWN,
        RIGHT,
        LEFT,
        DOWN,
        UP,

        NUM_DIVIDE,
        NUM_MULTIPLY,
        NUM_MINUS,
        NUM_PLUS,
        NUM_ENTER,
        NUM_0,
        NUM_1,
        NUM_2,
        NUM_3,
        NUM_4,
        NUM_5,
        NUM_6,
        NUM_7,
        NUM_8,
        NUM_9,
        NUM_PERIOD,
        NUM_EQUALS,
        NUM_COMMA,

        LCTRL,
        LSHIFT,
        LALT,
        LGUI,
        LWINDOWS = LGUI,
        LAPPLE = LGUI,

        RCTRL,
        RSHIFT,
        RALT,
        RGUI,
        RWINDOWS = RGUI,
        RAPPLE = RGUI,

        EXECUTE,
        HELP,
        MENU,
        SELECT,
        STOP,
        CLEAR,

        APPLICATION,           // the Application / Compose / Context Menu (Windows) key
        CONTEXT = APPLICATION, // the Application / Compose / Context Menu (Windows) key
        APP_HOME,
        APP_BACK,

        VOLUMEUP,
        VOLUMEDOWN,

        RESERVED1,
        RESERVED2,
        RESERVED3,
        RESERVED4,

        NUM_VIRTUAL_KEYS
    };

    enum class KeyMod
    {
        NONE = 0x0000,

        LSHIFT = 0x0001,
        RSHIFT = 0x0002,

        LCTRL = 0x0040,
        RCTRL = 0x0080,

        LALT = 0x0100,
        RALT = 0x0200,

        LGUI = 0x0400,
        RGUI = 0x0800,

        NUM = 0x1000,
        CAPS = 0x2000,
        MODE = 0x4000,

        RESERVED = 0x8000
    };

    ENUM_FLAGS(KeyMod);
} //> namespace event
#endif //> FG_INC_KEYBOARD