# ==============================================================================
# VEK (Vantor Engine Kernel)
# Author: Lukas Rennhofer @2025
# ==============================================================================

cmake_minimum_required(VERSION 3.15)
project(VEK LANGUAGES C CXX)

# =========================
# Options for flexibility
# =========================

# Choose graphics backend
option(VEK_USE_OPENGL "Use OpenGL backend" ON)
option(VEK_USE_VULKAN "Use Vulkan backend" OFF)

# Choose build type if not set
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# =========================
# Platform detection
# =========================

if(WIN32)
    add_compile_definitions(VEK_WINDOWS)
elseif(UNIX)
    add_compile_definitions(VEK_LINUX)
else()
    message(FATAL_ERROR "Unsupported platform!")
endif()

# =========================
# Backend selection
# =========================

if(VEK_USE_OPENGL)
    add_compile_definitions(VEK_OPENGL)
    message(STATUS "VEK: Building with OpenGL backend")
elseif(VEK_USE_VULKAN)
    add_compile_definitions(VEK_VULKAN)
    message(STATUS "VEK: Building with Vulkan backend")
else()
    message(FATAL_ERROR "No rendering backend selected. Enable VEK_USE_OPENGL or VEK_USE_VULKAN.")
endif()

# =========================
# Collect sources
# =========================

# Common sources
file(GLOB_RECURSE VEK_COMMON_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/VEK/Core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/VEK/Core/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/VEK/Math/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/VEK/Math/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/VEK/Integration/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/VEK/Integration/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/VEK/Network/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/VEK/Network/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/VEK/RHI/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/VEK/RHI/*.c"
)

# Platform-specific sources
if(WIN32)
    file(GLOB_RECURSE VEK_PLATFORM_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/VEK/Platform/VPL_Platform.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/VEK/Platform/Impl/Windows/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/VEK/Platform/Impl/Windows/*.c"
    )
elseif(UNIX)
    file(GLOB_RECURSE VEK_PLATFORM_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/VEK/Platform/VPL_Platform.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/VEK/Platform/Impl/Linux/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/VEK/Platform/Impl/Linux/*.c"
    )
endif()

# Combine all sources
set(VEK_SOURCES ${VEK_COMMON_SOURCES} ${VEK_PLATFORM_SOURCES})

# Add GLAD sources directly if using OpenGL
if(VEK_USE_OPENGL)
    list(APPEND VEK_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/External/glad/glad.c"
    )
endif()

# =========================
# Define library
# =========================

add_library(VEK STATIC ${VEK_SOURCES})

# Public include directory
target_include_directories(VEK
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/External>
        $<INSTALL_INTERFACE:include>
)

# =========================
# Compiler settings
# =========================

target_compile_features(VEK PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(VEK PRIVATE /W4 /permissive-)
else()
    target_compile_options(VEK PRIVATE -Wall -Wextra -Wpedantic)
endif()

# =========================
# Link dependencies
# =========================

if(VEK_USE_OPENGL)
    find_package(OpenGL REQUIRED)
    target_link_libraries(VEK PUBLIC OpenGL::GL)
endif()

if(VEK_USE_VULKAN)
    find_package(Vulkan REQUIRED)
    target_link_libraries(VEK PUBLIC Vulkan::Vulkan)
endif()

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    # Linux requires X11 libraries
    find_package(X11 REQUIRED)
    target_link_libraries(VEK PUBLIC ${X11_LIBRARIES})
    if(VEK_USE_OPENGL)
        # GLX for OpenGL on X11
        target_link_libraries(VEK PUBLIC GL)
    endif()
elseif(WIN32)
    # Windows platform libraries
    target_link_libraries(VEK PUBLIC gdi32 user32 dinput8 xinput dxguid)
    if(VEK_USE_OPENGL)
        target_link_libraries(VEK PUBLIC opengl32)
    endif()
endif()

# If an External folder exists, expose it to the build and install its headers
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/External")
    # Install any headers from External into the installed include tree under External/
    install(DIRECTORY External/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/External FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.inl" PATTERN "*.ipp")
endif()

# =========================
# Installation rules (optional)
# =========================

include(GNUInstallDirs)

install(TARGETS VEK
    EXPORT VEKTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY Include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT VEKTargets
    FILE VEKTargets.cmake
    NAMESPACE VEK::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VEK
)

# =========================
# Summary
# =========================

message(STATUS "-------------------------------------")
message(STATUS "Platform:     ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build type:   ${CMAKE_BUILD_TYPE}")
message(STATUS "OpenGL:       ${VEK_USE_OPENGL}")
message(STATUS "Vulkan:       ${VEK_USE_VULKAN}")
message(STATUS "-------------------------------------")
