cmake_minimum_required(VERSION 3.14)
project(TrueConfTest)

find_package(OpenMP)


add_executable(TrueConfTest main.cpp)
set_target_properties(TrueConfTest PROPERTIES
        CXX_STANDARD 20
        )
target_compile_options(TrueConfTest PUBLIC
        -O3
        -g -ggdb
        )
target_link_libraries(TrueConfTest PUBLIC

        )
if(OpenMP_CXX_FOUND)
    target_link_libraries(TrueConfTest PUBLIC OpenMP::OpenMP_CXX)
endif()
