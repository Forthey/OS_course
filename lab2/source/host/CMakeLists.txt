file(GLOB_RECURSE PROJECT_HOST_SOURCE_FILES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_executable(${PROJECT_NAME} ${PROJECT_HOST_SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} lib_${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE ../lib)

target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Werror
)
