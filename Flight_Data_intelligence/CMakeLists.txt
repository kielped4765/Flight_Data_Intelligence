cmake_minimum_required(VERSION 3.20)
project(Flight_Data_Intelligence VERSION 1.0)

# -- C++ Version --------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -- Memory Safety: AddressSanitizer (Linux/Mac only) -------------
# Not supported by MinGW GCC on Windows/MSYS2
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT WIN32)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

# -- Auto-Download Google Test ------------------------------------
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

# -- Main executable ----------------------------------------------
add_executable(Flight_Data_Intelligence
    src/main.cpp
    src/parser.cpp
    src/ring_buffer.cpp
)
target_include_directories(Flight_Data_Intelligence PRIVATE include)

# Link Windows socket library (required for WSAStartup, socket, recv, etc.)
target_link_libraries(Flight_Data_Intelligence ws2_32)

# Auto-create data/ directory after build
add_custom_command(
    TARGET Flight_Data_Intelligence POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/data
    COMMENT "Creating data/ directory"
)

# -- Test executable ----------------------------------------------
enable_testing()
add_executable(aero_tests
    tests/test_parser.cpp
    tests/test_ring_buffer.cpp
    src/parser.cpp
    src/ring_buffer.cpp
)
target_include_directories(aero_tests PRIVATE include)

# Link both ws2_32 and gtest in one line — cleaner and equivalent
target_link_libraries(aero_tests GTest::gtest_main ws2_32)

include(GoogleTest)
gtest_discover_tests(aero_tests)