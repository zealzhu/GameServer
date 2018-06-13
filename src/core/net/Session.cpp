#include "Session.h"
#include "module/ModuleMgr.h"
#include "msg/MsgMgr.h"

std::list<uint32_t> Session::reuse_ids_;
uint32_t  Session::current_id_ = 0;

Session::Session(tcp::socket socket)
	: socket_(std::move(socket)) {
	session_id_ = GenSessionId();
}

Session::~Session()
{
	reuse_ids_.emplace_back(session_id_);
}

void Session::DoRead(IContext * pContext)
{
	auto self(shared_from_this());
	socket_.async_read_some(asio::buffer(data_, MAX_LENGTH),
		[this, self, pContext](std::error_code ec, std::size_t length)
	{
		if (ec || length < 4) {
			ErrorLog("Read error: %s", ec.message().c_str());
			return;
		}

		auto size = ReadHdr(data_);
		if (size != (length - sizeof(uint32_t))) {
			ErrorLog("Size error: %u", size);
			return;
		}
		auto msg = Decode(data_ + sizeof(uint32_t), size);
		SessionMessage smsg{ session_id_, msg };
		MsgMgr::Instance()->InsertMessage(smsg);

		DoRead(pContext);
	});

}

uint32_t Session::GenSessionId()
{
	uint32_t id = ++current_id_;
	if (reuse_ids_.size() > 0) {
		id = reuse_ids_.front();
		reuse_ids_.pop_front();
	}
	return id;
}
