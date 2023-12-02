cmake_minimum_required(VERSION 3.22)

project(DDSHelloWorld)

find_package(fastcdr 2 REQUIRED)
find_package(fastrtps 2.12 REQUIRED)

add_compile_options(-std=c++11)

file(GLOB DDS_HELLOWORLD_SOURCES_CXX "src/idl/*.cxx")

add_executable(publisher src/Publisher.cpp ${DDS_HELLOWORLD_SOURCES_CXX})
target_link_libraries(publisher fastrtps fastcdr)

add_executable(subscriber src/Subscriber.cpp ${DDS_HELLOWORLD_SOURCES_CXX})
target_link_libraries(subscriber fastrtps fastcdr)