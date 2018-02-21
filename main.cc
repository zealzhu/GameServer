#include <config_manager.h>
#include <game_log.h>
#include <service.h>
#include <module_mgr.hpp>
#include <db_connection_pool.h>
#include <message_center.hpp>

int main()
{
    if(!GConfig.Initialize()
            || !GLog.Initialize()
            || !GService.Initialize()
            || !GModules.Initialize()
            || !GDBConnectionPool.Initialize()
            || !GMessage.Initialize())
    {
        assert(false);
    }

    GService.Start();
    GDBConnectionPool.Start();
    GModules.Start();
    GMessage.Start();

    getchar();
    GService.Stop();
    GDBConnectionPool.Stop();
    GModules.Stop();
    GMessage.Stop();

    return 0;
}
