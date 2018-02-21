aux_source_directory(user/ GAME_MODULE_SOURCE)
aux_source_directory(room/ GAME_MODULE_SOURCE)

add_library(module ${GAME_MODULE_SOURCE})
target_link_libraries(module msg)
