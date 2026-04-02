cmake_minimum_required(VERSION 3.14)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

find_package(Threads REQUIRED)

# Provide mdds alias via imported library
set(MDDS_LIBRARY "/mnt/data/workspace/moss/mdds/build/src/libmdds.so")
set(MDDS_INCLUDE_DIR "/mnt/data/workspace/moss/mdds/include")

add_library(mdds::mdds SHARED IMPORTED)
set_target_properties(mdds::mdds PROPERTIES
    IMPORTED_LOCATION "${MDDS_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${MDDS_INCLUDE_DIR}"
)

set(MRUNTIME_SOURCES
    mruntime.cpp
    process.cpp
    clock.cpp
    node.cpp
)

if(BUILD_SHARED_LIBS)
    add_library(mruntime SHARED ${MRUNTIME_SOURCES})
    set_target_properties(mruntime PROPERTIES
        VERSION 1.0.0
        SOVERSION 1
        POSITION_INDEPENDENT_CODE ON
    )
else()
    add_library(mruntime STATIC ${MRUNTIME_SOURCES})
    set_target_properties(mruntime PROPERTIES
        VERSION 1.0.0
        SOVERSION 1
    )
endif()

target_include_directories(mruntime PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${MDDS_INCLUDE_DIR}
)

target_link_libraries(mruntime PUBLIC
    mdds::mdds
    Threads::Threads
)

target_compile_options(mruntime PRIVATE
    -Wall -Wextra -Wpedantic
)

install(TARGETS mruntime
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
