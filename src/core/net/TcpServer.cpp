#include "TcpServer.h"
#include <exception>
#include "config/ConfigMgr.h"

using namespace asio;
using namespace asio::ip;

IContext * TcpServer::pContext = nullptr;

TcpServer::TcpServer()
	: acceptor_(io_context_)
{

}

bool TcpServer::Initialize(IContext * pContext)
{
	TcpServer::pContext = pContext;
	port_ = ConfigMgr::Instance()->GetCoreConfig().port;
	InfoLog("Init net. Listen port %02x", port_);
	return true;
}

void TcpServer::Start()
{
	InfoLog("Server start.");
	net_thread_ = std::thread([this]() {
		asio::error_code ec;
		auto ep = tcp::endpoint(tcp::v4(), port_);
		acceptor_.open(ep.protocol());
		acceptor_.set_option(socket_base::reuse_address(true));
		acceptor_.bind(ep);
		acceptor_.listen();
		acceptor_.async_accept(std::bind(&TcpServer::OnAccept, this, std::placeholders::_1, std::placeholders::_2));
		io_context_.run();
	});
	
}

void TcpServer::Destory()
{
	io_context_.stop();
	net_thread_.join();
	InfoLog("Net server stop.");
}

void TcpServer::OnAccept(asio::error_code ec, asio::ip::tcp::socket sock)
{
	auto session = std::make_shared<Session>(std::move(sock));
	session->Start(pContext);
	acceptor_.async_accept(std::bind(&TcpServer::OnAccept, this, std::placeholders::_1, std::placeholders::_2));
	sessions_[session->GetId()] = session;
	DebugLog("New connection. Ip: %s", session->GetIp().c_str());
}
