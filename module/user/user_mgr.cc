#include "user_mgr.h"
#include "protobuf_processor.hpp"
#include "event_processor.hpp"
#include <string_utils.h>

using namespace user;
using namespace StringUtils;

bool UserMgr::Initialize()
{
    RegProtoHandler(this, kC2SLogin, &UserMgr::Login);
    RegProtoHandler(this, kC2SLogout, &UserMgr::Logout);
    RegProtoHandler(this, kC2SRegister, &UserMgr::Register);

    RegEvent(this, kEventConnectionClose, &UserMgr::OnConnectClose);
    return true;
}

bool UserMgr::Start()
{}

void UserMgr::Stop()
{}

void UserMgr::Login(ISender * sender, LoginReq & login_req)
{
    LoginResp rsp;
    std::string account = login_req.account();
    std::string password = login_req.password();

    if(!user_dao_.IsExist(account)) {
        rsp.set_code(ACCOUNT_NOT_EXIST);
        sender->Send(kS2CLogin, rsp);
        return;
    }

    User user;
    if(!user_dao_.GetPlayer(account, password, user)) {
        rsp.set_code(ACCOUNT_OR_PASSWD_ERROR);
        sender->Send(kS2CLogin, rsp);
        return;
    }

    if(user_map_.find(user.id) != user_map_.end()) {
        rsp.set_code(ALREADY_LOGON);
        sender->Send(kS2CLogin, rsp);
        return;
    }
    else {
        user_map_[user.id] = user;
        session_map_[sender->GetId()] = user.id;
        rsp.set_code(SUCCESS);
        rsp.set_id(user.id);
        rsp.set_account(user.account);
        sender->Send(kS2CLogin, rsp);

        proto::EventBuff * send_buff = CreateEvtBuf(kEventUserOnline);
        send_buff->Append(user.id);
        send_buff->Append(sender);
        send_buff->Append(user.account);
        ExecEvent(send_buff);
    }
}

void UserMgr::Logout(ISender * sender, LogoutReq & logout_req)
{
    ErrorCode rsp;

    // 注销操作
    //ExecEvent(kEventUserOffline, nullptr, 0);
    rsp.set_code(SUCCESS);
    sender->Send(kS2CRegister, rsp);
}

void UserMgr::Register(ISender * sender, Player & player)
{
    RegisterResp rsp;
    std::string error_message;

    if(!ValidateRegisterReq(player, error_message)) {
        rsp.set_code(ACCOUNT_INVALIDATE);
        rsp.set_desc(error_message);
        sender->Send(kS2CRegister, rsp);
        return;
    }

    std::string name = player.name();
    std::string password = player.password();
    std::string account = player.account();
    bool male = player.sex() == Player::MALE ? true : false;

    if(user_dao_.IsExist(account)) {
        rsp.set_code(ACCOUNT_HAS_EXIST);
        sender->Send(kS2CRegister, rsp);
        return;
    }

    user_dao_.AddNewAccount(account, password, name, male);
    rsp.set_code(SUCCESS);
    sender->Send(kS2CRegister, rsp);
}

bool UserMgr::ValidateRegisterReq(const user::Player & register_req, std::string & error_message)
{
    if (!IsValidAccount(register_req.account()))
    {
        error_message = "account invalid, must be 3-16 number of alpha or digit";
        logger_debug("{}", error_message);
        return false;
    }
    if (!IsValidPassword(register_req.password()))
    {
        error_message = "password invalid";
        logger_debug("{}", error_message);
        return false;
    }
    if (register_req.name().length() < 1 || register_req.name().length() > 16)
    {
        error_message = "name invalid";
        logger_debug("{}", error_message);
        return false;
    }

    return true;
}

void UserMgr::OnConnectClose(proto::EventBuff & buff)
{
    int32_t sid = 0;
    buff >> sid;

    auto find = session_map_.find(sid);
    if(find != session_map_.end()) {
        user_map_.erase(find->second);
        session_map_.erase(sid);

        proto::EventBuff * send_buff = CreateEvtBuf(kEventUserLostConnect);
        int32_t uid = find->second;
        send_buff->Append(uid);
        ExecEvent(send_buff);
        logger_warn("user {} lose connection", uid);
    }
}
