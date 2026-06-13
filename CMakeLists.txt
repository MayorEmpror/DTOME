cmake_minimum_required(VERSION 3.20)
project(DTOME VERSION 1.0.0 LANGUAGES CXX)

# ─── C++ standard ────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ─── Build type default ──────────────────────────────────────────────────────
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler   : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

# ─── Compiler flags ──────────────────────────────────────────────────────────
set(DTOME_COMPILE_OPTIONS
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wno-unused-parameter
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND DTOME_COMPILE_OPTIONS -g -O0 -fsanitize=address,undefined)
    set(DTOME_LINK_OPTIONS -fsanitize=address,undefined)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    list(APPEND DTOME_COMPILE_OPTIONS -O3 -DNDEBUG -march=native)
endif()

# ─── Source discovery ────────────────────────────────────────────────────────
#
#   Expected layout inside Components/:
#
#     Components/
#       Exchange/          Exchange.h  Exchange.cpp
#       Market/            Market.h    Market.cpp
#       MatchingEngine/    MatchingEngine.h  MatchingEngine.cpp
#       OrderBook/         OrderBook.h  OrderBook.cpp  PriceLevel.h
#       Order/             Order.h     Order.cpp
#       OrderQueue/        OrderQueue.h  OrderQueue.cpp
#       Trade/             Trade.h     Trade.cpp
#
file(GLOB_RECURSE DTOME_SOURCES
    "${CMAKE_SOURCE_DIR}/Components/*.cpp"
)

file(GLOB_RECURSE DTOME_HEADERS
    "${CMAKE_SOURCE_DIR}/Components/*.h"
)

# ─── Core library ────────────────────────────────────────────────────────────
#   Built as a static lib so both the main binary and the test binary
#   link against the same compiled objects without re-compiling.
#
add_library(dtome_core STATIC ${DTOME_SOURCES})

target_include_directories(dtome_core
    PUBLIC
        "${CMAKE_SOURCE_DIR}/Components/Exchange"
        "${CMAKE_SOURCE_DIR}/Components/Market"
        "${CMAKE_SOURCE_DIR}/Components/MatchingEngine"
        "${CMAKE_SOURCE_DIR}/Components/OrderBook"
        "${CMAKE_SOURCE_DIR}/Components/Order"
        "${CMAKE_SOURCE_DIR}/Components/OrderQueue"
        "${CMAKE_SOURCE_DIR}/Components/Trade"
)

target_compile_options(dtome_core PRIVATE ${DTOME_COMPILE_OPTIONS})

if(DEFINED DTOME_LINK_OPTIONS)
    target_link_options(dtome_core PUBLIC ${DTOME_LINK_OPTIONS})
endif()

# Pthreads — required for std::mutex / std::condition_variable on Linux
find_package(Threads REQUIRED)
target_link_libraries(dtome_core PUBLIC Threads::Threads)

# ─── Main test binary ────────────────────────────────────────────────────────
add_executable(DTOME "${CMAKE_SOURCE_DIR}/test.cpp")
target_link_libraries(DTOME PRIVATE dtome_core)
target_compile_options(DTOME PRIVATE ${DTOME_COMPILE_OPTIONS})

# ─── Unit tests (optional — only built if tests/ directory exists) ───────────
#
#   Drop any *_test.cpp file under tests/ and it will be picked up
#   automatically.  Each file becomes its own CTest test case.
#
if(EXISTS "${CMAKE_SOURCE_DIR}/tests")
    enable_testing()
    file(GLOB TEST_SOURCES "${CMAKE_SOURCE_DIR}/tests/*_test.cpp")
    foreach(TEST_FILE ${TEST_SOURCES})
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
        add_executable(${TEST_NAME} ${TEST_FILE})
        target_link_libraries(${TEST_NAME} PRIVATE dtome_core)
        target_compile_options(${TEST_NAME} PRIVATE ${DTOME_COMPILE_OPTIONS})
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endforeach()
    message(STATUS "Tests      : ${TEST_SOURCES}")
else()
    message(STATUS "Tests      : none (create tests/ to enable)")
endif()

# ─── Install rules (optional) ────────────────────────────────────────────────
install(TARGETS DTOME DESTINATION bin)
install(TARGETS dtome_core
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
install(FILES ${DTOME_HEADERS} DESTINATION include/dtome)

# ─── Print summary ───────────────────────────────────────────────────────────
message(STATUS "Sources    : ${DTOME_SOURCES}")