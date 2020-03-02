cmake_minimum_required (VERSION 3.1)
project (ASYNC C)

enable_testing()

#set(CMAKE_C_STANDARD ...)
set(CMAKE_C_FLAGS "-g -Wall -Wextra -pthread")

# http://stackoverflow.com/questions/10555706/
macro (add_executable _name)
  # invoke built-in add_executable
  _add_executable(${ARGV})
  if (TARGET ${_name})
    target_link_libraries(${_name} asyncc)
  endif()
endmacro()

include_directories(include)
add_library(asyncc STATIC err.c threadpool.c future.c work_queue.c work_queue.h pool_collection.c pool_collection.h runnable.h)
add_executable(macierz macierz.c)
add_executable(silnia silnia.c)
add_subdirectory(test)

install(TARGETS asyncc DESTINATION .)
