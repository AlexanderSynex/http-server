cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME http_server)

project(${PROJECT_NAME})

if(NOT CMAKE_BUILD_TYPE)
  set(default_build_type "Debug")
  message(STATUS "Set the build type to `${default_build_type}` as none was specified.")
  set(CMAKE_BUILD_TYPE ${default_build_type} CACHE STRING "Chooce the build type." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
  "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()
message(STATUS "${PROJECT_NAME} Build Type: ${CMAKE_BUILD_TYPE}")

# Set the version for ${PROJECT_NAME}
set(${PROJECT_NAME}_Version_Major 0)
set(${PROJECT_NAME}_Version_Minor 1)
set(${PROJECT_NAME}_Version_Patch 0)
set(${PROJECT_NAME}_Version_Status "-dev")
set(PROJECT_VERSION
  "${${PROJECT_NAME}_Version_Major}.${${PROJECT_NAME}_Version_Minor}.${${PROJECT_NAME}_Version_Patch}${${PROJECT_NAME}_Version_Status}"
)
message(STATUS "${PROJECT_NAME} version: ${PROJECT_VERSION}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(third-party)

file(GLOB_RECURSE SOURCES src/**/*.cpp)

add_executable(server src/main.cpp ${SOURCES} ${TEMPLATE_SOURCES})
target_include_directories(server PRIVATE include)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
  target_link_options(server PRIVATE "-static-libstdc++" "-static-libgcc")
endif()

