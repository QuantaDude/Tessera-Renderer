cmake_minimum_required(VERSION 3.16)
SET(CMAKE_C_COMPILER clang)
SET(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# GLM options
set(GLM_ENABLE_CXX_20 ON)
set(GLM_ENABLE_SIMD_SSE2 ON)
set(GLM_ENABLE_SIMD_SSE3 ON)
set(GLM_ENABLE_SIMD_SSSE3 ON)
set(GLM_ENABLE_SIMD_SSE4_1 ON)
set(GLM_ENABLE_SIMD_SSE4_2 ON)
set(GLM_ENABLE_SIMD_AVX ON)
set(GLM_ENABLE_SIMD_AVX2 ON)


# set(VKINIT_IMPLEMENTATION ON)
# set(RGFW_EXPORT ON)
# set(RGFW_WAYLAND ON)
#! ! ! ! ! ! !
#set this to true to ship the game!
#basically this will change the RESOURCES_PATH to be the local path
#! ! ! ! ! ! !
#DELETE THE OUT FOLDER AFTER CHANGING THIS BECAUSE VISUAL STUDIO DOESN'T SEEM TO RECOGNIZE THIS CHANGE AND REBUILD!
#option(PRODUCTION_BUILD "Make this a production build" OFF)
#DELETE THE OUT FOLDER AFTER CHANGING THIS BECAUSE VISUAL STUDIO DOESN'T SEEM TO RECOGNIZE THIS CHANGE AND REBUILD!

# --------------------------------
# Keep compile_commands.json symlinked to project root
# --------------------------------
if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR) # skip in in-source builds
    add_custom_target(link_compile_commands ALL
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR}/compile_commands.json
            BYPRODUCTS ${CMAKE_SOURCE_DIR}/compile_commands.json
            COMMENT "Updating symlink: compile_commands.json → project root"
    )
endif()


if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_options(/arch:AVX2)
else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
endif()

# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)#link time optimization

if(MSVC)
    add_compile_options(/arch:AVX2)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-missing-field-initializers -Wno-gnu-array-member-paren-init")


    add_compile_options(-mavx2 -msimd128)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -flto -ftree-vectorize -march=native)
    endif()
endif()

project(TesseraRenderer LANGUAGES C CXX)

#add_subdirectory(thirdparty/stb_image)              # loading images
#add_subdirectory(thirdparty/stb_truetype)           # loading ttf files
#add_subdirectory(thirdparty/cglm-0.9.6)                    # math
# add_subdirectory(thirdparty/glm)
# add_subdirectory(thirdparty/RGFW-1.8.1)
add_subdirectory(thirdparty/imgui-docking)          # ui
#
#
# # MY_SOURCES is defined to be a list of all the source files for my game
# # DON'T ADD THE SOURCES BY HAND, they are already added with this macro
# file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
#
# add_executable("${CMAKE_PROJECT_NAME}")
# set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 23)
#
# target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC GLFW_INCLUDE_NONE=1)


# -------------------------
# Find system dependencies
# -------------------------

find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
find_program(WAYLAND_SCANNER wayland-scanner REQUIRED)

pkg_check_modules(WAYLAND REQUIRED
    wayland-client
    wayland-cursor
    wayland-egl
    xkbcommon
)

set(WAYLAND_PROTOCOLS
    stable/xdg-shell/xdg-shell
    unstable/xdg-decoration/xdg-decoration-unstable-v1
    staging/xdg-toplevel-icon/xdg-toplevel-icon-v1
    unstable/relative-pointer/relative-pointer-unstable-v1
    unstable/pointer-constraints/pointer-constraints-unstable-v1
    unstable/xdg-output/xdg-output-unstable-v1
    staging/pointer-warp/pointer-warp-v1
)

set(SYSTEM_WAYLAND /usr/share/wayland-protocols)
set(LOCAL_WAYLAND ${CMAKE_SOURCE_DIR}/thirdparty/rgfw/wayland)

set(GENERATED_WAYLAND_SOURCES)
set(GENERATED_WAYLAND_HEADERS)

foreach(PROTO ${WAYLAND_PROTOCOLS})

    # XML full path (system first, fallback to local)
    set(XML_SYSTEM "${SYSTEM_WAYLAND}/${PROTO}.xml")
    get_filename_component(PROTO_NAME ${PROTO} NAME)
    set(XML_LOCAL "${LOCAL_WAYLAND}/${PROTO_NAME}.xml")

    if(EXISTS ${XML_SYSTEM})
        set(XML_FILE ${XML_SYSTEM})
    else()
        set(XML_FILE ${XML_LOCAL})
    endif()

set(WAYLAND_GEN_DIR ${CMAKE_BINARY_DIR}/thirdparty/rgfw/wayland)
file(MAKE_DIRECTORY ${WAYLAND_GEN_DIR})

set(HEADER_FILE ${WAYLAND_GEN_DIR}/${PROTO_NAME}.h)
set(SOURCE_FILE ${WAYLAND_GEN_DIR}/${PROTO_NAME}.c)

    add_custom_command(
        OUTPUT ${HEADER_FILE}
        COMMAND ${WAYLAND_SCANNER} client-header
                ${XML_FILE}
                ${HEADER_FILE}
        DEPENDS ${XML_FILE}
    )

    add_custom_command(
        OUTPUT ${SOURCE_FILE}
        COMMAND ${WAYLAND_SCANNER} public-code
                ${XML_FILE}
                ${SOURCE_FILE}
        DEPENDS ${XML_FILE}
    )

    list(APPEND GENERATED_WAYLAND_HEADERS ${HEADER_FILE})
    list(APPEND GENERATED_WAYLAND_SOURCES ${SOURCE_FILE})

endforeach()

# -------------------------
# RGFW library
# -------------------------

add_library(rgfw STATIC
    thirdparty/rgfw/rgfw_impl.c
    ${GENERATED_WAYLAND_SOURCES}
    ${GENERATED_WAYLAND_HEADERS}
)
set_target_properties(rgfw PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/thirdparty/rgfw
)
set_target_properties(rgfw PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/thirdparty/rgfw
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/thirdparty/rgfw
)
target_include_directories(rgfw PUBLIC
    ${CMAKE_SOURCE_DIR}/thirdparty/rgfw
    ${WAYLAND_GEN_DIR}
)

# target_compile_definitions(rgfw PRIVATE
#     RGFW_IMPLEMENTATION
#     RGFW_OPENGL
#     RGFW_WAYLAND
# )
target_link_libraries(rgfw PUBLIC
    ${WAYLAND_LIBRARIES}
    OpenGL::GL
    EGL
    X11
    Xcursor
    Xrandr
    Xi
)

# -------------------------
# GLM (header-only)
# -------------------------

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE thirdparty/glm)

# -------------------------
# Your main executable
# -------------------------

file(GLOB_RECURSE SRC_FILES src/*.cpp)

add_executable("${CMAKE_PROJECT_NAME}" ${SRC_FILES})

target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE
    rgfw
    glm
    imgui
)

if(PRODUCTION_BUILD)
    # setup the ASSETS_PATH macro to be in the root folder of your exe
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="./resources/")
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=1)
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC DEVELOPLEMT_BUILD=0)
else()
    # This is useful to get an ASSETS_PATH in your IDE during development
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=0)
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC DEVELOPLEMT_BUILD=1)
endif()

target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES})

if(MSVC) # If using the VS compiler...
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC _CRT_SECURE_NO_WARNINGS)
    #add this line if you want to remove the console!
    #set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") #no console
endif()

target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/gameLayer")
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/platform")


# target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE
#         glm
#         #stb_image
#         #stb_truetype
#         rgfw
#         imgui
#         #SDL3::SDL3
#
# )
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
)


