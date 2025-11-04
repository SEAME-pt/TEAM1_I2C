cmake_minimum_required(VERSION 3.16)

# Don't create a separate project if included from parent
if(NOT CMAKE_PROJECT_NAME STREQUAL "TEAM1_Car_Control_I2C")
    project(I2C_Library LANGUAGES CXX)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =============================================================================
# I2C Library
# =============================================================================

set(I2C_SOURCES
    srcs/I2c.cpp
    srcs/I2c_PcA9685.cpp
    srcs/I2c_INA219.cpp
)

# Create static library
add_library(i2c_lib STATIC ${I2C_SOURCES})

# Public headers (accessible by parent and this lib)
target_include_directories(i2c_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Find linux/i2c-dev.h
find_path(I2C_DEV_INCLUDE linux/i2c-dev.h)
if(I2C_DEV_INCLUDE)
    target_include_directories(i2c_lib PRIVATE ${I2C_DEV_INCLUDE})
else()
    message(WARNING "linux/i2c-dev.h not found - I2C may not work")
endif()

# Compile options
target_compile_options(i2c_lib PRIVATE
    -Wall -Wextra
)

message(STATUS "I2C library configured")

# =============================================================================
# Optional: Tests
# =============================================================================
option(BUILD_I2C_TESTS "Build I2C library test programs" OFF)

if(BUILD_I2C_TESTS)
    message(STATUS "Building I2C tests...")
    
    # Test executables
    set(TEST_PROGRAMS
        main
        motor
        motor_break
        servo
        batry_test
    )
    
    foreach(test_prog ${TEST_PROGRAMS})
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test/${test_prog}.cpp)
            add_executable(test_${test_prog} test/${test_prog}.cpp)
            target_link_libraries(test_${test_prog} PRIVATE i2c_lib)
            target_include_directories(test_${test_prog} PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/include
            )
        endif()
    endforeach()
endif()