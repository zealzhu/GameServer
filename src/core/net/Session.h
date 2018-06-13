#ifndef _SESSION_HPP
#define _SESSION_HPP

#include <memory>
#include <asio.hpp>
#include <iostream>
#include <ByteBuffer.hpp>
#include "Code.hpp"
#include <IContext.h>

using asio::ip::tcp;

class Session
    : public std::enable_shared_from_this<Session>
{
public:
	Session(tcp::socket socket);

	~Session();
	

	inline uint32_t GetId() { return session_id_; }

	inline void Start(IContext * context) { DoRead(context); }

	void DoRead(IContext * pContext);

	inline std::string GetIp() { return socket_.remote_endpoint().address().to_string(); }

private:
	static uint32_t GenSessionId();

private:
	enum { MAX_LENGTH = 1024, };

    tcp::socket socket_;
    char data_[MAX_LENGTH];
	int64_t session_id_;

	static std::list<uint32_t> reuse_ids_;
	static uint32_t current_id_;
};

#endif