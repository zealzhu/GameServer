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
	// 4���ֽڵ�ͷ�������ȣ�+���ݰ����ݳ���
	buf_size = msg.ByteSize() + HEADER_SIZE;
	// ����һ���ô�С�Ļ�����
	char *buf = new char[buf_size];
	// ����
	memset(buf, buf_size, 0);
	// �������ֽ���ת��Ϊ�����ֽ���
	uint32_t header_size = htonl(msg.ByteSize());
	// ͷ�ĸ��ֽ������洢���ݰ���С
	memcpy(buf, &header_size, HEADER_SIZE);
	// ���¾��ǽ�protobuf�������л����ڴ���
	google::protobuf::io::ArrayOutputStream aos(buf + HEADER_SIZE, msg.ByteSize());
	google::protobuf::io::CodedOutputStream code_output(&aos);
	msg.SerializeToCodedStream(&code_output);

	// ���ر����Ļ���������Ҫ�ֶ��Լ��ͷ�
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
