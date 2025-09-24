cmake_minimum_required(VERSION 3.10)
project(IIC_DRIVERS C CXX)

# c lib
add_library(iicx_c_lib  STATIC src/iic.c)
target_include_directories(iicx_c_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/c
)

#c++ lib
add_library(iicx_c++_lib  STATIC src/iicc++.cpp)
target_include_directories(iicx_c++_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/c++
)

#test
add_executable(iic_test test.cpp)
target_link_libraries(iic_test
    PRIVATE iicx_c_lib
    PRIVATE iicx_c++_lib
)
