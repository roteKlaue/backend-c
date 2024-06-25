cmake_minimum_required(VERSION 3.15)
project(WMCCBackend C)

set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/build/build/generators" ${CMAKE_PREFIX_PATH})

find_package(mongoc-1.0 REQUIRED)

add_executable(${PROJECT_NAME} ./src/main.c ./src/map.c ./src/routes.c ./src/mongoc-setup.c ./src/util.c)
target_link_libraries(${PROJECT_NAME} mongo::mongoc_static)