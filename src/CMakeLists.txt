add_library(mruntime_src OBJECT
    mruntime.cpp
    process.cpp
    clock.cpp
    node.cpp
)

target_include_directories(mruntime_src PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../mdds/include
)

set(MDDS_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/../../mdds/build/src/libmdds.a)
set(MDDS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../mdds/include)

add_library(mdds STATIC IMPORTED)
set_target_properties(mdds PROPERTIES IMPORTED_LOCATION ${MDDS_LIBRARY})
set_target_properties(mdds PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${MDDS_INCLUDE_DIR}")

target_link_libraries(mruntime_src PRIVATE mdds pthread)

target_compile_options(mruntime_src PRIVATE
    -Wall -Wextra -Wpedantic
)

set_target_properties(mruntime_src PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
