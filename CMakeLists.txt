cmake_minimum_required(VERSION 3.28)
project(arcade-shooter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)

FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.2
        GIT_SHALLOW ON
)

FetchContent_MakeAvailable(SFML)

add_executable(main main.cpp)

target_include_directories(main PRIVATE
        ${sfml_SOURCE_DIR}/include
)

target_link_libraries(main PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
)
