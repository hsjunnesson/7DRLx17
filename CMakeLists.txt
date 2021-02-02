cmake_minimum_required(VERSION 3.16)
project(7DRLx17 VERSION 1.0.0)

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

file(GLOB_RECURSE SRC_Roguelike
    "src/*.cpp"
    "Src/*.h"
)

file(GLOB_RECURSE SRC_Foundation
    "${bitsquidfoundation_SOURCE_DIR}/*.cpp"
    "${bitsquidfoundation_SOURCE_DIR}/*.h"
)

list(FILTER SRC_Foundation EXCLUDE REGEX ".*/unit_test.cpp$")

add_executable(${PROJECT_NAME}
    ${SRC_Roguelike}
    ${SRC_Foundation}
)

source_group("src" FILES ${SRC_Roguelike})
source_group("foundation" FILES ${SRC_Foundation})

include_directories(${stackwalker_SOURCE_DIR}/Main/StackWalker)
include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIR})
include_directories(${bitsquidfoundation_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARY} ${SDL2_MAIN})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2_image)
target_link_libraries(${PROJECT_NAME} PRIVATE protobuf::libprotobuf-lite)
target_link_libraries(${PROJECT_NAME} PRIVATE StackWalker)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
