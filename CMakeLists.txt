cmake_minimum_required(VERSION 3.16)
project(7DRLx17 VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)


# Find locally installed dependencies. Tip: Use VCPKG for these.

find_path(SDL2_INCLUDE_DIR SDL2/SDL.h)
find_package(SDL2 CONFIG REQUIRED)
find_package(sdl2-image CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)


# Fetch dependencies from Github

include(FetchContent)

FetchContent_Declare(
    StackWalker
    GIT_REPOSITORY https://github.com/JochenKalmbach/StackWalker.git
    GIT_TAG        4161d093d57ad6c905599b85861601146c3ef1e9
)

if (NOT stackwalker_POPULATED)
    FetchContent_Populate(StackWalker)
    add_subdirectory(${stackwalker_SOURCE_DIR} ${stackwalker_BINARY_DIR})
endif()


FetchContent_Declare(
    BitSquidFoundation
    GIT_REPOSITORY https://github.com/hsjunnesson/bitsquid-foundation.git
    GIT_TAG        fa0066ff464f5e1022ddc0abcdddcf793b8946c9
)

if (NOT bitsquidfoundation_POPULATED)
    FetchContent_MakeAvailable(BitSquidFoundation)
endif()


# Main game source

file(GLOB_RECURSE SRC_Roguelike
    "src/*.cpp"
    "Src/*.h"
)

if (MSVC)
    set_source_files_properties(${SRC_Roguelike} PROPERTIES COMPILE_FLAGS "/W4 /WX")
endif()


# Bitsquid foundation source

file(GLOB_RECURSE SRC_Foundation
    "${bitsquidfoundation_SOURCE_DIR}/*.cpp"
    "${bitsquidfoundation_SOURCE_DIR}/*.h"
)

list(FILTER SRC_Foundation EXCLUDE REGEX ".*/unit_test.cpp$")


# Generated protobuf implementation

file(GLOB_RECURSE SRC_Generated
    "generated_src/*.cc"
    "generated_src/*.h"
)


# Create executable
add_executable(${PROJECT_NAME}
    ${SRC_Roguelike}
    ${SRC_Foundation}
    ${SRC_Generated}
)


include_directories(${stackwalker_SOURCE_DIR}/Main/StackWalker)
include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIR})
include_directories(${bitsquidfoundation_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/generated_src)

target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARY} ${SDL2_MAIN})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2_image)
target_link_libraries(${PROJECT_NAME} PRIVATE protobuf::libprotobuf)
target_link_libraries(${PROJECT_NAME} PRIVATE StackWalker)


# Visual Studio Setup

if (MSVC)
    source_group("src" FILES ${SRC_Roguelike})
    source_group("foundation" FILES ${SRC_Foundation})
    source_group("generated" FILES ${SRC_Generated})
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()


# Enable cmake tests

enable_testing()
add_subdirectory(tests)
