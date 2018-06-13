#ifndef _TOOLS_HPP
#define _TOOLS_HPP

#include <string>
#include <chrono>

#ifdef WIN32
#include <direct.h>
#include <Windows.h>
#elif linux
#include <iconv.h>
#include <sys/stat.h>
#include <string.h>
#endif

namespace tools
{
	static bool IsDirExist(const std::string & dir_path)
	{
		struct stat info;

		if (stat(dir_path.c_str(), &info) != 0)
			return false;

		if (info.st_mode & S_IFDIR)
			return true;

		return false;
	}

	static void CreateDir(const std::string & dir_path)
	{
#ifdef WIN32
		_mkdir(dir_path.c_str());
#else
		mkdir(dir_path.c_str(), 0777);
#endif
	}

#ifdef linux
	static int CodeConvert(char *from_charset, char *to_charset, char *inbuf, int inlen, char *outbuf, int outlen)
	{
		iconv_t cd;
		char **pin = &inbuf;
		char **pout = &outbuf;

		cd = iconv_open(to_charset, from_charset);
		if (cd == 0)
			return -1;
		memset(outbuf, 0, outlen);
		if (iconv(cd, pin, (size_t*)&inlen, pout, (size_t*)&outlen) == -1)
			return -1;
		iconv_close(cd);
		return 0;
	}
#endif // linux

	static void GBKToUTF8(std::string& strGBK)
	{
#ifdef  WIN32
		int len = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
		wchar_t * wszUtf8 = new wchar_t[len];
		memset(wszUtf8, 0, len);
		MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, wszUtf8, len);

		len = WideCharToMultiByte(CP_UTF8, 0, wszUtf8, -1, NULL, 0, NULL, NULL);
		char *szUtf8 = new char[len + 1];
		memset(szUtf8, 0, len + 1);
		WideCharToMultiByte(CP_UTF8, 0, wszUtf8, -1, szUtf8, len, NULL, NULL);

		strGBK = szUtf8;
		delete[] szUtf8;
		delete[] wszUtf8;
#elif linux
		char buff[8192] = { 0 };
		CodeConvert((char*)"gb2312", (char*)"utf-8",
                const_cast<char *>(strGBK.c_str()), strGBK.size(), buff, 8192);
		strGBK = buff;
#endif //  WIN32
	}

	static void UTF8ToGBK(std::string & strUTF8)
	{
#ifdef  WIN32
		int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, NULL, 0);
		wchar_t * wszGBK = new wchar_t[len];
		memset(wszGBK, 0, len);
		MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, wszGBK, len);

		len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
		char *szGBK = new char[len + 1];
		memset(szGBK, 0, len + 1);
		WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);

		strUTF8 = szGBK;
		delete[] szGBK;
		delete[] wszGBK;
#elif linux
		char buff[8192] = { 0 };
		CodeConvert((char*)"utf-8", (char*)"gb2312",
                const_cast<char *>(strUTF8.c_str()), strUTF8.size(), buff, 8192);
		strUTF8 = buff;
#endif //  WIN32
	}

	static int64_t GetSecTime() {
		return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	static int64_t GetMillTime() {
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	static int64_t GetNanoTime() {
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}
} // tools



#endif // _TOOLS_HPP
