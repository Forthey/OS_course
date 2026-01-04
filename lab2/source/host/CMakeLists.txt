file(GLOB_RECURSE PROJECT_HOST_SOURCE_FILES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_executable(host ${PROJECT_HOST_SOURCE_FILES})

target_link_libraries(host PRIVATE lib_${PROJECT_NAME})
target_include_directories(host PRIVATE ../lib)

target_compile_options(host PRIVATE
        -Wall
        -Werror
)
