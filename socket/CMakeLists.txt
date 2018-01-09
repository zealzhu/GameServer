aux_source_directory(. GAME_SOCKET_SOURCE)
add_library(libsocket ${GAME_SOCKET_SOURCE})
target_link_libraries(libsocket libprotocol)
