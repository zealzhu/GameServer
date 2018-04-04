/**
 * @file StringUtils.h
 * @brief string帮助类
 * @author zhu peng cheng
 * @version 1.0
 * @date 2018-01-15
 */
#ifndef _STRING_UTILS_H
#define _STRING_UTILS_H

#include <string>
#include <sstream>

namespace StringUtils
{

std::string IntToString(int value);

bool IsValidAccount(const std::string& strAccount);

bool IsValidPassword(const std::string& strPassword);

bool IsValidMail(const std::string& strMail);

bool IsValidMobile(const std::string& strMobile);

bool IsValidHomeNumber(const std::string& strHomeNum);

}

#endif // _STRING_UTILS_H
