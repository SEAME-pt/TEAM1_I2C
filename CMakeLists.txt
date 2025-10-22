
cmake_minimum_required(VERSION 3.10)

# Incluir toolchain antes de project()
if(EXISTS "${CMAKE_SOURCE_DIR}/toolchain-rpi.cmake")
    message(STATUS "🔧 Usando toolchain para Raspberry Pi")
    include("${CMAKE_SOURCE_DIR}/toolchain-rpi.cmake")
endif()

project(I2c LANGUAGES CXX)

# Definir arquivos fonte
set(SRC
    srcs/I2c.cpp
    srcs/I2c_PcA9685.cpp
    srcs/I2c_INA219.cpp
)

# Biblioteca estática
add_library(lib_i2c STATIC ${SRC})

# Executável de testes
add_executable(testes test/main.cpp ${SRC})

