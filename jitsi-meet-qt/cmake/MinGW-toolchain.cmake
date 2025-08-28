# CMake toolchain file for MinGW cross-compilation
# This file configures CMake to use MinGW compiler for Windows builds

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# MinGW工具链路径配置
if(NOT DEFINED MINGW_ROOT)
    # 尝试自动检测MinGW安装路径
    set(MINGW_PATHS
        "C:/Qt/Tools/mingw810_64"
        "C:/Qt/Qt5.15.2/Tools/mingw810_64"
        "C:/mingw64"
        "C:/msys64/mingw64"
    )
    
    foreach(path ${MINGW_PATHS})
        if(EXISTS "${path}/bin/gcc.exe")
            set(MINGW_ROOT ${path})
            break()
        endif()
    endforeach()
    
    if(NOT MINGW_ROOT)
        message(FATAL_ERROR "MinGW not found. Please set MINGW_ROOT or install MinGW.")
    endif()
endif()

message(STATUS "Using MinGW at: ${MINGW_ROOT}")

# 设置编译器
set(CMAKE_C_COMPILER ${MINGW_ROOT}/bin/gcc.exe)
set(CMAKE_CXX_COMPILER ${MINGW_ROOT}/bin/g++.exe)
set(CMAKE_RC_COMPILER ${MINGW_ROOT}/bin/windres.exe)

# 设置工具
set(CMAKE_AR ${MINGW_ROOT}/bin/ar.exe)
set(CMAKE_RANLIB ${MINGW_ROOT}/bin/ranlib.exe)
set(CMAKE_STRIP ${MINGW_ROOT}/bin/strip.exe)

# 设置查找路径
set(CMAKE_FIND_ROOT_PATH ${MINGW_ROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# MinGW特定编译选项
set(CMAKE_CXX_FLAGS_INIT "-static-libgcc -static-libstdc++")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static-libgcc -static-libstdc++")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-static-libgcc -static-libstdc++")

# 优化设置
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O2 -DNDEBUG -g")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-O0 -g3 -DDEBUG")

# Windows特定设置
add_definitions(-DWIN32 -D_WIN32 -DWINVER=0x0601 -D_WIN32_WINNT=0x0601)

# 禁用一些MinGW警告
add_compile_options(-Wno-unknown-pragmas -Wno-unused-parameter)

# 设置运行时库
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_MSVCRT_LIB "")
else()
    set(CMAKE_MSVCRT_LIB "")
endif()