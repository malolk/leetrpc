add_executable(echo_server echo_server.cc)
target_link_libraries(echo_server network libbase jsonutil leetrpc)

add_executable(echo_client echo_client.cc)
target_link_libraries(echo_client network libbase jsonutil leetrpc)
