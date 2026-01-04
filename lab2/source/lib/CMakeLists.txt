file(GLOB_RECURSE PROJECT_SOURCE_FILES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

set(PROTO_DIR ${CMAKE_SOURCE_DIR}/proto)
set(PROTO_FILES
        ${PROTO_DIR}/messages.proto
)

add_library(messages_proto ${PROTO_FILES})

target_link_libraries(messages_proto PUBLIC protobuf::libprotobuf)

target_include_directories(messages_proto
        PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
        ${PROTO_DIR}
        ${protobuf_SOURCE_DIR}/src
)

protobuf_generate(
        TARGET messages_proto
        LANGUAGE cpp
        APPEND_PATH
        PROTOS ${PROTO_FILES}
        IMPORT_DIRS ${PROTO_DIR} ${protobuf_SOURCE_DIR}/src
)

add_library(lib_${PROJECT_NAME} STATIC ${PROJECT_SOURCE_FILES})
target_include_directories(lib_${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lib_${PROJECT_NAME} PUBLIC messages_proto)

target_compile_options(lib_${PROJECT_NAME} PRIVATE
        -Wall
        -Werror
)
