cmake_minimum_required(VERSION 3.9)
project(Code)

set(CMAKE_CXX_STANDARD 11)

add_executable(Code
        .idea/codeStyles/Project.xml
        .idea/Code.iml
        .idea/misc.xml
        .idea/modules.xml
        .idea/workspace.xml
        cmake-build-debug/CMakeCache.txt
        CMakeLists.txt
        main.c
        ip.c Hashmap.c socket.c tcp.c byte_queue.c packet_handlers.c packet_handlers.h connection_queue.c connection_queue.h)
