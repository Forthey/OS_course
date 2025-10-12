file(GLOB_RECURSE PROJECT_SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h)

add_library(lib_${PROJECT_NAME} SHARED ${PROJECT_SOURCE_FILES}
        SigHandler/SignalHandler.cpp
        SigHandler/SignalHandler.h
        OnceInstaniated/OnceInstantiated.h
        Config/Config.h
        Daemon/DiskMonitor.cpp
        Daemon/DiskMonitor.h
        Config/ConfigLoader.h
        Config/YamlConfigLoader.cpp
        Config/YamlConfigLoader.h
        Logger/Logger.h)

target_include_directories(lib_${PROJECT_NAME} PRIVATE ./)

include(${CMAKE_SOURCE_DIR}/deps/yaml_cpp.cmake)

target_link_libraries(lib_${PROJECT_NAME} PRIVATE yaml-cpp)
