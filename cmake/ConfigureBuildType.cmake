# Set the varibales indicating the type of release when building executables
# Set as default: Release build
if(NOT CMAKE_BUILD_TYPE)
	if(NOT RELEASE)
		set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
	else()
		set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
	endif()
elseif(CMAKE_BUILD_TYPE MATCHES "Debug")
	set(RELEASE OFF CACHE BOOL "Build release version" FORCE)
elseif(CMAKE_BUILD_TYPE MATCHES "Release")
	set(RELEASE ON CACHE BOOL "Build release version" FORCE)
elseif(NOT RELEASE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
else()
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

