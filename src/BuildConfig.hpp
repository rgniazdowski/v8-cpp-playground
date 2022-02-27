//
// The build configuration file is used to configure the whole engine at compile
// time. The whole idea of using macros to add or remove big chunks of code is to
// generate different executables depending on the target platform, build platform
// and libraries availability on the current system.
//
#pragma once
#ifndef FG_INC_BUILD_CONFIG
#define FG_INC_BUILD_CONFIG

#if defined(USE_PRE_CONFIG)
//#include <preConfiguration.h>
#endif /* defined(USE_PRE_CONFIG) */

#define FG_VERBOSE_LVL_NONE 0   // report only critical errors - no warnings, info, status etc
#define FG_VERBOSE_LVL_LOW 1    // report only errors
#define FG_VERBOSE_LVL_MEDIUM 2 // report only errors and warnings (+info, success)
#define FG_VERBOSE_LVL_HIGH 3   // report everything: Info, Status, Warning, Debug, Errors and others

/*
    FG_USING_OPENGL_ES               //  Is the OpenGL ES used?
    FG_USING_OPENGL                  //  Is the plain OpenGL used in this build?
    FG_USING_OPENGL_GLU              //  Is the GLU library used in this build?
    FG_USING_OPENGL_GLUT             //  Is the GLUT library used?
    FG_USING_OPENGL_GLEW             //  Is the GLEW library used?
    FG_USING_GLM                     //  Is the OpenGL Mathematics library (GLM) used?
    FG_USING_SDL2                    //  Is the latest SDL2 used in this build?
    FG_USING_SDL_MIXER               //  Is the latest SDL/SDL2 mixer library used in this build?
    FG_USING_OPENAL                  //  Is the OpenAL library used (for sound system) in this build?
    FG_USING_EGL                     //  Is the EGL library used in this build?
    FG_USING_SENSORS                 //  Are sensors used in this build?
    FG_USING_AUDIO                   //  Is audio used at all?
    FG_USING_THREADS                 //  Are threads used at all?
    FG_USING_DIRECTX                 //  Is the DirectX (any version) used in this build?
    FG_USING_DIRECTSOUND             //  Is the DirectSound used?
    FG_USING_DPI_INFO                //  Is the DPI Info extension used?
    FG_USING_TINYXML                 //  Is the tinyxml library used in this build?
    FG_USING_LUA_PLUS                //  Is the enhanced Lua C API used?
    FG_USING_LUA                     //  Is the original Lua C API used?
    FG_USING_BULLET                  //  Is the Bullet (physics engine) library used in this build?
    FG_USING_ASSIMP                  //  Is the Open Asset Import Library used in this build?
    FG_USING_PLATFORM_WINDOWS        //  Is the target platform Windows in this build?
    FG_USING_PLATFORM_LINUX          //  Is the target platform Linux?
    FG_USING_PLATFORM_ANDROID        //  Is the target platform Android?
    FG_USING_PLATFORM_IOS            //  Is the target platform iOS?
    FG_USING_PLATFORM_MOBILE         //  Is the target platform mobile? (Android/iOS/Other)
    FG_USING_PLATFORM_MACOSX         //  Is the target platform MACOS/X
    FG_USING_CYGWIN                  //  Is Cygwin used for current build?
    FG_USING_MINGW                   //  Is MinGW used for current build?
    FG_USING_VISUAL_STUDIO           //  Does the build system run under Visual Studio (any version)?
    FG_USING_QT                      //  Does the build system run under QtCreator (any version)?
    FG_USING_PLUGINS                 //  Is Plugin Subsystem used in this build?
    FG_USING_WXWIDGETS               //  Is wxWidgets GUI library used in this build?
    FG_SUPPORT_WXWIDGETS             //  Is wxWidgets enabled/supported in this build?
 */

/********************** CURRENT TARGET PLATFORM / BUILD SYSTEM ****************/

#if defined(__gnu_linux__) || defined(__linux__) || defined(linux) || defined(__linux)
#define FG_USING_PLATFORM_LINUX
#endif

#if defined(ANDROID) || defined(__ANDROID__)
#undef ANDROID
#undef __ANDROID__
#define ANROID 1
#define __ANDROID__ 1
#define FG_USING_PLATFORM_ANDROID
#undef FG_USING_PLATFORM_LINUX
#undef linux
#undef LINUX
#endif

#if defined(__CYGWIN__)
#define FG_USING_CYGWIN
#endif

#if defined(__MINGW__) || defined(__MINGW32__) || defined(__MINGW64__)
#define FG_USING_MINGW
#endif

#if (defined(__APPLE__) && defined(__MACH__)) || defined(Macintosh) || defined(macintosh)
#define FG_USING_PLATFORM_MACOSX
#endif

#if defined(__APPLE__) && defined(__IPHONEOS__)
#undef FG_USING_PLATFORM_MACOSX
#define FG_USING_PLATFORM_IOS
#endif

#if (defined __WINDOWS__ || defined __WIN32__ || defined _WIN32 || defined _WIN64 || defined __TOS_WIN__ || defined WIN32 || defined win32)
#define FG_USING_PLATFORM_WINDOWS
#endif

#if defined(_MSC_VER)
#define FG_USING_VISUAL_STUDIO
#endif

#if defined(_DEBUG) || defined(DEBUG)
#undef DEBUG
#define DEBUG 1
#undef FG_DEBUG
#define FG_DEBUG 1
#endif

#if defined(FG_DEBUG)
#ifndef FG_VERBOSE_LEVEL
#define FG_VERBOSE_LEVEL FG_VERBOSE_LVL_HIGH
#define FG_VERBOSE
#endif
#else
#ifndef FG_VERBOSE_LEVEL
#define FG_VERBOSE_LEVEL FG_VERBOSE_LVL_LOW
#define FG_VERBOSE
#endif
#endif

#if defined(_RELEASE) || defined(__release) || !defined(FG_DEBUG)
#undef FG_RELEASE
#define FG_RELEASE 1
#undef FG_DEBUG
#undef _DEBUG
#undef DEBUG
#endif

/********************************** DEFAULTS **********************************/

#if defined(FG_DEFAULTS)
#undef FG_USING_LUA_PLUS
#define FG_USING_LUA_PLUS

#undef FG_USING_GLM
#define FG_USING_GLM

#undef FG_USING_SDL2
#define FG_USING_SDL2

#undef FG_USING_THREADS
#define FG_USING_THREADS

#undef FG_USING_TINYXML
#define FG_USING_TINYXML

#undef TIXML_USE_STL
#define TIXML_USE_STL

#if !defined(FG_NO_PHYSICS)
#undef FG_USING_BULLET
#define FG_USING_BULLET
#endif

#if defined(FG_USING_PLATFORM_WINDOWS) || defined(FG_USING_PLATFORM_LINUX) || defined(FG_USING_PLATFORM_MACOSX)
#define FG_USING_GLEW
#define FG_USING_OPENGL
#define FG_USING_OPENGL_GLEW
#endif /* PC PLATFORMS */

#if defined(FG_USING_PLATFORM_ANDROID) || defined(FG_USING_PLATFORM_IOS)
#define FG_USING_OPENGL_ES
#endif /* MOBILE PLATFORMS */
#endif /* FG_DEFAULTS */

/********************* SUBSYSTEM / INPUT / AUDIO SUPPORT **********************/

/****************************** GRAPHICS SPECIFIC *****************************/

// Under Linux - use plain GL
#if defined(FG_USING_PLATFORM_LINUX)
#if !defined(FG_USING_OPENGL)
#define FG_USING_OPENGL
#endif
#endif

////////////////// ANDROID DEFAULTS ////////////////////////////////////////////

// Under Android - use OGLES2 - 2.0 is default - 1.0 unused/unsupported
#if defined(FG_USING_PLATFORM_ANDROID)
#undef FG_USING_OPENGL_ES
#define FG_USING_OPENGL_ES
#undef FG_USING_EGL
#endif

// Under Android - almost certain that build using Android SDK
// Usage only with SDL2
#if defined(FG_USING_PLATFORM_ANDROID)
#undef FG_USING_SDL2
#define FG_USING_SDL2
#endif

// Under android - always use lua plus - hard coded
#if defined(FG_USING_PLATFORM_ANDROID)
#undef FG_USING_LUA_PLUS
#define FG_USING_LUA_PLUS
#endif

// Android
#if defined(FG_USING_PLATFORM_ANDROID)
#define FG_USING_TINYXML
#undef TIXML_USE_STL // When using Android - do not use STL in TinyXML
#endif

/*********************** EXTENSIONS / PLUGINS SUPPORT *************************/

// Now this is just a general define for threads, I think it wont be used as
// more specific defines will occur in the future
// (depending on the OS and available API) // #FIXME
//#define FG_USING_THREADS

/* Use TinyXML even on Linux */
#if defined(FG_USING_PLATFORM_LINUX)
#if !defined(FG_USING_TINYXML)
#define FG_USING_TINYXML
#endif
#endif

/************************ CROSS PLATFORM COMPATIBILITY ************************/

// Always (as default) use OpenGL Mathematics library
// Will later check if it's possible to use it with DX11 (some hacks required)
#ifndef FG_USING_GLM
#define FG_USING_GLM
#endif
// As default force radians as standard parameter type for angles
#ifdef FG_USING_GLM
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#if !defined(GLM_MESSAGES) && defined(FG_DEBUG) && defined(FG_MESSAGES)
#define GLM_MESSAGES
#endif
#endif

/* Force TinyXml to use STL */
#if defined(FG_USING_TINYXML)
#if !defined(TIXML_USE_STL) && !defined(FG_USING_PLATFORM_ANDROID)
#define TIXML_USE_STL
#endif
#endif

/* No profiling in Bullet physics engine */
#if defined(FG_USING_BULLET)
#if defined(FG_PHYSICS_DOUBLE_PRECISION)
#undef BT_USE_DOUBLE_PRECISION
#define BT_USE_DOUBLE_PRECISION 1
#endif
// if btScalar was 'double' instead of 'float' there would be a problem
// with data copying from btTransform to fg::Matrix4f
#undef BT_NO_PROFILE
#define BT_NO_PROFILE 1
#endif

/****************************** AUDIO / SOUND FIX *****************************/

#if defined(FG_USING_SDL2)
#if !defined(FG_USING_AUDIO)
#define FG_USING_AUDIO
#define FG_USING_SDL_MIXER
#endif
#endif

/*********************** OTHER DEFINITIONS / STRUCTURES ***********************/

#if defined(FG_USING_PLATFORM_ANDROID) || defined(FG_USING_PLATFORM_IOS)
#undef FG_USING_PLATFORM_MOBILE
#define FG_USING_PLATFORM_MOBILE
#endif
/*
namespace fg
{
    // Special structure for runtime access of the build configuration variables
    struct BuildConfig
    {
        const bool isDebug;
        const bool isVerbose;
        const int verboseLevel;
        const bool usingOpenGLES;       //    Is the OpenGL ES used?
        const bool usingOpenGL;         //      Is the plain OpenGL used in this build?
        const bool usingOpenGL_GLU;     //  Is the GLU library used in this build?
        const bool usingOpenGL_GLUT;    // Is the GLUT library used? (freeglut) (DEPRECATED)
        const bool usingOpenGL_GLEW;    // Is the GLEW library used?
        const bool usingGLM;            //         Is the OpenGL Mathematics library (GLM) used? (OPTIONAL)
        const bool usingSDL2;           //        Is SDL 2.0 library used in this build?
        const bool usingSDLMixer;       //    Is the latest SDL/SDL2 mixer library used in this build?
        const bool usingOpenAL;         //      Is the OpenAL library used (for sound system) in this build (DEPRECATED)
        const bool usingEGL;            //         Is the EGL library used in this build?
        const bool usingSensors;        //     Are sensors used in this build?
        const bool usingAudio;          //       Is audio used at all?
        const bool usingThreads;        //     Are threads used at all?
        const bool usingDirectX;        //     Is the DirectX (any version) used in this build?
        const bool usingDirectSound;    // Is the DirectSound used?
        const bool usingDPIInfo;        //     Is the DPI Info extension used?
        const bool usingTinyXML;        //     Is the tinyxml library used in this build?
        const bool usingLuaPlus;        //     Is the enhanced Lua C API used?
        const bool usingLua;            //         Is the original Lua C API used?
        const bool usingBullet;         //      Is the Bullet (physics engine) library used in this build?
        const bool usingAssimp;         //      Is the Open Asset Import Library used in this build?
        const bool usingSteamworks;     //  Is Steamworks API used in this build?
        const bool usingOpenVR;         //      Is OpenVR used in this build?
        const bool usingOgre3D;         //      Is Ogre3D gfx engine used in this build? (any version)
        const bool usingOgre3D_1_10;    // Is Ogre3D gfx version 1.10 used in this build?
        const bool usingOgre3D_1_9;     //  Is Ogre3D gfx version 1.9 used in this build?
        const bool usingOgre3D_2_0;     //  Is Ogre3D gfx version 2.0 used in this build? (default)
        const bool usingOgre3D_2_1;     //  Is Ogre3D gfx version 2.1 used in this build?
        const bool usingFlexiGraphics;  // Is FlexiGame gfx rendering used in this build?
        const bool isPlatformWindows;   // Is the target platform Windows in this build?
        const bool isPlatformLinux;     //  Is the target platform Linux?
        const bool isPlatformAndroid;   // Is the target platform Android?
        const bool isPlatformIOS;       //    Is the target platform iOS?
        const bool isPlatformMobile;    // Is the target platform mobile?
        const bool isPlatformOSX;       // Is the target platform MAC OSX
        const bool usingCygwin;         //      Is Cygwin used for current build?
        const bool usingMinGW;          //       Is MinGW used for current build?
        const bool usingVisualStudio;   // Does the build system run under Visual Studio (any version)?
        const bool usingQT;             //          Does the build system run under QtCreator (any version)?
        const bool usingPlugins;        //     Is Plugin Subsystem used in this build?
        const bool using_wxWidgets;     //  Is wxWidgets GUI library used in this build?
        const bool usingDynamicLoading; // Is dynamic DLL loading used in this build?
        const bool support_wxWidgets;   // Is wxWidgets enabled/supported in this build?
        const bool support_OpenVR;      //   Is OpenVR lib enabled/supported in this build?
        const bool support_Ogre3D;      //   Is Ogre3D lib supported in this build?
        const unsigned int version;
        const unsigned short versionMajor;
        const unsigned short versionMinor;
        const unsigned short versionRevision;
        const char *buildDate;
        const char *buildTime;
        const char *buildVersionText;
        const char *packageName;
        const char *packageFullText;
        const char *assimpSupportedExtensions;
        int empty;
    }; // struct BuildConfig

    extern struct BuildConfig g_BuildConfig;
} // namespace fg

#define FG_BUILD ::fg::g_BuildConfig
#define FG_BUILD_CONFIG ::fg::g_BuildConfig
*/
#define FG_BUILD_DATE __DATE__
#define FG_BUILD_TIME __TIME__

#define FG_XSTR(S) FG_STR(S)
#define FG_STR(S) #S

/* Major part of the version */
#define FG_BUILD_VERSION_MAJ 0
/* Minor part of the version */
#define FG_BUILD_VERSION_MIN 10
/* Revision part */
#define FG_BUILD_VERSION_REV 0

#define FG_BUILD_VERSION_SUFFIX "unstable"

/* Build version number [Major]*.[Minor].[Revision] */
#define FG_BUILD_VERSION ((FG_BUILD_VERSION_MAJ << 16) | (FG_BUILD_VERSION_MIN << 8) | FG_BUILD_VERSION_REV)
/* Stringified version number */
#define FG_BUILD_VERSION_STR FG_XSTR(FG_BUILD_VERSION)
/* Version text, parts separated by . */
#define FG_BUILD_VERSION_TEXT     \
    FG_XSTR(FG_BUILD_VERSION_MAJ) \
    "." FG_XSTR(FG_BUILD_VERSION_MIN) "." FG_XSTR(FG_BUILD_VERSION_REV)

/* Package name - name of the engine */
#define FG_PACKAGE_NAME "FlexiGame"
/* Code name of this version */
#define FG_PROJECT_CODE_NAME "Infinium"
#define FG_PREFIX "fg"

/* Package name with version and codename */
#define FG_PACKAGE_FULL_TEXT FG_PACKAGE_NAME " v" FG_BUILD_VERSION_TEXT " codename: " FG_PROJECT_CODE_NAME

/* Full name of the project */
#define FG_FULL_NAME "Flexible Game Engine"
#define FG_PACKAGE_ALIAS "FG:E"

/* Copyright text */
#define FG_COPYRIGHT "Copyright (C) Radoslaw Gniazdowski. All rights reserved."
/* License type */
#define FG_LICENSE "New BSD License (3-clause license) and others"
/* Home page address */
#define FG_HOMEPAGE "https://varia.blog/"

#endif //> FG_INC_BUILD_CONFIG
/******************************** END MAIN BUILD CONFIG *********************************/
