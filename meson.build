project('cinepi-gui', 'cpp', default_options : ['c_std=c++11', 'warning_level=2'])
add_project_arguments('-DIMGUI_IMPL_OPENGL_ES3', language: ['c', 'cpp'])  # Apply to both C and C++

imgui_dir = 'imgui'
shaders_dir = 'shaders'
source_files = files('main.cpp',
                     'Application.cpp',
                     'SharedContext.cpp',
                     'EglBuffers.cpp',
                     'Diagnostics.cpp',
                     'Page.cpp',
                     'Viewport.cpp',
                     'Overlays.cpp',
                     'Menus.cpp',
                     'ImgAsset.cpp',
                     join_paths(imgui_dir, 'imgui.cpp'),
                     join_paths(shaders_dir, 'Shader.cpp'),
                     join_paths(shaders_dir, 'ComputeShader.cpp'),
                     join_paths(imgui_dir, 'imgui_demo.cpp'),
                     join_paths(imgui_dir, 'imgui_draw.cpp'),
                     join_paths(imgui_dir, 'imgui_tables.cpp'),
                     join_paths(imgui_dir, 'imgui_widgets.cpp'),
                     join_paths(imgui_dir, 'backends/imgui_impl_sdl2.cpp'),
                     join_paths(imgui_dir, 'backends/imgui_impl_opengl3.cpp'))

# Dependency management
spdlog_dep = dependency('spdlog')
sdl2_dep = dependency('sdl2', method : 'pkg-config')
#epoxy_deps = dependency('epoxy', required : false)
egl_dep = dependency('egl')
drm_dep = dependency('libdrm')
libcamera_dep = dependency('libcamera', required : true)


# Platform-specific configurations
if host_machine.system() == 'linux'
    platform_deps = [dependency('gl')]
elif host_machine.system() == 'darwin'
    platform_deps = [dependency('opengl')]
else
    platform_deps = []
endif

# dependencies : [sdl2_dep, opengl_dep, egl_dep, drm_dep, spdlog_dep, libcamera_dep] + platform_deps,

# Executable
executable('cinepi-gui', source_files,
           include_directories : include_directories(imgui_dir, join_paths(imgui_dir, 'backends')),
           dependencies : [spdlog_dep, libcamera_dep, sdl2_dep, egl_dep, drm_dep] + platform_deps,
           install : true)
