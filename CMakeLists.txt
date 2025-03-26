cmake_minimum_required(VERSION 3.10)
project(UdpTest)

set(CMAKE_CXX_STANDARD 17)

# Specify the include directory
include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(udp_client udp_client.cpp src/udp_socket.cpp src/udp_client_tester.cpp src/timer.cpp)
add_executable(udp_server udp_server.cpp src/udp_socket.cpp src/udp_server_tester.cpp src/timer.cpp)

target_include_directories(udp_client PRIVATE .)
target_include_directories(udp_server PRIVATE .)

target_link_libraries(udp_client pthread)
target_link_libraries(udp_server pthread)


