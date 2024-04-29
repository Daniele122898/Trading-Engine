cmake_minimum_required(VERSION 3.16.3)
set(CMAKE_TOOLCHAIN_FILE "C:/Code/tools/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
project(Server)

set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

# Get recursively all .cpp and .hpp files in src subdir
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.h")

add_executable(${PROJECT_NAME} ${SOURCES})

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PostgreSQL REQUIRED)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX /wd4201)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror -Wno-deprecated-declarations -Wno-unused-lambda-capture -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function -Wno-unused-but-set-variable -fdiagnostics-show-template-tree)
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR}/../internal/matching/src)

target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/../external/crow/include
        )

target_link_libraries(${PROJECT_NAME}
        PRIVATE
            Threads::Threads
            data
            util
            # matching
            readerwriterqueue
            nlohmann_json
            date
            pqxx
            OpenSSL::Crypto
            OpenSSL::SSL
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
