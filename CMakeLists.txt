cmake_minimum_required(VERSION 3.14)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

project(mruntime VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)

install(TARGETS mruntime DESTINATION lib)

install(DIRECTORY include/ DESTINATION include/mruntime)
