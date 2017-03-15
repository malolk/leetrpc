file(GLOB leetrpc_SRCS "*.cc")

add_library(leetrpc ${leetrpc_SRCS})
target_link_libraries(leetrpc network libbase jsonutil pthread)
set_target_properties(leetrpc PROPERTIES COMPILE_FLAGS "-std=c++0x")
