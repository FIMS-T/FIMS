cmake_minimum_required(VERSION 3.18)
project(FIMS VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable SIMD instructions
if(MSVC)
    add_compile_options(/arch:AVX2)
else()
    add_compile_options(-mavx2 -march=native)
endif()

include(FetchContent)

# --------------------------
# 1. Dependencies
# --------------------------

# ASIO
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-30-2
)
FetchContent_MakeAvailable(asio)

# miniupnpc
FetchContent_Declare(
    miniupnpc
    GIT_REPOSITORY https://github.com/miniupnp/miniupnp.git
    GIT_TAG miniupnpc_2_3_3
    SOURCE_SUBDIR miniupnpc
)
set(UPNPC_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(UPNPC_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(UPNPC_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(miniupnpc)

# Monocypher
FetchContent_Declare(
    monocypher
    GIT_REPOSITORY https://github.com/LoupVaillant/Monocypher.git
    GIT_TAG 4.0.2
)
FetchContent_MakeAvailable(monocypher)
if(NOT TARGET monocypher)
    add_library(monocypher STATIC ${monocypher_SOURCE_DIR}/src/monocypher.c ${monocypher_SOURCE_DIR}/src/optional/monocypher-ed25519.c)
    target_include_directories(monocypher PUBLIC ${monocypher_SOURCE_DIR}/src ${monocypher_SOURCE_DIR}/src/optional)
endif()

# SDL2
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.2
)
FetchContent_MakeAvailable(SDL2)

# Google Test
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# --------------------------
# 2. Build the Core Library (Logic)
# --------------------------
# We put all logic here EXCEPT main.cpp files
file(GLOB_RECURSE LIB_SOURCES
    "src/network/*.cpp" 
    "src/local/*.cpp" 
    "src/ui/*.cpp"
)

add_library(fims_core STATIC ${LIB_SOURCES})
target_include_directories(fims_core PUBLIC src include ${asio_SOURCE_DIR}/include)
target_compile_definitions(fims_core PUBLIC ASIO_STANDALONE SDL_MAIN_HANDLED)
target_link_libraries(fims_core PUBLIC
    libminiupnpc-static
    monocypher
    SDL2::SDL2-static
)

# --------------------------
# 3. Main Executable
# --------------------------
add_executable(FIMS src/main.cpp)
target_link_libraries(FIMS PRIVATE fims_core)

if(WIN32)
    target_link_libraries(FIMS PRIVATE ws2_32 iphlpapi shlwapi user32 gdi32 winmm imm32 ole32 oleaut32 shell32 advapi32 version uuid)
else()
    target_link_libraries(FIMS PRIVATE pthread dl)
endif()

# --------------------------
# 4. Tests
# --------------------------
enable_testing()

# Find all test files, including tests/main.cpp
file(GLOB_RECURSE TEST_FILES "tests/*.cpp")

add_executable(fims_tests ${TEST_FILES})
target_link_libraries(fims_tests PRIVATE fims_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(fims_tests)
