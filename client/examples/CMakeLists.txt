cmake_minimum_required(VERSION 3.1)

set (CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/include)

find_package(Threads REQUIRED)

if(WIN32)
  link_libraries(ws2_32.lib)
  link_libraries(winmm.lib)
elseif(APPLE)
  link_libraries(pcap)
else()
  link_libraries(rt)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()
 
add_executable(example_soem soem.cpp)
target_link_libraries(example_soem link_soem)
target_include_directories(example_soem PUBLIC ${PROJECT_SOURCE_DIR}/link/soem/include)
