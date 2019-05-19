cmake_minimum_required(VERSION 3.8.2)

if ($ENV{TRAVIS_BUILD_NUMBER})
    project(cmd VERSION 1.0.$ENV{TRAVIS_BUILD_NUMBER})
else()
    project(cmd VERSION 1.0.0)
endif()

message("-- Looking for async")

if ($ENV{TRAVIS_BUILD_NUMBER})
    set(ASYNC_INCLUDE_DIR "/usr/include")
    set(ASYNC_LIBRARY     "/usr/lib/libasync.so")
else()
    set(ASYNC_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
    set(ASYNC_LIBRARY     "${CMAKE_CURRENT_SOURCE_DIR}/lib/libasync.so")
endif()

message("-- ASYNC_INCLUDE_DIR=" ${ASYNC_INCLUDE_DIR})
message("-- ASYNC_LIBRARY="     ${ASYNC_LIBRARY})

################################################################################

find_package(Threads)
find_package(Boost COMPONENTS system REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
include_directories(${ASYNC_INCLUDE_DIR})

add_executable(bulk_server server.cpp)

set_target_properties(bulk_server PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra"
)

target_link_libraries(bulk_server 
                                  ${CMAKE_THREAD_LIBS_INIT} 
                                  ${Boost_LIBRARIES} 
                                  ${ASYNC_LIBRARY})

################################################################################

install (TARGETS bulk_server RUNTIME DESTINATION bin)

################################################################################

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_NAME bulk_server)
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64)
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Anton Sochnev <an.sochnev@yandex.ru>")
set(CPACK_DEBIAN_PACKAGE_SECTION "homework")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION 
"bulk_server
 bulk_server."
)
set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION})

include(CPack)
