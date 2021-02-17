#include <assert.h>

#include "../src/line.hpp"
#include "../build/_deps/bitsquidfoundation-src/array.h"
#include "../build/_deps/bitsquidfoundation-src/memory.h"

using namespace foundation;

void test_los() {
    auto clear = [](int64_t x, int64_t y) {
        if (x == 2 && y == 2) {
            return false;
        }

        return true;
    };

    bool has_los = line::los({0, 0}, {5, 0}, clear, line::LineMode::AllowDiagonal);
    assert(has_los == true);

    bool blocked_los = line::los({0, 0}, {5, 5}, clear, line::LineMode::AllowDiagonal);
    assert(blocked_los == false);
}

void test_line() {
    Allocator &allocator = foundation::memory_globals::default_allocator();
    Array<line::Coordinate> coordinates = line::line(allocator, {0, 0}, {3, 0});

    assert(array::size(coordinates) == 4);
    assert(coordinates[0].x == 0);
    assert(coordinates[0].y == 0);
    assert(coordinates[1].x == 1);
    assert(coordinates[1].y == 0);
    assert(coordinates[2].x == 2);
    assert(coordinates[2].y == 0);
    assert(coordinates[3].x == 3);
    assert(coordinates[3].y == 0);

    auto clear = [](int64_t x, int64_t y) {
        if (x == 1 && y == 0) {
            return false;
        }

        return true;
    };

    bool blocked = false;
    coordinates = line::line(allocator, {0, 0}, {3, 0}, clear, &blocked, line::LineMode::AllowDiagonal);

    assert(blocked == true);
    assert(array::size(coordinates) == 1);
    assert(coordinates[0].x == 0);
    assert(coordinates[0].y == 0);
}

void test_diagonal() {
    Allocator &allocator = foundation::memory_globals::default_allocator();
    Array<line::Coordinate> coordinates = line::line(allocator, {0, 0}, {2, 2}, nullptr, nullptr, line::LineMode::AllowDiagonal);
    assert(array::size(coordinates) == 3);

    coordinates = line::line(allocator, {0, 0}, {2, 2}, nullptr, nullptr, line::LineMode::OnlyOrthogonal);
    assert(array::size(coordinates) == 5);
}

void test_zig_zag() {
    Allocator &allocator = foundation::memory_globals::default_allocator();
    Array<line::Coordinate> coordinates = line::zig_zag(allocator, {0, 0}, {5, 5});
    assert(array::size(coordinates) == 11);
}

int main(int, char**) {
    memory_globals::init();
    test_los();
    test_line();
    test_diagonal();
    test_zig_zag();
    memory_globals::shutdown();
}
