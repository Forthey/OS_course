find_package(yaml-cpp QUIET)

if (NOT yaml-cpp_FOUND)
    message(STATUS "yaml-cpp not found, downloading from source...")

    include(FetchContent)
    FetchContent_Declare(
            yaml-cpp
            GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
            GIT_TAG 0.8.0
    )

    FetchContent_MakeAvailable(yaml-cpp)
else()
    message(STATUS "yaml-cpp was found in system!")
endif()
