cmake_minimum_required(VERSION 3.20)
project(unit_tests)

set(CMAKE_CXX_STANDARD 20)

Include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.0.0-preview3
)

FetchContent_MakeAvailable(Catch2)

file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
file(GLOB_RECURSE AVRLIB_SOURCES "lib/*.cpp")
set(SOURCE_FILES ${TEST_SOURCES} ${AVRLIB_SOURCES})

add_executable(unit_tests ${SOURCE_FILES})
target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain)
