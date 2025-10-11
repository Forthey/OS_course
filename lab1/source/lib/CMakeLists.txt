file(GLOB_RECURSE PROJECT_SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h)

add_library(lib_${PROJECT_NAME} SHARED ${PROJECT_SOURCE_FILES}
        SigHandler/SigHandler.cpp
        SigHandler/SigHandler.h
        OnceInstaniated/OnceInstantiated.h
        Config/Config.h
        Daemon/DiskMonitor.cpp
        Daemon/DiskMonitor.h)

target_include_directories(lib_${PROJECT_NAME} PRIVATE ./)