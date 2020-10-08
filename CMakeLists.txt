cmake_minimum_required(VERSION 3.15)
project(Project C)

set(CMAKE_C_STANDARD 11)

add_executable(Project main.c Hashmap.c Hashmap.h  ip.c ip.h network.h util_types.h Functions.c Functions.h socket_utils.c socket_utils.h fifo_utils.c fifo_utils.h util_types.c array_queue.c array_queue.h)
