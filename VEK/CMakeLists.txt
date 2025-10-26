# ==============================================================================
# VEK (Vantor Engine Kernel)
# Author: Lukas Rennhofer @2025
# ==============================================================================

cmake_minimum_required(VERSION 3.15)
project(VEK LANGUAGES CXX)

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

file(GLOB_RECURSE VEK_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/VEK/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/VEK/*.c"
)

# =========================
# Define library
# =========================

add_library(VEK STATIC ${VEK_SOURCES})

# Public include directory
target_include_directories(VEK
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
    # Expose a virtual include path so users can write #include <External/...>
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
# Link dependencies (optional)
# =========================

if(VEK_USE_OPENGL)
    find_package(OpenGL REQUIRED)
    target_link_libraries(VEK PUBLIC OpenGL::GL)
endif()

if(VEK_USE_VULKAN)
    find_package(Vulkan REQUIRED)
    target_link_libraries(VEK PUBLIC Vulkan::Vulkan)
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

# If an External folder exists, expose it to the build and install its headers
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/External")
    # If External contains a CMakeLists.txt, add it so any subtargets get configured
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/External/CMakeLists.txt")
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/External)
    endif()

    # Install any headers from External into the installed include tree under External/
    install(DIRECTORY External/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/External FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.inl" PATTERN "*.ipp")
endif()

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
