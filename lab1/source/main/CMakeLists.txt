add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} lib_${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE ../lib)
