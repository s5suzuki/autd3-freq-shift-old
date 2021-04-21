cmake_minimum_required(VERSION 3.10)

project(autd3-lite)
set(VERSION_MAJOR   1   CACHE STRING "Project major version number.")
set(VERSION_MINOR   0   CACHE STRING "Project minor version number.")
set(VERSION_PATCH   0   CACHE STRING "Project patch version number.")
mark_as_advanced(VERSION_MAJOR VERSION_MINOR VERSION_PATCH)

option(BUILD_DOC "BUILD_DOC" OFF)
option(USE_DOUBLE "USE_DOUBLE" OFF)
option(IGNORE_EXAMPLE "IGNORE_EXAMPLE" OFF)
option(ENABLE_LINT "ENABLE_LINT" OFF)

if(BUILD_DOC)
  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    message(STATUS "Build doc")
  else()
    message(STATUS "Doxygen is needed to build the documentation. Skip building doc...")
    set(BUILD_DOC OFF)
  endif()
endif()

if(USE_DOUBLE)
  message(STATUS "Use double precision floating point number")
  add_definitions(-DUSE_DOUBLE_AUTD)
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

if(ENABLE_LINT)
  execute_process(COMMAND git -C ${PROJECT_SOURCE_DIR} submodule update --init --recursive -- ${PROJECT_SOURCE_DIR}/deps/cpplint)
  file(GLOB_RECURSE lint_files lib/*.cpp lib/*.hpp include/*.hpp capi/*.cpp link/*.cpp link/*.hpp)
  ADD_CUSTOM_TARGET(cpplint
    python
    ${PROJECT_SOURCE_DIR}/deps/cpplint/cpplint.py
    --filter=-runtime/references,-build/c++11,-whitespace/comments
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

find_package(Git)
if(NOT Git_FOUND)
  message(WARNING "Git not found.")
endif()

add_subdirectory(osal_timer)

# Link
if(Git_FOUND)
  execute_process(COMMAND git -C ${PROJECT_SOURCE_DIR} submodule update --init --recursive -- ${PROJECT_SOURCE_DIR}/deps/SOEM/SOEM)
endif()
set(SOEM_PATH ${PROJECT_SOURCE_DIR}/deps/SOEM/SOEM)
add_subdirectory(link/soem)

if(WIN32)
  add_subdirectory(link/twincat)
endif()

if(BUILD_DOC)
  add_subdirectory(doc)
endif()

if(NOT IGNORE_EXAMPLE)
  add_subdirectory(examples)
  if(WIN32 AND BUILD_SOEM_LINK)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT example_soem)
  endif()
endif()
