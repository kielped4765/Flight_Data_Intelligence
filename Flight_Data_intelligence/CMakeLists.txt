cmake_minimum_required(VERSION 3.20)
project(Flight_Data_Intelligence VERSION 1.0)

# -- C++ Version --------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Memory Saftey: AddressSanitizer in Debug builds -------------
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT WIN32)
   add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
   add_link_options(-fsanitize=address)
endif()

# -- Auto-Download Google Test ------------------------------------
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# --- Main executable --------------------------------------------- 
add_executable(Flight_Data_Intelligence src/main.cpp src/parser.cpp src/ring_buffer.cpp)
target_include_directories(Flight_Data_Intelligence PRIVATE include)

# --- Test executable ---------------------------------------------
enable_testing()
add_executable(aero_tests
    tests/test_parser.cpp
    tests/test_ring_buffer.cpp
    src/parser.cpp
    src/ring_buffer.cpp
)
target_include_directories(aero_tests PRIVATE include)
target_link_libraries(aero_tests GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(aero_tests)