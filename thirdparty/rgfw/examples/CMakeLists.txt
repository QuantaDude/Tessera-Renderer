function(add_rgfw_example name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE rgfw)
endfunction()

# ------------------------------------------------------------
# Simple examples
# ------------------------------------------------------------
set(SIMPLE_EXAMPLES
        callbacks
        flags
        gears
        monitor
        gl33_ctx
        smooth-resize
        multi-window
)

foreach(ex ${SIMPLE_EXAMPLES})
    add_rgfw_example(
            ${ex}
            ${ex}/${ex}.c
    )
endforeach()

# ------------------------------------------------------------
# Special examples
# ------------------------------------------------------------
#add_rgfw_example(icons icons/icons.c)
add_rgfw_example(gamepad gamepad/gamepad.c)
#add_rgfw_example(silk silk/silk.c)
add_rgfw_example(camera first-person-camera/camera.c)
add_rgfw_example(gl33 gl33/gl33.c)
add_rgfw_example(pgl portableGL/pgl.c)

# microui
add_executable(microui_demo
        microui_demo/microui_demo.c
        microui_demo/microui.c
)
target_link_libraries(microui_demo PRIVATE rgfw)

# GLES
if(NOT RGFW_NO_GLES)
    add_rgfw_example(gles2 gles2/gles2.c)
    target_link_libraries(gles2 PRIVATE EGL)
endif()

# OSMesa
if(NOT RGFW_NO_OSMESA)
    add_rgfw_example(osmesa_demo osmesa_demo/osmesa_demo.c)
    target_link_libraries(osmesa_demo PRIVATE OSMesa)
endif()

# Vulkan
if(NOT RGFW_NO_VULKAN)
    add_rgfw_example(vk10 vk10/vk10.c)
endif()

# DX11
if(WIN32)
    add_executable(dx11 dx11/dx11.c)
    target_link_libraries(dx11 PRIVATE
            rgfw d3d11 dxgi d3dcompiler uuid gdi32
    )
endif()

# Metal
if(APPLE)
    add_executable(metal
            metal/metal.c
            ../RGFW.c
    )
    target_compile_options(metal PRIVATE -x objective-c)
    target_link_libraries(metal PRIVATE
            rgfw
            "-framework Metal"
            "-framework QuartzCore"
    )
endif()
