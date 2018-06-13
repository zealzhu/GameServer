#ifndef _TCP_SERVER_H
#define _TCP_SERVER_H

#include <asio.hpp>
#include "IContext.h"
#include <unordered_map>
#include "Session.h"
#include <thread>

class TcpServer
{
public:
	static TcpServer * Instance() {
		static TcpServer instance;
		return &instance;
	}
	bool Initialize(IContext * context);
	void Start();
	void Destory();

private:
	TcpServer();
	void OnAccept(asio::error_code ec, asio::ip::tcp::socket sock);

private:
	typedef std::weak_ptr<Session> SessionPtr;

	std::thread net_thread_;

	asio::io_context io_context_;
	asio::ip::tcp::acceptor acceptor_;
	std::unordered_map<uint32_t, SessionPtr> sessions_;
	uint16_t port_;

	static IContext * pContext;
};

#endif // _TCP_SERVER_H
