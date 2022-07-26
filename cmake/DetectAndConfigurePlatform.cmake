if (NOT PLATFORM)
message(STATUS "Dynamically determining native platform...")
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(OS "linux")
    if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64" OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "AMD64")
        set(ARCHITECTURE "x86_64")
        set(PLATFORM "linux-x86-64")
    elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_32" OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "i[36]86")
        set(ARCHITECTURE "x86_32")
        set(PLATFORM "linux-x86-32")
    else () # default to linux-x86_64
        set(ARCHITECTURE "x86_64")
        set(PLATFORM "linux-x86-64")
    endif()
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(OS "windows")
    # On Windows, rely on the generator for the build bitness
    if(CMAKE_GENERATOR MATCHES "Win64" OR CMAKE_GENERATOR MATCHES "x64" OR CMAKE_GENERATOR_PLATFORM MATCHES "x64")
        set(ARCHITECTURE "x86_64")
        set(PLATFORM "win64")
    else()
        set(ARCHITECTURE "x86_32")
        set(PLATFORM "win32")
    endif()
endif ()
endif ()

if("${OS}" STREQUAL "" OR "${ARCHITECTURE}" STREQUAL "")
message(FATAL_ERROR "Unsupported platform: ${PLATFORM}")
endif()