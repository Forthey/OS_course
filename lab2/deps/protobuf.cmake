message("Installing Protobuf...")
set(protobuf_BUILD_TESTS OFF CACHE BOOL "Disable protobuf tests" FORCE)
set(protobuf_BUILD_CONFORMANCE OFF CACHE BOOL "Disable protobuf conformance tests" FORCE)
set(protobuf_WITH_ZLIB OFF CACHE BOOL "Disable zlib in protobuf" FORCE)
set(utf8_range_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

include(FetchContent)

FetchContent_Declare(
        protobuf
        GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
        GIT_TAG        v33.2
)

FetchContent_MakeAvailable(protobuf)

include(${protobuf_SOURCE_DIR}/cmake/protobuf-generate.cmake)
message("Protobuf installed")
