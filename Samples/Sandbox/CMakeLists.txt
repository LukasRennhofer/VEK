# ==============================================================================
# Sandbox - Example Project using VEK (Vantor Engine Kernel)
# Author: Lukas Rennhofer @2025
# ==============================================================================

cmake_minimum_required(VERSION 3.15)
project(Sandbox LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =========================
# Add VEK
# =========================

set(VEK_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../VEK")
set(VEK_BINARY_DIR "${CMAKE_BINARY_DIR}/VEK_BUILD")

add_subdirectory("${VEK_SOURCE_DIR}" "${VEK_BINARY_DIR}")

# =========================
# Sample sources
# =========================
file(GLOB_RECURSE SANDBOX_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(Sandbox ${SANDBOX_SOURCES})

# Include sources
target_include_directories(Sandbox PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Link to VEK
target_link_libraries(Sandbox PRIVATE VEK)

# =========================
# Summary
# =========================
message(STATUS "-------------------------------------")
message(STATUS "Building Sandbox")
message(STATUS "Platform:   ${CMAKE_SYSTEM_NAME}")
message(STATUS "-------------------------------------")
