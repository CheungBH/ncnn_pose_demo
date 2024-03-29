if(CMAKE_TOOLCHAIN_FILE)
		set(LIBRARY_OUTPUT_PATH_ROOT ${CMAKE_BINARY_DIR} CACHE PATH "root for library output, set this to change where android libs are compiled to")
    # get absolute path, but get_filename_component ABSOLUTE only refer with source dir, so find_file here :(
    get_filename_component(CMAKE_TOOLCHAIN_FILE_NAME ${CMAKE_TOOLCHAIN_FILE} NAME)
    find_file(CMAKE_TOOLCHAIN_FILE ${CMAKE_TOOLCHAIN_FILE_NAME} PATHS ${CMAKE_SOURCE_DIR} NO_DEFAULT_PATH)
    message(STATUS "CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")
endif()

if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Installation Directory")
endif()
message(STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")

cmake_minimum_required(VERSION 2.8.12)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE release CACHE STRING "Choose the type of build" FORCE)
endif()

if(NOT CMAKE_VERSION VERSION_LESS "3.15")
    # enable CMAKE_MSVC_RUNTIME_LIBRARY
    cmake_policy(SET CMP0091 NEW)
endif()

project(ncnn)

if(MSVC AND NOT CMAKE_VERSION VERSION_LESS "3.15")
    option(NCNN_BUILD_WITH_STATIC_CRT "Enables use of statically linked CRT for statically linked ncnn" OFF)
    if(NCNN_BUILD_WITH_STATIC_CRT)
        # cmake before version 3.15 not work
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

option(NCNN_OPENMP "openmp support" OFF)
option(NCNN_STDIO "load model from external file" ON)
option(NCNN_STRING "plain and verbose string" ON)
option(NCNN_INSTALL_SDK "install ncnn library and headers" ON)
option(NCNN_SIMPLEOCV "minimal opencv structure emulation" OFF)
option(NCNN_SIMPLEOMP "minimal openmp runtime emulation" OFF)
option(NCNN_SIMPLESTL "minimal cpp stl structure emulation" OFF)
option(NCNN_THREADS "build with threads" ON)
option(NCNN_BENCHMARK "print benchmark information for every layer" OFF)
option(NCNN_PIXEL "convert and resize from/to image pixel" ON)
option(NCNN_PIXEL_ROTATE "rotate image pixel orientation" ON)
option(NCNN_PIXEL_AFFINE "warp affine image pixel" ON)
option(NCNN_CMAKE_VERBOSE "print verbose cmake messages" OFF)
option(NCNN_VULKAN "vulkan compute support" ON)
option(NCNN_SYSTEM_GLSLANG "use system glslang library" OFF)
option(NCNN_REQUANT "auto merge int8 quant and dequant" OFF)
option(NCNN_RUNTIME_CPU "runtime dispatch cpu routines" ON)
option(NCNN_DISABLE_PIC "disable position-independent code" OFF)
option(NCNN_BUILD_TESTS "build tests" OFF)
option(NCNN_COVERAGE "build for coverage" OFF)
option(NCNN_BUILD_BENCHMARK "build benchmark" OFF)

if(ANDROID OR IOS OR NCNN_SIMPLESTL)
    option(NCNN_DISABLE_RTTI "disable rtti" ON)
    option(NCNN_BUILD_TOOLS "build tools" OFF)
    option(NCNN_BUILD_EXAMPLES "build examples" ON)
else()
    option(NCNN_DISABLE_RTTI "disable rtti" OFF)
    option(NCNN_BUILD_TOOLS "build tools" ON)
    option(NCNN_BUILD_EXAMPLES "build examples" ON)
endif()

if(ANDROID OR IOS OR LINUX OR NCNN_SIMPLESTL)
    option(NCNN_DISABLE_EXCEPTION "disable exception" ON)
else()
    option(NCNN_DISABLE_EXCEPTION "disable exception" OFF)
endif()

##############################################

if((IOS AND CMAKE_OSX_ARCHITECTURES MATCHES "arm")
    OR (APPLE AND CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
    OR (CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm|aarch64)"))
    set(NCNN_TARGET_ARCH arm)
    option(NCNN_ARM82 "optimize aarch64 platform with armv8.2" ON)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(mips)")
    set(NCNN_TARGET_ARCH mips)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(riscv)")
    set(NCNN_TARGET_ARCH riscv)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(powerpc)")
    set(NCNN_TARGET_ARCH powerpc)
else()
    set(NCNN_TARGET_ARCH x86)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
        option(NCNN_AVX2 "optimize x86 platform with avx2" ON)
    endif()
endif()

message(STATUS "Target arch: ${NCNN_TARGET_ARCH}")

##############################################

# set cmake default folder name
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "cmake")

if(NCNN_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -coverage -lgcov")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s FORCE_FILESYSTEM=1 -s INITIAL_MEMORY=1024MB -s EXIT_RUNTIME=1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s FORCE_FILESYSTEM=1 -s INITIAL_MEMORY=1024MB -s EXIT_RUNTIME=1")
    set(CMAKE_EXECUTBLE_LINKER_FLAGS "${CMAKE_EXECUTBLE_LINKER_FLAGS} -s FORCE_FILESYSTEM=1 -s INITIAL_MEMORY=1024MB -s EXIT_RUNTIME=1")

    if(NCNN_OPENMP AND NCNN_SIMPLEOMP)
        # TODO better flags for emscripten
        # node --experimental-wasm-threads xxx.js
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=15")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=15")
        set(CMAKE_EXECUTBLE_LINKER_FLAGS "${CMAKE_EXECUTBLE_LINKER_FLAGS} -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=15")
    endif()
endif()

if(NCNN_VULKAN)
    if(NCNN_SYSTEM_GLSLANG)
        set(GLSLANG_TARGET_DIR "GLSLANG-NOTFOUND" CACHE PATH "Absolute path to glslangTargets.cmake directory")
        if(NOT GLSLANG_TARGET_DIR AND NOT DEFINED ENV{GLSLANG_TARGET_DIR})
            message(WARNING "GLSLANG_TARGET_DIR must be defined! NCNN_SYSTEM_GLSLANG will be turned off.")
            set(NCNN_SYSTEM_GLSLANG OFF)
        else()
            message(STATUS "Using glslang install located at ${GLSLANG_TARGET_DIR}")

            find_package(Threads)

            include("${GLSLANG_TARGET_DIR}/OSDependentTargets.cmake")
            include("${GLSLANG_TARGET_DIR}/OGLCompilerTargets.cmake")
            if(EXISTS "${GLSLANG_TARGET_DIR}/HLSLTargets.cmake")
                # hlsl support can be optional
                include("${GLSLANG_TARGET_DIR}/HLSLTargets.cmake")
            endif()
            include("${GLSLANG_TARGET_DIR}/glslangTargets.cmake")
            include("${GLSLANG_TARGET_DIR}/SPIRVTargets.cmake")

            if (NOT TARGET glslang OR NOT TARGET SPIRV)
                message(WARNING "glslang or SPIRV target not found! NCNN_SYSTEM_GLSLANG will be turned off.")
                set(NCNN_SYSTEM_GLSLANG OFF)
            endif()
        endif()
    endif()

    if(NOT NCNN_SYSTEM_GLSLANG)
        if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/glslang/CMakeLists.txt")
            message(FATAL_ERROR "The submodules were not downloaded! Please update submodules with \"git submodule update --init\" and try again.")
        else()
            # glslang requires c++11
            set(CMAKE_CXX_STANDARD 17)

            option(BUILD_EXTERNAL "" OFF)
            option(ENABLE_SPVREMAPPER "" OFF)
            option(ENABLE_GLSLANG_BINARIES "" OFF)
            option(ENABLE_HLSL "" OFF)
            option(ENABLE_RTTI "" OFF)
            option(ENABLE_EXCEPTIONS "" OFF)
            option(ENABLE_OPT "" OFF)
            option(ENABLE_PCH "s" OFF)
            option(ENABLE_CTEST "" OFF)
            add_subdirectory(glslang)
        endif()
    endif()
endif()

add_subdirectory(src)
add_subdirectory(examples)

# Copy resources
#FILE(COPY dev DESTINATION "${CMAKE_BINARY_DIR}/examples")
#FILE(COPY CNN_models DESTINATION "${CMAKE_BINARY_DIR}/examples")
#FILE(COPY pose_models DESTINATION "${CMAKE_BINARY_DIR}/examples")
#FILE(COPY yolov4-tiny-opt.param DESTINATION "${CMAKE_BINARY_DIR}/examples")
#FILE(COPY yolov4-tiny-opt.bin DESTINATION "${CMAKE_BINARY_DIR}/examples")

