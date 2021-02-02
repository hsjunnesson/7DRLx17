cmake_minimum_required(VERSION 3.16)
project(7drl VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

find_path(SDL2_INCLUDE_DIR SDL2/SDL.h)
find_package(SDL2 CONFIG REQUIRED)
find_package(sdl2-image CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)

include(FetchContent)

FetchContent_Declare(
    StackWalker
    GIT_REPOSITORY https://github.com/JochenKalmbach/StackWalker.git
    GIT_TAG        4161d093d57ad6c905599b85861601146c3ef1e9
)

#FetchContent_GetProperties(StackWalker)
if (NOT stackwalker_POPULATED)
    FetchContent_Populate(StackWalker)
    add_subdirectory(${stackwalker_SOURCE_DIR} ${stackwalker_BINARY_DIR})
#    FetchContent_MakeAvailable(StackWalker)
endif()

add_executable(Roguelike
    src/engine.cpp
    src/input.cpp
    src/log.cpp
    src/main.cpp
    src/texture.cpp
    src/world.cpp
    src/engine.h
    src/input.h
    src/log.h
    src/texture.h
    src/world.h
    src/json.hpp
    bitsquid_foundation/array.h
    bitsquid_foundation/collection_types.h
    bitsquid_foundation/hash.h
    bitsquid_foundation/math_types.h
    bitsquid_foundation/memory.cpp
    bitsquid_foundation/memory.h
    bitsquid_foundation/memory_types.h
    bitsquid_foundation/murmur_hash.cpp
    bitsquid_foundation/murmur_hash.h
    bitsquid_foundation/queue.h
    bitsquid_foundation/snprintf_msvc.h
    bitsquid_foundation/string_stream.cpp
    bitsquid_foundation/string_stream.h
    bitsquid_foundation/temp_allocator.h
    bitsquid_foundation/types.h
)

include_directories(bitsquid_foundation)
include_directories(${stackwalker_SOURCE_DIR}/Main/StackWalker)
include_directories(Roguelike PUBLIC ${SDL2_INCLUDE_DIR})

target_link_libraries(Roguelike PRIVATE ${SDL2_LIBRARY} ${SDL2_MAIN})
target_link_libraries(Roguelike PRIVATE SDL2::SDL2_image)
target_link_libraries(Roguelike PRIVATE protobuf::libprotobuf-lite)
target_link_libraries(Roguelike PRIVATE StackWalker)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Roguelike)
set_property(TARGET Roguelike PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
