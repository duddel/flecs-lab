cmake_minimum_required(VERSION 3.6)

project(flecs-lab)

# enforce C++11 and disable extensions
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()
include(ExternalProject)

ExternalProject_Add(flecs-clone
  GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
  GIT_TAG v2.2.0
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  GIT_SHALLOW ON
)

ExternalProject_Get_property(flecs-clone SOURCE_DIR)

list(APPEND FLECS_SOURCES
  ${SOURCE_DIR}/flecs.c
)

set_property(SOURCE ${FLECS_SOURCES} PROPERTY GENERATED TRUE)

add_executable(${CMAKE_PROJECT_NAME}
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${FLECS_SOURCES}
)

add_dependencies(${CMAKE_PROJECT_NAME} flecs-clone)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
  ${SOURCE_DIR}
)

# add the executable as a test to be run by ctest
add_test(
  NAME ${CMAKE_PROJECT_NAME}_run
  COMMAND ${CMAKE_PROJECT_NAME}
)
