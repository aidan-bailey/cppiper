cmake_minimum_required(VERSION 3.10)
project(CPPIPER)

option(DEV "Generate compiler commands and set logging to debug." OFF)
option(DOC "Generate documentation." OFF)

set(CPPIPER_VERSION_MAJOR 0)
set(CPPIPER_VERSION_MINOR 1)
set(CPPIPER_VERSION ${CPPIPER_VERSION_MAJOR}.${CPPIPER_VERSION_MINOR})
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
if(DEV)
    set(CMAKE_EXPORT_COMPILE_COMMANDS True)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

configure_file(src/cppiperconfig.hh.in cppiperconfig.hh)

include(GenerateExportHeader)

########################
# LIBRARY DECLARATIONS #
########################

# Dependencies
find_package (glog REQUIRED)

# Dynamic Lib
add_library(cppiper SHARED src/pipemanager.cc  src/receiver.cc src/sender.cc)
target_link_libraries(cppiper PUBLIC glog::glog)
generate_export_header(cppiper)
set_property(TARGET cppiper PROPERTY VERSION ${CPPIPER_VERSION})
#set_property(TARGET cppiper PROPERTY SOVERSION 3)
#set_property(TARGET cppiper PROPERTY
#  INTERFACE_CPPIPER_MAJOR_VERSION 3)
#set_property(TARGET cppiper APPEND PROPERTY
#  COMPATIBLE_INTERFACE_STRING CPPIPER_MAJOR_VERSION
#)

# Static Lib
add_library(cppiper_static STATIC src/pipemanager.cc  src/receiver.cc src/sender.cc)
target_link_libraries(cppiper_static PUBLIC glog::glog)
generate_export_header(cppiper_static)
set_property(TARGET cppiper_static PROPERTY VERSION ${CPPIPER_VERSION})
#set_property(TARGET cppiper_static PROPERTY SOVERSION 3)
#set_property(TARGET cppiper_static PROPERTY
#  INTERFACE_CPPIPER_MAJOR_VERSION 3)
#set_property(TARGET cppiper_static APPEND PROPERTY
#  COMPATIBLE_INTERFACE_STRING CPPIPER_MAJOR_VERSION
#)

################
# INSTALLATION #
################

install(TARGETS cppiper EXPORT cppiper-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

install(TARGETS cppiper_static EXPORT cppiper-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

install(
  FILES
    include/pipemanager.hh
    include/receiver.hh
    include/sender.hh
    "${CMAKE_CURRENT_BINARY_DIR}/cppiper_export.h"
    "${CMAKE_CURRENT_BINARY_DIR}/cppiperconfig.hh"
  DESTINATION
    include/cppiper
  COMPONENT
    Devel
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/cppiper/cppiper-config-version.cmake"
  VERSION ${CPPIPER_VERSION}
  COMPATIBILITY AnyNewerVersion
)

export(EXPORT cppiper-targets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/cppiper/cppiper-targets.cmake"
  NAMESPACE cppiper::
)
configure_file(cmake/cppiper-config.cmake
  "${CMAKE_CURRENT_BINARY_DIR}/cppiper/cppiper-config.cmake"
  COPYONLY
)

set(ConfigPackageLocation lib/cmake/cppiper)
install(EXPORT cppiper-targets
  FILE
    cppiper-targets.cmake
  NAMESPACE
    cppiper::
  DESTINATION
    ${ConfigPackageLocation}
)

install(
  FILES
    cmake/cppiper-config.cmake
    "${CMAKE_CURRENT_BINARY_DIR}/cppiper/cppiper-config-version.cmake"
  DESTINATION
    ${ConfigPackageLocation}
  COMPONENT
    Devel
)

########################
# BENCHMARK EXECUTABLE #
########################

add_executable(benchmark benchmark/benchmark.cc)
target_include_directories(benchmark PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           include
                           )
target_link_libraries(benchmark PUBLIC cppiper)

##################
# DOC GENERATION #
##################

if (DOC)
  find_package(Doxygen)
  if (DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
  else (DOXYGEN_FOUND)
    message("Doxygen need to be installed to generate the doxygen documentation")
  endif (DOXYGEN_FOUND)
endif()
