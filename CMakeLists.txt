cmake_minimum_required(VERSION 3.15)
 
project("image-viewer")
 
enable_language( C CXX ASM )
set (CMAKE_CXX_STANDARD 11)
 
message("Building ${PROJECT_NAME}...")
 
include_directories(include)
 
add_executable(${PROJECT_NAME}
       src/main.cpp
       src/gif_parser.cpp
)
