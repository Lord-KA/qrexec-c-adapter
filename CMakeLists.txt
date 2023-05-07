cmake_minimum_required(VERSION 3.1)

set(QREXEC_LIB_FLAGS "-lvchan-xen -lqrexec-utils")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${QREXEC_LIB_FLAGS}" CACHE STRING "Comment" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -D NDEBUG" CACHE STRING "Comment" FORCE)
set(CMAKE_CXX_FLAGS_SANITIZER "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wformat=2 -fsanitize=address,undefined -g" CACHE STRING "Comment" FORCE)

# Enable debug symbols by default
# must be done before project() statement
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()
# (you can also set it on the command line: -D CMAKE_BUILD_TYPE=Release)

include_directories("include")

file(GLOB qrexec_c_adapter_SRC "./src/*.c")

add_library(qrexec-c-adapter ${qrexec_c_adapter_SRC})

add_executable(example ./example/example.cpp)

target_link_libraries(example qrexec-c-adapter)
