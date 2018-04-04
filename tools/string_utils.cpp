/**
 * @file StringUtils.cpp
 * @brief string帮助类
 * @author zhu peng cheng
 * @version 1.0
 * @date 2018-01-15
 */
#include "string_utils.h"
#include <regex.h>

using namespace std;

namespace StringUtils
{

string IntToString(int value)
{
    ostringstream os;
    os << value;
    return os.str();
}

bool RegexMatch(const std::string & str, const std::string & patten)
{
    bool ret = false;
    regex_t reg;
    int rnt = regcomp(&reg, patten.c_str(), REG_NOSUB | REG_EXTENDED | REG_ICASE);
    if(rnt) return false;

    rnt = regexec(&reg, str.c_str(), 0, NULL, 0);
    if(rnt == REG_NOERROR) ret = true;
    else ret = false;
    regfree(&reg);
    return ret;
}

bool IsValidAccount(const string& account)
{
    std::string patten = "^[a-z0-9_-]{3,16}$";
    return RegexMatch(account, patten);
}

bool IsValidPassword(const string& password)
{
    std::string patten = "^[a-z0-9_-]{3,16}$";
    return RegexMatch(password, patten);
}

bool IsValidMail(const std::string& mail)
{
    std::string patten("([0-9A-Za-z\\-_\\.]+)@([0-9a-z]+\\.[a-z]{2,3}(\\.[a-z]{2})?)");
    return RegexMatch(mail, patten);

}

bool IsValidMobile(const string& mobile)
{
    std::string patten("^1[34578]\\d{9}$");
    return RegexMatch(mobile, patten);
}

}
