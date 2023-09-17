cmake_minimum_required(VERSION 3.16.3)
project(data)

set(CMAKE_CXX_STANDARD 20)

# Get recursively all .cpp and .hpp files in src subdir
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.h")
add_library(${PROJECT_NAME} STATIC ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PUBLIC /W4 /WX /wd4201)
else()
    target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra -Werror -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function -fdiagnostics-show-template-tree)
endif()

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        )

target_link_libraries(${PROJECT_NAME} PRIVATE
        util
        )