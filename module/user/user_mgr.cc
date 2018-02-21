#include "user_mgr.h"
#include "protobuf_processor.hpp"
#include "event_processor.hpp"

using namespace user;

bool UserMgr::Initialize()
{
    RegProtoHandler(this, kC2SLogin, &UserMgr::Login);
    RegProtoHandler(this, kC2SLogout, &UserMgr::Logout);
    RegProtoHandler(this, kC2SRegister, &UserMgr::Register);

    RegEvent(this, kEventConnectionClose, &UserMgr::OnConnectClose);
    return true;
}

bool UserMgr::Start()
{
}

void UserMgr::Stop()
{}

void UserMgr::Login(ISender * sender, LoginReq & login_req)
{
    LoginResp rsp;

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
    ErrorCode rsp;

    if(!ValidateRegisterReq(player)) {
        rsp.set_code(ACCOUNT_INVALIDATE);
        sender->Send(kS2CRegister, rsp);
        return;
    }

    rsp.set_code(SUCCESS);
    sender->Send(kS2CRegister, rsp);
}

bool UserMgr::ValidateRegisterReq(const user::Player & pRegisterReq)
{
    //if (!CStringUtils::IsValidAccount(pRegisterReq->account()))
    //{
        //errorMessage.set_desc("account invalid, must be 3-16 number of alpha or digit");
        //return false;
    //}
    //if (!CStringUtils::IsValidPassword(pRegisterReq->password()))
    //{
        //errorMessage.set_desc("password invalid");
        //return false;
    //}
    //if (pRegisterReq->name().length() < 1 || pRegisterReq->name().length() > 16)
    //{
        //errorMessage.set_desc("name invalid");
        //return false;
    //}
    //if (pRegisterReq->email().length() > 0 && !CStringUtils::IsValidMail(pRegisterReq->email()))
    //{
        //errorMessage.set_desc("email invalid");
        //return false;
    //}
    //for (int i = 0; i < pRegisterReq->phones_size(); i++)
    //{
        //if (zhu::user::Player_PhoneType_MOBILE == pRegisterReq->phones(i).type()
            //&& !CStringUtils::IsValidMobile(pRegisterReq->phones(i).number()))
        //{
            //errorMessage.set_desc("mobile invalid");
            //return false;
        //}
        //else if (zhu::user::Player_PhoneType_HOME == pRegisterReq->phones(i).type()
            //&& !CStringUtils::IsValidHomeNumber(pRegisterReq->phones(i).number()))
        //{
            //errorMessage.set_desc("home phone invalid");
            //return false;
        //}
    //}

    return true;
}

void UserMgr::OnConnectClose(proto::EventBuff & buff)
{
    int32_t sid = 0;
    buff >> sid;

    proto::EventBuff * send_buff = CreateEvtBuf(kEventUserLostConnect);
    auto find = user_map_.find(sid);
    if(find != user_map_.end()) {
        int32_t uid = find->first;
        send_buff->Append(uid);
        ExecEvent(send_buff);
    }
}
