cmake_minimum_required(VERSION 3.16)

set (CMAKE_CXX_STANDARD 17)

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
target_link_libraries(example_soem soem_link)
target_include_directories(example_soem PRIVATE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/link/soem/include ${EIGEN_PATH})
