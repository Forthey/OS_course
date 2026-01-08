message("Adding interface library...")
file(GLOB_RECURSE PROJECT_TYPES_SOURCE_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(types_${PROJECT_NAME} INTERFACE)

target_include_directories(types_${PROJECT_NAME}
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
message("Interface library added")
