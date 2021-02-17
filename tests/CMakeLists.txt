include(FetchContent)

FetchContent_Declare(
    BitSquidFoundation
    GIT_REPOSITORY https://github.com/hsjunnesson/bitsquid-foundation.git
    GIT_TAG        fa0066ff464f5e1022ddc0abcdddcf793b8946c9
)

if (NOT bitsquidfoundation_POPULATED)
    FetchContent_MakeAvailable(BitSquidFoundation)
endif()

file(GLOB_RECURSE SRC_Foundation
    "${bitsquidfoundation_SOURCE_DIR}/*.cpp"
    "${bitsquidfoundation_SOURCE_DIR}/*.h"
)

list(FILTER SRC_Foundation EXCLUDE REGEX ".*/unit_test.cpp$")

add_executable(test_7DRLx17
    test_line.cpp
    ${SRC_Foundation}
)

add_test(7DRLx17 test_7DRLx17)

include_directories(${bitsquidfoundation_SOURCE_DIR})
