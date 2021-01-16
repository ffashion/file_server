cmake_minimum_required(VERSION 3.5.1)

add_executable(server server.c server.h)
add_executable(client client.c client.h)

add_executable(test test.c)