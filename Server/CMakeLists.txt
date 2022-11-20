cmake_minimum_required(VERSION 3.16.3)
set(CMAKE_TOOLCHAIN_FILE "C:/Code/tools/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
project(Server)

set(CMAKE_CXX_STANDARD 20)

# Get recursively all .cpp and .hpp files in src subdir
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.h")

add_executable(${PROJECT_NAME} ${SOURCES})

find_package(Threads REQUIRED)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX /wd4201)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror -Wno-unused-lambda-capture -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function -fdiagnostics-show-template-tree)
endif()

target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/../external/crow/include
        )

target_link_libraries(${PROJECT_NAME}
        PRIVATE
            Threads::Threads
            data
            util
            matching
            nlohmann_json
            pqxx
)

if(WIN32)
    target_include_directories(${PROJECT_NAME}
            PRIVATE
            ws2_32
            wsock32
            )

    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            ws2_32
            wsock32
            )
endif()