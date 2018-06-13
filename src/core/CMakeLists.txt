include_directories(
	"${PROJECT_SOURCE_DIR}/src/core"
	"${PROJECT_SOURCE_DIR}/src/spdlog"
	"${PROJECT_SOURCE_DIR}/src/interface"
	"${PROJECT_SOURCE_DIR}/src/public"
	"${PROJECT_SOURCE_DIR}/src/proto"
	"${PROJECT_SOURCE_DIR}/include/asio"
	"${PROJECT_SOURCE_DIR}/include/protobuf"
	"${PROJECT_SOURCE_DIR}/include/spdlog"
	"${PROJECT_SOURCE_DIR}/include/mysql"
	)
	
if(WIN32)
set(THIRD_LIB_DEBUG
	debug "${PROJECT_SOURCE_DIR}/lib/windows/protobuf/Debug/libprotobufd.lib"
	debug "${PROJECT_SOURCE_DIR}/lib/windows/mysql/release/libmysql.lib"
	)
	
set(THIRD_LIB_RELEASE
	optimized "${PROJECT_SOURCE_DIR}/lib/windows/protobuf/Release/libprotobuf.lib"
	optimized "${PROJECT_SOURCE_DIR}/lib/windows/mysql/release/libmysqld.lib"
	)
endif()

set(CORE
	"${PROJECT_SOURCE_DIR}/src/core/Context.h"
	"${PROJECT_SOURCE_DIR}/src/core/Context.cpp"
	"${PROJECT_SOURCE_DIR}/src/core/main.cpp"
	)
	
set(CORE_PROTO
	"${PROJECT_SOURCE_DIR}/src/proto/message.pb.h"
	"${PROJECT_SOURCE_DIR}/src/proto/message.pb.cc"
	)
	
set(CORE_INTERFACE
	"${PROJECT_SOURCE_DIR}/src/interface/IContext.h"
	"${PROJECT_SOURCE_DIR}/src/interface/IModule.h"
	"${PROJECT_SOURCE_DIR}/src/interface/IMsgMgr.h"
	"${PROJECT_SOURCE_DIR}/src/interface/ITimer.h"
	"${PROJECT_SOURCE_DIR}/src/interface/IDB.h"
	)

set(CORE_PUBLIC
	"${PROJECT_SOURCE_DIR}/src/public/Tools.hpp"
	)

set(CORE_MSG
	"${PROJECT_SOURCE_DIR}/src/core/msg/MsgQueue.hpp"
	"${PROJECT_SOURCE_DIR}/src/core/msg/MsgMgr.h"
	"${PROJECT_SOURCE_DIR}/src/core/msg/MsgMgr.cpp"
	)
	
set(CORE_NET
	"${PROJECT_SOURCE_DIR}/src/core/net/TcpServer.h"
	"${PROJECT_SOURCE_DIR}/src/core/net/TcpServer.cpp"
	"${PROJECT_SOURCE_DIR}/src/core/net/Session.h"
	"${PROJECT_SOURCE_DIR}/src/core/net/Session.cpp"
	"${PROJECT_SOURCE_DIR}/src/core/net/Code.hpp"
	)
	
set(CORE_MODULE
	"${PROJECT_SOURCE_DIR}/src/core/module/ModuleMgr.h"
	"${PROJECT_SOURCE_DIR}/src/core/module/ModuleMgr.cpp"
	)

set(CORE_TIMER
	"${PROJECT_SOURCE_DIR}/src/core/timer/TimerQueue.h"
	"${PROJECT_SOURCE_DIR}/src/core/timer/TimerQueue.cpp"
	)
	
set(CORE_CONFIG
	"${PROJECT_SOURCE_DIR}/src/core/config/Config.h"
	"${PROJECT_SOURCE_DIR}/src/core/config/Config.cpp"
	"${PROJECT_SOURCE_DIR}/src/core/config/ConfigMgr.h"
	"${PROJECT_SOURCE_DIR}/src/core/config/ConfigMgr.cpp"
	"${PROJECT_SOURCE_DIR}/src/core/config/ini.h"
	"${PROJECT_SOURCE_DIR}/src/core/config/ini.c"
	"${PROJECT_SOURCE_DIR}/src/core/config/INIReader.h"
	"${PROJECT_SOURCE_DIR}/src/core/config/INIReader.cpp"
	)
	
set(CORE_LOG
	"${PROJECT_SOURCE_DIR}/src/core/log/Logger.h"
	"${PROJECT_SOURCE_DIR}/src/core/log/Logger.cpp"
	)
	
set(CORE_DB
	"${PROJECT_SOURCE_DIR}/src/core/db/DBConnectionPool.h"
	"${PROJECT_SOURCE_DIR}/src/core/db/DBConnectionPool.cpp"
	"${PROJECT_SOURCE_DIR}/src/core/db/DBHelper.h"
	"${PROJECT_SOURCE_DIR}/src/core/db/DBHelper.cpp"
	)

source_group(public FILES ${CORE_PUBLIC})
source_group(proto FILES ${CORE_PROTO})

source_group(src FILES ${CORE})
source_group(src\\interface FILES ${CORE_INTERFACE})
source_group(src\\log FILES ${CORE_LOG})
source_group(src\\config FILES ${CORE_CONFIG})
source_group(src\\module FILES ${CORE_MODULE})
source_group(src\\net FILES ${CORE_NET})
source_group(src\\msg FILES ${CORE_MSG})
source_group(src\\timer FILES ${CORE_TIMER})
source_group(src\\db FILES ${CORE_DB})

add_executable(server
	${CORE}
	${CORE_INTERFACE}
	${CORE_PUBLIC}
	${CORE_PROTO}
	${CORE_MSG}
	${CORE_NET}
	${CORE_MODULE}
	${CORE_CONFIG}
	${CORE_LOG}
	${CORE_TIMER}
	${CORE_DB}
	)
	
target_link_libraries(server
	${THIRD_LIB_DEBUG}
	${THIRD_LIB_RELEASE}
	)

if(UNIX)
    target_link_libraries(server pthread protobuf dl mysqlclient)
    set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build/linux")
else()
    set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build/windows")
	set_target_properties(server PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/build/windows/Debug")
endif()

