cmake_minimum_required(VERSION 3.15)
project(vectordb)




set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(pybind11 CONFIG REQUIRED)
find_package(Threads REQUIRED)

add_executable(cpp_bench src/main.cpp)

pybind11_add_module(vectordb
    src/main.cpp
    src/bindings.cpp
)
target_link_libraries(vectordb PRIVATE Threads::Threads)