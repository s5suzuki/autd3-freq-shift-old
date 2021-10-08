cmake_minimum_required(VERSION 3.16)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt. ")
endif()

project(autd3-freq-shift)
set(VERSION_MAJOR   0   CACHE STRING "Project major version number.")
set(VERSION_MINOR   2   CACHE STRING "Project minor version number.")
set(VERSION_PATCH   0   CACHE STRING "Project patch version number.")
mark_as_advanced(VERSION_MAJOR VERSION_MINOR VERSION_PATCH)

set (CMAKE_CXX_STANDARD 17)

option(BUILD_DOC "BUILD_DOC" OFF)
option(BUILD_EXAMPLES "BUILD_EXAMPLES" ON)
option(ENABLE_LINT "ENABLE_LINT" OFF)

option(BUILD_SHARED_LIBS "BUILD_SHARED_LIBS" OFF)
option(BUILD_WITH_STATIC_CRT "BUILD_WITH_STATIC_CRT" OFF)

option(USE_SYSTEM_EIGEN "USE_SYSTEM_EIGEN" OFF)

if(BUILD_DOC)
  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    message(STATUS "Build doc")
  else()
    message(STATUS "Doxygen is needed to build the documentation. Skip building doc...")
    set(BUILD_DOC OFF)
  endif()
endif()

if(WIN32)
  set(OS win32)
  include(cmakes/win_vs.cmake)
elseif(APPLE)
  set(OS macosx)
  include(cmakes/mac.cmake)
else()
  set(OS linux)
  include(cmakes/linux.cmake)
endif()

add_compile_definitions(_USE_MATH_DEFINES)
if(WIN32)
  add_compile_definitions(NOMINMAX)
  add_compile_definitions(__STDC_LIMIT_MACROS)
endif()

find_package(Git)
if(NOT Git_FOUND)
  message(WARNING "Git not found.")
endif()

if(ENABLE_LINT)
  if(Git_FOUND)
  execute_process(COMMAND git -C ${PROJECT_SOURCE_DIR} submodule update --init --recursive -- ${PROJECT_SOURCE_DIR}/deps/cpplint)
  endif()
  file(GLOB_RECURSE lint_files lib/*.cpp lib/*.hpp include/*.hpp capi/*.cpp capi/*.hpp examples/*.cpp examples/*.hpp)
  ADD_CUSTOM_TARGET(cpplint
    python
    ${PROJECT_SOURCE_DIR}/deps/cpplint/cpplint.py
    --filter=-runtime/references,-build/c++11,-whitespace/comments,-readability/braces
    --linelength=150
    ${lint_files}
  )
endif()

if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

if(POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif()

if(NOT USE_SYSTEM_EIGEN)
  if(Git_FOUND)
    execute_process(COMMAND git -C ${PROJECT_SOURCE_DIR} submodule update --init --recursive -- ${PROJECT_SOURCE_DIR}/deps/Eigen3)
    set(EIGEN_PATH ${PROJECT_SOURCE_DIR}/deps/Eigen3)
  endif()
endif()
add_compile_definitions(EIGEN_MPL2_ONLY)

# Link
if(Git_FOUND)
  execute_process(COMMAND git -C ${PROJECT_SOURCE_DIR} submodule update --init --recursive -- ${PROJECT_SOURCE_DIR}/deps/SOEM)
endif()
set(SOEM_PATH ${PROJECT_SOURCE_DIR}/deps/SOEM)
add_subdirectory(lib/link/soem)

if(NOT IGNORE_EXAMPLE)
  add_subdirectory(examples)
  if(WIN32 AND BUILD_SOEM_LINK)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT example_soem)
  endif()
endif()
