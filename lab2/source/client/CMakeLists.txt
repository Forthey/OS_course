message("Configuring client...")
file(GLOB_RECURSE PROJECT_CLIENT_SOURCE_FILES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

foreach (CONN_TYPE IN LISTS CONN_TYPES)
    message("Configuring client_${CONN_TYPE}...")
    add_executable(client_${CONN_TYPE} ${PROJECT_CLIENT_SOURCE_FILES})

    target_link_libraries(client_${CONN_TYPE} PRIVATE lib_${PROJECT_NAME} conn_${CONN_TYPE})

    target_compile_options(client_${CONN_TYPE} PRIVATE
            -Wall
            -Werror
    )

    string(TOUPPER "${CONN_TYPE}" CONN_TYPE_UPPER)
    target_compile_definitions(client_${CONN_TYPE} PRIVATE CLIENT_CONN_${CONN_TYPE_UPPER})
endforeach ()
message("Client configure complete")
