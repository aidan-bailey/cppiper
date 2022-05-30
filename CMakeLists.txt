##########
# HEADER #
##########

cmake_minimum_required(VERSION 3.10)
project(CPPIPER VERSION 0.1)

# Create options
option(DEV "Generate compiler commands and set logging to debug." OFF)

# Set options
set(PROJECT_BINARY_DIR "build")
set(PROJECT_SOURCE_DIR "src")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(GCC_COVERAGE_COMPILE_FLAGS "-I/usr/lib/gcc/x86_64-pc-linux-gnu/11.2.0/include -I/usr/include/c++/11.2.0/include -pthreads")
if(DEV)
    set(CMAKE_EXPORT_COMPILE_COMMANDS True)
endif()

# Configure injection header
configure_file(src/cppiperconfig.hh.in cppiperconfig.hh)

# Find dependencies
find_package(spdlog REQUIRED) # logging

###############
# COMPILATION #
###############

# Dynamic library
add_library(
  cppiper
  SHARED
  src/sender.cc
  src/receiver.cc
  )
set_target_properties(cppiper PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(cppiper PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           include
                           )
#target_link_libraries(cppiper PRIVATE spdlog::spdlog)

# Static library
add_library(
  cppiper_static
  STATIC
  src/sender.cc
  src/receiver.cc
  )
set_target_properties(cppiper_static PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(cppiper_static PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           include
                           )
target_link_libraries(cppiper_static PRIVATE spdlog::spdlog)

################
# INSTALLATION #
################

install(TARGETS cppiper DESTINATION lib)
install(TARGETS cppiper_static DESTINATION lib)
install(FILES "${PROJECT_BINARY_DIR}/cppiperconfig.h"
  DESTINATION include
)

###########
# TESTING #
###########

add_executable(test test/test.cc)
target_include_directories(test PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           include
                           )
target_link_libraries(test PUBLIC cppiper_static)
