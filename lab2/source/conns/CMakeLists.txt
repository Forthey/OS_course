message("Configuring Connections...")
file(GLOB CHILD_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} */)

set(CONN_TYPES)

foreach (DIR ${CHILD_DIRS})
    if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${DIR}")

        string(REGEX REPLACE "/$" "" DIR_NAME "${DIR}")

        string(TOLOWER "${DIR_NAME}" CONN_TYPE)

        file(GLOB SRC
                "${CMAKE_CURRENT_SOURCE_DIR}/${DIR_NAME}/*.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/${DIR_NAME}/*.h"
        )

        add_library(conn_${CONN_TYPE} ${SRC})

        target_link_libraries(conn_${CONN_TYPE} PUBLIC types_${PROJECT_NAME} PRIVATE lib_${PROJECT_NAME})
        target_include_directories(conn_${CONN_TYPE} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

        target_compile_options(conn_${CONN_TYPE} PRIVATE
                -Wall
                -Werror
        )

        list(APPEND CONN_TYPES ${CONN_TYPE})

        message(STATUS "Created connection lib: conn_${CONN_TYPE}")
    endif ()
endforeach ()

set(CONN_TYPES "${CONN_TYPES}" PARENT_SCOPE)
message("Connections configure complete")
