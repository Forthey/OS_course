message("Configuring host...")
file(GLOB_RECURSE PROJECT_HOST_SOURCE_FILES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_executable(host ${PROJECT_HOST_SOURCE_FILES})

target_link_libraries(host PRIVATE lib_${PROJECT_NAME})

foreach (CONN_TYPE IN LISTS CONN_TYPES)
    target_link_libraries(host PRIVATE conn_${CONN_TYPE})
endforeach ()

target_compile_options(host PRIVATE
        -Wall
        -Werror
)
message("Host configure complete")
