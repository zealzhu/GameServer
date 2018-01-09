set(PROTOBUF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../protobuf)
include_directories(${PROTOBUF_DIR}/include)

aux_source_directory(. PROTOCOL_SOURCE)

add_library(libprotocol ${PROTOCOL_SOURCE})
target_link_libraries(libprotocol proto)
