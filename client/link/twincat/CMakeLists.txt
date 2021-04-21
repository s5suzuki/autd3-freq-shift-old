cmake_minimum_required(VERSION 3.10)

set (CMAKE_CXX_STANDARD 17)

include_directories(
  ./include
  ${PROJECT_SOURCE_DIR}/include
)

add_library(link_twincat STATIC
  ./src/twincatlink.cpp
  ./include/twincatlink.hpp
)

set_target_properties(link_twincat
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267 /wd4996 /wd26495 /wd26812")
endif()

if(NOT WIN32)
  set_target_properties(link_twincat PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
