file(GLOB_RECURSE PROJECT_SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h)

add_library(lib_${PROJECT_NAME} STATIC ${PROJECT_SOURCE_FILES})

target_include_directories(lib_${PROJECT_NAME} PRIVATE ./)

include(${CMAKE_SOURCE_DIR}/deps/yaml_cpp.cmake)

target_link_libraries(lib_${PROJECT_NAME} PRIVATE yaml-cpp)

target_compile_options(lib_${PROJECT_NAME} PRIVATE
        -Wall
        -Werror
)
