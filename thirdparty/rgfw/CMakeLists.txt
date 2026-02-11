cmake_minimum_required(VERSION 3.16)
project(RGFW C)

# ============================================================
# Options
# ============================================================

option(RGFW_WAYLAND "Enable Wayland support" OFF)
set(WAYLAND ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ============================================================
# RGFW implementation source (RGFW.h -> RGFW.c)
# ============================================================

set(RGFW_IMPL ${CMAKE_CURRENT_BINARY_DIR}/RGFW.c)

add_custom_command(
    OUTPUT ${RGFW_IMPL}
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/RGFW.h
            ${RGFW_IMPL}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/RGFW.h
    COMMENT "Generating RGFW.c from RGFW.h"
)

# ============================================================
# Create library FIRST (CRITICAL)
# ============================================================

add_library(rgfw STATIC
    ${RGFW_IMPL}
)

# ============================================================
# Wayland support
# ============================================================

if (RGFW_WAYLAND)
    message(STATUS "RGFW: Wayland enabled")

    # Ask wayland.mk what files it generates
    execute_process(
        COMMAND ${CMAKE_MAKE_PROGRAM} -f wayland.mk sources -s
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE WAYLAND_SOURCES_RAW
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    separate_arguments(WAYLAND_SOURCES UNIX_COMMAND "${WAYLAND_SOURCES_RAW}")
list(TRANSFORM WAYLAND_SOURCES
         PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
    # Generate protocol sources at build time
    add_custom_command(
        OUTPUT ${WAYLAND_SOURCES}
        COMMAND ${CMAKE_MAKE_PROGRAM} -f wayland.mk
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating Wayland protocol sources"
    )

    # Compile Wayland protocol sources INTO rgfw
  target_sources(rgfw PRIVATE 
        ${WAYLAND_SOURCES}
    )

    # Required compile define (matches Makefile)
    target_compile_definitions(rgfw PRIVATE RGFW_WAYLAND)

    # Public: propagated to final executable
    target_link_libraries(rgfw PUBLIC
        GL
        EGL
    )

    # Private: internal implementation details
    target_link_libraries(rgfw PRIVATE
        wayland-client
        wayland-cursor
        wayland-egl
        xkbcommon
        m
    )
endif()

# ============================================================
# Include paths
# ============================================================

# Needed so xdg-shell.h and friends are visible to dependents
target_include_directories(rgfw PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ============================================================
# Output
# ============================================================

set_target_properties(rgfw PROPERTIES
    OUTPUT_NAME rgfw
)
