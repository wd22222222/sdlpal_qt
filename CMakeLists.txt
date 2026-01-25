cmake_minimum_required(VERSION 3.14)

# 设置编译器静态开关
set(USING_STATIC 1)

# 设置SDL3开关
set(USING_SDL3 1)

#设置ogg vorbis开关 
set(USING_OGG 1)

#step 1: 设置vcpkg工具链
if(USING_STATIC)
    set(VCPKG_ROOT "d:/d/vcpkg" CACHE PATH "Path to vcpkg root")
    set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "vcpkg target triplet (static)")
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
else()
    set(VCPKG_ROOT "d:/d/vcpkg" CACHE PATH "Path to vcpkg root")
    set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "")
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

#step 2: 项目配置
# 调试编译器信息
message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_C_COMPILER_ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "MSVC: ${MSVC}")

# QT 工具
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 输出运行文件名
project(sdlpal_qt LANGUAGES CXX C)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#step 3: 配置编译选项
if(USING_STATIC)
# 设置静态运行时库 - 兼容不同编译器
    if(MSVC)
        message(STATUS "Using MSVC compiler")
        # Prefer CMake's runtime selection if policy available
        if(POLICY CMP0091)
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        else()
            # Fallback for older CMake: add compile options per-config
            add_compile_options($<$<CONFIG:Debug>:/MTd> $<$<CONFIG:Release>:/MT> $<$<CONFIG:RelWithDebInfo>:/MT> $<$<CONFIG:MinSizeRel>:/MT>)
        endif()
    elseif(MINGW)
        message(STATUS "Using MinGW compiler")
        # MinGW 的静态链接设置
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -static-libgcc")
    else()
        message(STATUS "Using other compiler: ${CMAKE_CXX_COMPILER_ID}")
    endif()
endif()
 
#step 4: 查找和链接库

# 添加路径
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 定义带值的宏
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_IN_CMAKE=1")
add_definitions(-DPROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
# 优先使用 config 模式的包，以便使用 vcpkg 导出的目标
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)


# 找到Qt库
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# remove previous set_directory_properties usage; set USING_SDL3 based on detection
if(USING_SDL3)
    find_package(SDL3 QUIET)
endif()

if(SDL3_FOUND)
    message(STATUS "Found SDL3: ${SDL3_VERSION}")
    add_compile_definitions(USING_SDL3=1)
    set(SDL_FOUND TRUE)
    set(SDL_VERSION_MAJOR 3)
else()
    add_compile_definitions(USING_SDL3=0)
    find_package(SDL2)
    if(SDL2_FOUND)
        message(STATUS "Found SDL2: ${SDL2_VERSION}")
        message(STATUS "SDL2 DIR: ${SDL_INCLUDE_DIRS}")
        set(SDL_FOUND TRUE)
        set(SDL_VERSION_MAJOR 2)

    # 函数：过滤包含目录,使用#include sdl.h 非法
    function(filter_include_dirs input_list output_var exclude_pattern)
        set(result_list)
        foreach(dir ${input_list})
        # 检查是否匹配排除模式
        string(FIND "${dir}" "${exclude_pattern}" pattern_found)
        if(pattern_found EQUAL -1)
            list(APPEND result_list ${dir})
        else()
            message(STATUS "Excluding directory: ${dir}")
        endif()
        endforeach()
        set(${output_var} ${result_list} PARENT_SCOPE)
    endfunction()

    # 过滤掉包含 /SDL2 的路径
    filter_include_dirs("${SDL2_INCLUDE_DIRS}" SDL2_INCLUDE_DIRS  "/SDL2")
    message(STATUS "SDL2 filtered include_dirs: ${SDL2_INCLUDE_DIRS}")
    endif()

endif()

# include directories for SDL
if(SDL3_FOUND)
    include_directories(${SDL3_INCLUDE_DIRS})
else()
    include_directories(${SDL2_INCLUDE_DIRS})
endif()

# 查找其他库
if(SDL3_FOUND)
    message(STATUS "Using SDL3")
else()
    #find_package(SDL2_ttf QUIET)
    if(SDL2_ttf_FOUND)
        message(STATUS "SDL2_ttf found")
    endif()
endif()

find_package(glad QUIET)

find_package(Freetype REQUIRED)

if(WIN32)
    message(STATUS "Windows platform detected")
else()
    find_package(iconv QUIET)
    find_package(OpenGL QUIET)
endif()

# Collect sources for libraries and executables
file(GLOB_RECURSE S_SOURCES "s/*.cpp" "s/*.h")
file(GLOB_RECURSE MAIN_SOURCES "main/*.cpp" "main/*.c" "main/*.h")
file(GLOB_RECURSE SOUND_SOURCES "sound/*.cpp" "sound/*.c" "sound/*.h")

# Create libraries
# s_lib depends on Qt + SDL
add_library(s_lib STATIC ${S_SOURCES})
target_include_directories(s_lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
if(SDL3_FOUND)
    target_link_libraries(s_lib PRIVATE SDL3::SDL3)
else()
    target_link_libraries(s_lib PRIVATE SDL2::SDL2)
endif()
# link Qt to s_lib
target_link_libraries(s_lib PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# main_lib - no Qt dependency
add_library(main_lib STATIC ${MAIN_SOURCES})
target_include_directories(main_lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
if(SDL3_FOUND)
    target_compile_definitions(main_lib PRIVATE USING_SDL3=1)
else()
    target_compile_definitions(main_lib PRIVATE USING_SDL3=0)
endif()

# sound_lib - no Qt dependency
add_library(sound_lib STATIC ${SOUND_SOURCES})
target_include_directories(sound_lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
if(SDL3_FOUND)
    target_compile_definitions(sound_lib PRIVATE USING_SDL3=1)
else()
    target_compile_definitions(sound_lib PRIVATE USING_SDL3=0)
endif()

# Handle Ogg/Vorbis for sound_lib
if(USING_OGG)
    target_compile_definitions(main_lib PUBLIC USING_OGG=1)
    target_compile_definitions(sound_lib PUBLIC USING_OGG=1)
    find_package(Ogg REQUIRED)
    if(TARGET Ogg::ogg)
        target_link_libraries(sound_lib PRIVATE Ogg::ogg)
    else()
        target_link_libraries(sound_lib PRIVATE ogg)
    endif()
    find_library(VORBISFILE_LIB NAMES vorbisfile libvorbisfile vorbisfile_static)
    find_library(VORBIS_LIB NAMES vorbis libvorbis vorbis_static)
    if (VORBISFILE_LIB AND VORBIS_LIB)
        message(STATUS "Found Vorbis libs: ${VORBISFILE_LIB}, ${VORBIS_LIB}")
        target_link_libraries(sound_lib PRIVATE ${VORBISFILE_LIB} ${VORBIS_LIB})
    else()
        message(FATAL_ERROR "Could not find Vorbis libraries (vorbisfile/ vorbis). Install libvorbis via vcpkg or system packages.")
    endif()
else()
    target_compile_definitions(main_lib PUBLIC USING_OGG=0)
    target_compile_definitions(sound_lib PUBLIC USING_OGG=0)
endif()

# After find_package(Freetype REQUIRED)
if(TARGET Freetype::Freetype)
    target_link_libraries(main_lib PRIVATE Freetype::Freetype)
elseif(TARGET freetype)
    target_link_libraries(main_lib PRIVATE freetype)
elseif(DEFINED FREETYPE_LIBRARIES)
    target_link_libraries(main_lib PRIVATE ${FREETYPE_LIBRARIES})
endif()

# After SDL detection, link/include for all internal libraries
if(SDL3_FOUND)
    # link SDL3 target to all libs so they get include dirs
    target_link_libraries(s_lib PRIVATE SDL3::SDL3)
    target_link_libraries(main_lib PRIVATE SDL3::SDL3)
    target_link_libraries(sound_lib PRIVATE SDL3::SDL3)
    if (DEFINED SDL3_INCLUDE_DIRS)
        target_include_directories(s_lib PRIVATE ${SDL3_INCLUDE_DIRS})
        target_include_directories(main_lib PRIVATE ${SDL3_INCLUDE_DIRS})
        target_include_directories(sound_lib PRIVATE ${SDL3_INCLUDE_DIRS})
    endif()
else()
    target_link_libraries(s_lib PRIVATE SDL2::SDL2)
    target_link_libraries(main_lib PRIVATE SDL2::SDL2)
    target_link_libraries(sound_lib PRIVATE SDL2::SDL2)
    if (DEFINED SDL2_INCLUDE_DIRS)
        target_include_directories(s_lib PRIVATE ${SDL2_INCLUDE_DIRS})
        target_include_directories(main_lib PRIVATE ${SDL2_INCLUDE_DIRS})
        target_include_directories(sound_lib PRIVATE ${SDL2_INCLUDE_DIRS})
    endif()
endif()

# Remove any previous direct link of s_lib to Qt (already added earlier) -- keep as is

# Build executables
# sdlpal_qt.exe: use s + sound + main
add_executable(sdlpal_qt "s/main.cpp")
target_link_libraries(sdlpal_qt PRIVATE s_lib sound_lib main_lib Qt${QT_VERSION_MAJOR}::Widgets)
set_target_properties(sdlpal_qt PROPERTIES WIN32_EXECUTABLE TRUE)

# sdlpal.exe: palmain/main.cpp + s + sound
# User requested palmain/main.cpp as entry; ensure file exists
set(PALMAIN_ENTRY "palmain/main.cpp")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${PALMAIN_ENTRY}")
    add_executable(sdlpal "${PALMAIN_ENTRY}")
else()
    # fallback: try main/CPalMain.cpp as entry wrapper if palmain not present
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/main/CPalMain.cpp")
        add_executable(sdlpal "main/CPalMain.cpp")
    else()
        message(FATAL_ERROR "Entry source for sdlpal not found: ${PALMAIN_ENTRY} nor main/CPalMain.cpp")
    endif()
endif()

target_link_libraries(sdlpal PRIVATE sound_lib main_lib)
set_target_properties(sdlpal PROPERTIES WIN32_EXECUTABLE TRUE)

# Link other global libs to executables as needed
if(glad_FOUND)
    target_link_libraries(sdlpal_qt PRIVATE glad::glad)
    target_link_libraries(sdlpal PRIVATE glad::glad)
endif()

if(WIN32)
    target_link_libraries(sdlpal_qt PRIVATE winmm opengl32.lib gdi32 user32 shell32)
    target_link_libraries(sdlpal PRIVATE winmm opengl32.lib gdi32 user32 shell32)
else()
    if(iconv_FOUND)
        target_link_libraries(sdlpal_qt PRIVATE Iconv::Iconv)
        target_link_libraries(sdlpal PRIVATE Iconv::Iconv)
    endif()
    if(OpenGL_FOUND)
        target_link_libraries(sdlpal_qt PRIVATE OpenGL::OpenGL)
        target_link_libraries(sdlpal PRIVATE OpenGL::OpenGL)
    endif()
endif()

# Enforce static MSVC runtime flags for all project targets when USING_STATIC on MSVC
if(USING_STATIC AND MSVC)
    set(_static_runtime_flags
        $<$<CONFIG:Debug>:/MTd>
        $<$<CONFIG:RelWithDebInfo>:/MT>
        $<$<CONFIG:Release>:/MT>
        $<$<CONFIG:MinSizeRel>:/MT>
    )
    foreach(_t IN LISTS PROJECT_NAME)
        # placeholder to satisfy generator; will set individual targets below
    endforeach()
    # Apply flags to libraries and executables created in this CMakeLists
    # s_lib, main_lib, sound_lib, sdlpal_qt, sdlpal
    foreach(_target IN ITEMS s_lib main_lib sound_lib sdlpal_qt sdlpal)
        if(TARGET ${_target})
            target_compile_options(${_target} PRIVATE ${_static_runtime_flags})
        endif()
    endforeach()
endif()

# End of file