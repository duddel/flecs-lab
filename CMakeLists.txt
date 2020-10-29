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
  ${SOURCE_DIR}/src/api_support.c
  ${SOURCE_DIR}/src/bootstrap.c
  ${SOURCE_DIR}/src/entity.c
  ${SOURCE_DIR}/src/filter.c
  ${SOURCE_DIR}/src/hierarchy.c
  ${SOURCE_DIR}/src/iter.c
  ${SOURCE_DIR}/src/log.c
  ${SOURCE_DIR}/src/map.c
  ${SOURCE_DIR}/src/misc.c
  ${SOURCE_DIR}/src/os_api.c
  ${SOURCE_DIR}/src/query.c
  ${SOURCE_DIR}/src/signature.c
  ${SOURCE_DIR}/src/sparse.c
  ${SOURCE_DIR}/src/stage.c
  ${SOURCE_DIR}/src/strbuf.c
  ${SOURCE_DIR}/src/switch_list.c
  ${SOURCE_DIR}/src/table.c
  ${SOURCE_DIR}/src/table_graph.c
  ${SOURCE_DIR}/src/type.c
  ${SOURCE_DIR}/src/vector.c
  ${SOURCE_DIR}/src/world.c
  ${SOURCE_DIR}/src/modules/stats.c
  ${SOURCE_DIR}/src/modules/timer.c
  ${SOURCE_DIR}/src/modules/pipeline/pipeline.c
  ${SOURCE_DIR}/src/modules/pipeline/worker.c
  ${SOURCE_DIR}/src/modules/system/system.c
  ${SOURCE_DIR}/src/modules/system/system_dbg.c
  ${SOURCE_DIR}/src/addons/bulk.c
  ${SOURCE_DIR}/src/addons/dbg.c
  ${SOURCE_DIR}/src/addons/module.c
  ${SOURCE_DIR}/src/addons/queue.c
  ${SOURCE_DIR}/src/addons/reader.c
  ${SOURCE_DIR}/src/addons/snapshot.c
  ${SOURCE_DIR}/src/addons/writer.c
)

set_property(SOURCE ${FLECS_SOURCES} PROPERTY GENERATED TRUE)

add_executable(${CMAKE_PROJECT_NAME}
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${FLECS_SOURCES}
)

add_dependencies(${CMAKE_PROJECT_NAME} flecs-clone)

target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC flecs_STATIC)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
  ${SOURCE_DIR}/include
)

# add the executable as a test to be run by ctest
add_test(
  NAME ${CMAKE_PROJECT_NAME}_run
  COMMAND ${CMAKE_PROJECT_NAME}
)
