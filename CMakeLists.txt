cmake_minimum_required(VERSION 2.8)
project(game-server)

find_package(Boost REQUIRED)
find_package(Protobuf REQUIRED)

#set(CMAKE_BUILD_TYPE "release")
#set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
#set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLASG} -O3 -Wall")

message(STATUS "CMake version: " ${CMAKE_VERSION})
if(NOT ${CMAKE_VERSION} VERSION_LESS "3.2")
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
else()
    message(STATUS "Checking compiler flags for C++11 support.")

    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
    check_cxx_compiler_flag("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

    if(COMPILER_SUPPORTS_CXX11)
        message(STATUS "C++11 is supported.")
        if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        endif()
    elseif(COMPILER_SUPPORTS_CXX0X)
        message(STATUD "C++0X is supported.")
        if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -stdlib=libc++")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
        endif()
    else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
    endif()
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR}/msg)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tools)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/msg)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/net)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/protocol)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/opt/spdlog/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/module)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/db/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/db)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/timer)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/net/znet/pool)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/net)

#link_directories(${CMAKE_CURRENT_SOURCE_DIR}/db/lib)
# libtools
add_subdirectory(tools)

add_subdirectory(timer)

# libmsg
add_subdirectory(msg)

# libnet
add_subdirectory(net)

# libmodule
add_subdirectory(module)

# libdb
add_subdirectory(db)


add_executable(server main.cc)
add_executable(client client/client.cc)

target_link_libraries(server tools net msg db module)
target_link_libraries(server protobuf pthread mysqlcppconn)
target_link_libraries(client protobuf pthread msg)
target_link_libraries(client tools)
#target_link_libraries(server mysqlcppconn)
