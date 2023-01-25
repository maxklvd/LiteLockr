cmake_minimum_required(VERSION 3.12)
project(LiteLockr)

find_package(OpenMP REQUIRED)
set(CMAKE_CXX_STANDARD 20)

include_directories(ext)
include_directories(ext/agg-2.4/include)
include_directories(c:/boost_1_81_0)
include_directories(src)

add_definitions("-DNOMINMAX")
add_definitions("-DUNICODE")
add_definitions("-D_DEBUG")
add_definitions("-D_WIN32_WINNT=0x0A00")
add_definitions("-DBZ_NO_STDIO")
add_definitions("-DINTERCEPTION_STATIC")

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.rc ext/*.cpp ext/*.c)

message("PROJECT_SOURCE_DIR= ${PROJECT_SOURCE_DIR}")
message("PROJECT_BINARY_DIR= ${PROJECT_BINARY_DIR}")
message("SRC_FILES= ${SRC_FILES}")

add_executable(LiteLockr ${SRC_FILES})

target_link_libraries(LiteLockr winmm comctl32 Wtsapi32 Shlwapi UxTheme Dbghelp Dwmapi)
