file(GLOB SUB_MODULE_DIRS "${PROJECT_SOURCE_DIR}/src/module/*")
file(GLOB MODULE_INTERFACE "${PROJECT_SOURCE_DIR}/src/interface/*.*")
file(GLOB MODULE_PUBLIC "${PROJECT_SOURCE_DIR}/src/public/*.*")
file(GLOB MODULE_PROTO "${PROJECT_SOURCE_DIR}/src/proto/*.*")

if(WIN32)
set(THIRD_LIB_DEBUG
	debug "${PROJECT_SOURCE_DIR}/lib/windows/protobuf/Debug/libprotobufd.lib"
	debug "${PROJECT_SOURCE_DIR}/lib/windows/mysql/debug/libmysqld.lib"
	)
	
set(THIRD_LIB_RELEASE
	optimized "${PROJECT_SOURCE_DIR}/lib/windows/protobuf/Release/libprotobuf.lib"
	optimized "${PROJECT_SOURCE_DIR}/lib/windows/mysql/release/libmysqld.lib"
	)
endif()

foreach(SUB_MODULE_DIR ${SUB_MODULE_DIRS})
	message("添加子目录：${SUB_MODULE_DIR}")
	string(COMPARE NOTEQUAL ${SUB_MODULE_DIR} "${PROJECT_SOURCE_DIR}/src/module/CMakeLists.txt" NOT_SELF)
	if(NOT_SELF)
		ADD_SUBDIRECTORY(${SUB_MODULE_DIR})
	endif()
endforeach()