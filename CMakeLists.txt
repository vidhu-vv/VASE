cmake_minimum_required(VERSION 3.20)
project(VASE VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()
set(SOURCES
    src/main.cpp
    src/VASE.cpp
    src/fft.cpp
    src/fft_convolver.cpp
    src/sh_encoder.cpp
    src/rotation.cpp
    src/hrtf_decoder.cpp
)

add_executable(VASE ${SOURCES})
target_include_directories(VASE PRIVATE
    src
    deps
)
if(MSVC)
    target_compile_options(VASE PRIVATE /W4)
else()
    target_compile_options(VASE PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
    )
endif()
if(UNIX AND NOT APPLE)
    target_link_libraries(VASE PRIVATE
        pthread
        m
        dl
    )
elseif(APPLE)
    target_link_libraries(VASE PRIVATE
        "-framework CoreAudio"
        "-framework CoreFoundation"
        "-framework AudioToolbox"
        -O3 
        -ffast-math
        m
    )
elseif(WIN32)
    target_link_libraries(VASE PRIVATE
        winmm
    )
endif()
add_custom_command(TARGET VASE POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data
        $<TARGET_FILE_DIR:VASE>/data
    COMMENT "Copying data/ folder to build output"
)

message(STATUS "")
message(STATUS "VASE configuration")
message(STATUS "  Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ std    : ${CMAKE_CXX_STANDARD}")
message(STATUS "  Platform   : ${CMAKE_SYSTEM_NAME}")
message(STATUS "  FFT        : hand-rolled (src/fft.cpp)")
message(STATUS "")
