#ifndef _CODE_HPP
#define _CODE_HPP

#include <message.pb.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <memory>

typedef std::shared_ptr<BaseMessage> MessagePtr;

static const char* Encode(const google::protobuf::Message& msg, uint32_t & buf_size)
{
	const uint32_t HEADER_SIZE = sizeof(uint32_t);
	// 4个字节的头部（长度）+数据包内容长度
	buf_size = msg.ByteSize() + HEADER_SIZE;
	// 创建一个该大小的缓存区
	char *buf = new char[buf_size];
	// 清零
	memset(buf, buf_size, 0);
	// 将本地字节流转换为网络字节流
	uint32_t header_size = htonl(msg.ByteSize());
	// 头四个字节用来存储数据包大小
	memcpy(buf, &header_size, HEADER_SIZE);
	// 以下就是将protobuf数据序列化到内存中
	google::protobuf::io::ArrayOutputStream aos(buf + HEADER_SIZE, msg.ByteSize());
	google::protobuf::io::CodedOutputStream code_output(&aos);
	msg.SerializeToCodedStream(&code_output);

	// 返回编码后的缓存区，需要手动自己释放
	return buf;
}

static uint32_t ReadHdr(char *pBuf)
{
	uint32_t size;
	//Decode the HDR and get the size
	//iSize = (pBuf[0] << 24) | (pBuf[1] << 16) | (pBuf[2] << 8) | pBuf[3];
	size = ntohl(*(uint32_t *)pBuf);
	return size;
}

static MessagePtr Decode(const char * buf, uint32_t size)
{
	uint32_t byte_count = 0;
	MessagePtr msg_ptr(new BaseMessage());
	////Read the entire buffer including the hdr
	//unsigned uiTotal = 0;
	//while (uiTotal < iSize && (uiByteCount = recv(csock, pBuffer + uiByteCount, iSize - uiByteCount, 0)) > 0)
	//{
	//	if (uiByteCount == -1)
	//	{
	//		SAFE_DELETE_ARRAY(pBuffer);
	//		throw SocketLib::Exception(GetError());
	//	}

	//	uiTotal += uiByteCount;
	//}
	//logger_trace("Second read byte count is {}", uiByteCount);
	//Assign ArrayInputStream with enough memory
	google::protobuf::io::ArrayInputStream ais(buf, size);
	google::protobuf::io::CodedInputStream coded_input(&ais);
	//De-Serialize
	msg_ptr->ParseFromCodedStream(&coded_input);

	return msg_ptr;
}

#endif // _CODE_HPP
