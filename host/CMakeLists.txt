cmake_minimum_required(VERSION 3.5)
project(raptor-host)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT DEFINED RAPTOR_DIR)
  get_filename_component(RDIR "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)
  set(RAPTOR_DIR "${RDIR}")
endif()

include_directories(${RAPTOR_DIR})
