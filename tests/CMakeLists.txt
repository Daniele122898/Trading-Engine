cmake_minimum_required(VERSION 3.16.3)
project(tests)

set(CMAKE_CXX_STANDARD 20)

enable_testing()

# Get recursively all .cpp and .hpp files in src subdir
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "./*.cpp" "./*.h")

add_executable(${PROJECT_NAME} ${SOURCES})

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX /wd4201)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function -fdiagnostics-show-template-tree)
endif()

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        data
        util
        gtest
        matching
        )
