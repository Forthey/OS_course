file(GLOB_RECURSE PROJECT_CLIENT_SOURCE_FILES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

set(CLIENT_VARIANTS mq fifo sock)

foreach (VARIANT IN LISTS CLIENT_VARIANTS)
    add_executable(client_${VARIANT} ${PROJECT_CLIENT_SOURCE_FILES})

    target_link_libraries(client_${VARIANT} PRIVATE lib_${PROJECT_NAME})
    target_include_directories(client_${VARIANT} PRIVATE ../lib)

    target_compile_options(client_${VARIANT} PRIVATE
            -Wall
            -Werror
    )

    string(TOUPPER "${VARIANT}" VARIANT_UPPER)
    target_compile_definitions(client_${VARIANT} PRIVATE CLIENT_CONN_${VARIANT_UPPER})
endforeach ()
