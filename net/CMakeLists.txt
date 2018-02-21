aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/. GAME_NET_SOURCE)
add_library(net STATIC ${GAME_NET_SOURCE})
target_link_libraries(net tools)
