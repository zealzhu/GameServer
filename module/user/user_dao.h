#ifndef _USER_DAO_H
#define _USER_DAO_H

#include <string>

enum UserStatus
{
    kUserOffline = 0,
    kUserOnline = 1,
    kUserForbidden = 2,
};

struct User
{
    int32_t id;
    std::string account;
    std::string name;
    bool male;
    UserStatus status;
};

class UserDao
{
public:
    bool IsExist(int32_t id);
    bool IsExist(const std::string & account);
    bool GetPlayer(const std::string & account, const std::string & password, User & user);
    bool GetAccountStatus(const std::string & account, UserStatus & status);
    bool ChangeAccountStatus(const std::string & account, UserStatus status);
    bool AddNewAccount(const std::string & account, const std::string & password, const std::string & name, bool male);

private:
    std::string StatusToString(UserStatus status);
};

#endif // _USER_DAO_H
