cmake_minimum_required(VERSION 3.18)

project(CPURasterizer VERSION 1.0)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

include(FetchContent)

# glm
FetchContent_Declare(
        glm
        GIT_REPOSITORY	https://github.com/g-truc/glm.git
        GIT_TAG 	0af55ccecd98d4e5a8d1fad7de25ba429d60e863 #refs/tags/1.0.1
        FIND_PACKAGE_ARGS CONFIG
)

# SDL
FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG     d9d5536704d585616d4db3c8ba3c4ff6fc2757e1
        FIND_PACKAGE_ARGS CONFIG
)

# STB_image
FetchContent_Declare(
        stb_image
        URL "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"
        DOWNLOAD_NO_EXTRACT TRUE
)

# either find or download external libraries
FetchContent_MakeAvailable(glm SDL3 stb_image)

set(SDL_TEST_LIBRARY OFF CACHE BOOL "")
set(SDL_EXAMPLES OFF CACHE BOOL "")
set(SDL_TESTS OFF CACHE BOOL "")

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "src/*.cpp")

add_library(stb_image INTERFACE)

target_include_directories(stb_image INTERFACE ${stb_image_SOURCE_DIR})

add_executable(CPURasterizer ${SOURCE_FILES})

target_compile_definitions(CPURasterizer PRIVATE "PROJECT_ROOT_DIR=R\"(${CMAKE_SOURCE_DIR})\"")

target_include_directories(CPURasterizer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src stb_image)

target_link_libraries(CPURasterizer PRIVATE
                      glm::glm
                      SDL3::SDL3
                      stb_image)

