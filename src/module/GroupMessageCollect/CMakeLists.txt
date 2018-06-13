include_directories(
	"${PROJECT_SOURCE_DIR}/src/interface"
	"${PROJECT_SOURCE_DIR}/src/public"
	"${PROJECT_SOURCE_DIR}/src/proto"
	"${PROJECT_SOURCE_DIR}/include/protobuf"
	"${PROJECT_SOURCE_DIR}/include/spdlog"
	)

set(GROUP_MESSAGE_COLLECT
	"${CMAKE_CURRENT_SOURCE_DIR}/GroupMessageCollect.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/GroupMessageCollect.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
	)
	
source_group(public FILES ${MODULE_PUBLIC})
source_group(interface FILES ${MODULE_INTERFACE})
source_group(\\ FILES ${GROUP_MESSAGE_COLLECT})
source_group(proto FILES ${MODULE_PROTO})
	
add_library(GroupMessageCollect SHARED
	${GROUP_MESSAGE_COLLECT}
	${MODULE_INTERFACE}
	${MODULE_PUBLIC}
	)
	
if(WIN32)
    set_property(TARGET GroupMessageCollect PROPERTY RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/windows)
else()
    set(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build/linux/lib")
endif()
set_property(TARGET GroupMessageCollect PROPERTY FOLDER "module")
