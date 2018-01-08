set(INPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB PROTO_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/*.proto)

set(PROTOBUF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../protobuf)
set(PROTOC ${PROTOBUF_DIR}/lib/protoc)
set(PROTOC_C_OUT_FLAG --cpp_out)
set(PROTO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR})
message(STATUS "PROTOC = ${PROTOC}")

foreach(PROTO_FILE ${PROTO_INPUT})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)

    set(CUR_PROTO_GEN
        ${PROTO_GEN_DIR}/${PROTO_NAME}.pb.h
        ${PROTO_GEN_DIR}/${PROTO_NAME}.pb.cc
    )
    set(PROTO_GEN
        ${PROTO_GEN}
        ${CUR_PROTO_GEN}
    )
    message(STATUS "CUR_PROTO_GEN = ${CUR_PROTO_GEN}")

    add_custom_command(
        OUTPUT ${CUR_PROTO_GEN}
        COMMAND ${PROTOC} ${PROTO_FILE} ${PROTOC_C_OUT_FLAG} ${PROTO_GEN_DIR} -I${INPUT_DIR}
        DEPENDS ${PROTOC} ${PROTO_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endforeach(PROTO_FILE ${PROTO_INPUT})

include_directories(${PROTOBUF_DIR}/include)
link_directories(${PROTOBUF_DIR}/lib)

add_library(proto STATIC ${PROTO_GEN})
target_link_libraries(proto gprotobuf)
