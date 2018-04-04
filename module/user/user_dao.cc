#include "user_dao.h"
#include <db_help.h>
#include <game_log.h>

using namespace GameDB;

bool UserDao::IsExist(int32_t id)
{
    RecordData where;
    where["id"] = MAKE_INT_VALUE(id);
    return GDB.QueryRecord("t_player", "id", where).size() > 0;
}

bool UserDao::IsExist(const std::string & account)
{
    RecordData where;
    where["account"] = MAKE_STRING_VALUE(account);
    return GDB.QueryRecord("t_player", "id", where).size() > 0;
}

bool UserDao::GetPlayer(const std::string & account, const std::string & password, User & user)
{
    RecordData where;
    where["account"] = MAKE_STRING_VALUE(account);
    where["password"] = MAKE_STRING_VALUE(password);
    auto qr = GDB.QueryRecord("t_player", "id,account,name,sex,status", where);
    if(qr.size() == 0) return false;

    auto & qrMap = qr[0];
    user.id = atoi(qrMap["id"].c_str());
    user.account = qrMap["account"];
    user.name = qrMap["name"];
    user.male = qrMap["sex"] == "MALE" ? 1 : 0;
    auto & status = qrMap["status"];
    if(status == "ONLINE")
        user.status = kUserOnline;
    else if(status == "OFFLINE")
        user.status = kUserOffline;
    else if(status == "FORBIDDEN")
        user.status = kUserForbidden;
    return true;
}

bool UserDao::GetAccountStatus(const std::string & account, UserStatus & status)
{
    RecordData where;
    where["account"] = MAKE_STRING_VALUE(account);

    auto qr = GDB.QueryRecord("t_player", "status", where);
    if(qr.size() == 0) return false;
    auto & sta = qr[0]["status"];
    if(sta == "ONLINE")
        status = kUserOnline;
    else if(sta == "OFFLINE")
        status = kUserOffline;
    else if(sta == "FORBIDDEN")
        status = kUserForbidden;
    return true;
}

bool UserDao::ChangeAccountStatus(const std::string & account, UserStatus status)
{
    RecordData column_map;
    column_map["status"] = MAKE_STRING_VALUE(StatusToString(status));

    RecordData where_map;
    where_map["account"] = MAKE_STRING_VALUE(account);

    auto size = GDB.UpdateRecord("t_player", column_map, where_map);
    return size > 0;
}

bool UserDao::AddNewAccount(const std::string & account, const std::string & password, const std::string & name, bool male)
{
    RecordData column_map;
    column_map["account"] = MAKE_STRING_VALUE(account);
    column_map["password"] = MAKE_STRING_VALUE(password);
    column_map["name"] = MAKE_STRING_VALUE(name);
    std::string male_str = male ? "MALE" : "FEMALE";
    column_map["sex"] = MAKE_STRING_VALUE(male_str);

    auto size = GDB.InsertRecord("t_player", column_map);
    return size > 0;
}

std::string UserDao::StatusToString(UserStatus status)
{
    switch(status)
    {
    case kUserOnline:
        return "ONLINE";
    case kUserOffline:
        return "OFFLINE";
    case kUserForbidden:
        return "FORBIDDEN";
    default:
        logger_error("not found status: {}", status);
        break;
    }
    return "";
}
