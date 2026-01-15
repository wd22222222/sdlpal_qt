#pragma once
#ifndef CONVERS_H
#define CONVERS_H

#include "sdl2_compat.h"
#include <string>
#include <vector>
#ifndef  _WIN32
#include <iconv.h>
#endif
//

class Cls_Iconv
{
public:
#ifndef  _WIN32
    static std::string convert(const std::string& srcStr, LPCSTR to, LPCSTR from)
    {
        iconv_t cd = iconv_open(to, from);
        if (cd == (iconv_t)-1)
        {
            return std::string();
        }
        std::string dstStr;
        dstStr.resize(srcStr.size() * 4);
        char* inBuf = const_cast<char*>(srcStr.data());
        size_t inBytesLeft = srcStr.size();
        char* outBuf = dstStr.data();
        size_t outBytesLeft = dstStr.size();
        if (iconv(cd, &inBuf, &inBytesLeft, &outBuf, &outBytesLeft) == (size_t)-1)
        {
            iconv_close(cd);
            return std::string();
        }
        dstStr.resize(dstStr.size() - outBytesLeft);
        iconv_close(cd);
        return dstStr;
    }
#else
	//windows下使用API函数
	//使用windows API函数MultiByteToWideChar和WideCharToMultiByte
	//进行字符编码转换
    static std::wstring convertCodetoWstring(const std::string& srcStr, LPCSTR code)
    {
        UINT32 incodePage = 0; //输入编码
        if (code == "gbk")
        {
            incodePage = 936; //GBK
        }
        else if (code == "BIG5")
        {
            incodePage = 950; //BIG5
        }
        else if (code == "utf-8")
        {
            incodePage = CP_UTF8;
        }
        //计算转换的字符数
        int iLen = MultiByteToWideChar(incodePage, 0, srcStr.c_str(), -1, nullptr, 0);
        //给wszUnicode分配内存
        std::wstring dstStr;
        dstStr.resize(iLen + 1);
        //转换GBK码到Unicode码，使用了API函数MultiByteToWideChar
        MultiByteToWideChar(incodePage, 0, srcStr.c_str(), -1, (LPWSTR)dstStr.c_str(), iLen);
        return dstStr;
    }
    
    static std::string convertWstringToCode(const std::wstring& srcStr, LPCSTR code)
    {
		Uint32 outcodePage = 0; //输出编码
		if (code == "gbk")
		{
			outcodePage = 936; //GBK
		}
		else if (code == "BIG5")
		{
			outcodePage = 950; //BIG5
		}
		else if (code == "utf-8")
		{
			outcodePage = CP_UTF8;
		}
		//计算转换的字符数
		int iLen = WideCharToMultiByte(outcodePage, 0, srcStr.c_str(), -1, nullptr, 0, NULL, NULL);
		//给pszGbt分配内存
		std::string dstStr;
		dstStr.resize(iLen + 4);
		//转换Unicode码到GBK码，使用API函数WideCharToMultiByte
		WideCharToMultiByte(outcodePage, 0, srcStr.c_str(), -1, (LPSTR)dstStr.c_str(), iLen, NULL, NULL);
		return dstStr;
	}

    static std::string convert(const std::string& srcStr, LPCSTR to, LPCSTR from)
	{
		std::wstring wstr = convertCodetoWstring(srcStr, from);
		std::string str = convertWstringToCode(wstr, to);
		return str;
    }
#endif

    static std::string UTF8toGBK(const std::string& srcStr)
    {
        return convert(srcStr, "gbk", "utf-8");
    }
    static std::string GBKtoUTF8(const std::string& srcStr)
    {
        return convert(srcStr, "utf-8", "gbk");
    }
    static std::string BIG5toUTF8(const std::string& srcStr)
    {
        return convert(srcStr, "utf-8", "BIG5");
    };
    static std::string UTF8toBIG5(const std::string& srcStr)
    {
        return convert(srcStr, "BIG5", "utf-8");
    }
    
    static std::wstring UTF8toWCHAR(const std::string& str) {
        return StringToWstring(str);
    }
    
    static std::string WCHARtoUTF8(const std::wstring& str) {
        return WStringToString(str);
    }

#ifdef  _WIN32
    //以下转换使用windows API
    static std::string WStringToString(const std::wstring& wStr)
    {
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wStr[0], (int)wStr.size(), NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wStr[0], (int)wStr.size(), &strTo[0], size_needed, NULL, NULL);
        return strTo;
    }
    static std::wstring StringToWstring(const std::string& srcStr)
    {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &srcStr[0], (int)srcStr.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &srcStr[0], (int)srcStr.size(), &wstrTo[0], size_needed);
        return wstrTo;
    }
#else
    static std::string WstringToString(const std::wstring& wStr)
    {
        iconv_t cd = iconv_open("utf-8", "wchar_t");
        if(cd== (iconv_t)-1)
        {
            return std::string();
        }
        std::string dstStr;
        char* inBuf = reinterpret_cast<char*>(const_cast<wchar_t*>(wStr.data()));
        dstStr.resize((size_t)(wStr.size() * 4 + 1));
        size_t inBytesLeft = wStr.size();
        char* outBuf = dstStr.data();
        size_t outBytesLeft = dstStr.size();
        if (iconv(cd, &inBuf, &inBytesLeft, &outBuf, &outBytesLeft) == (size_t)-1)
        {
            iconv_close(cd);
            return std::string();
        }
        dstStr.resize(dstStr.size() - outBytesLeft);
        iconv_close(cd);
        return dstStr;
    }

    static std::wstring StringToWstring(const std::string& srcStr)
    {
        iconv_t cd = iconv_open("wchar_t","utf-8");
        if (cd == (iconv_t)-1)
        {
            return std::wstring();
        }
        std::wstring dstStr;
        dstStr.resize(srcStr.size() * 4 + 1);
        char* inBuf = const_cast<char*>(srcStr.data());
        size_t inBytesLeft = srcStr.size();
        char* outBuf = (char*)(dstStr.data());
        size_t outBytesLeft = dstStr.size();
        if (iconv(cd, &inBuf, &inBytesLeft, &outBuf, &outBytesLeft) == (size_t)-1)
        {
            iconv_close(cd);
            return std::wstring();
        }
        dstStr.resize(dstStr.size() - outBytesLeft);
        iconv_close(cd);
        return dstStr;
    }
#endif
    //繁体到简体
    std::string Big5ToGb2312(const std::string& SrcStr);
    //简体到繁体
    std::string Gb2312ToBig5(const std::string SrcStr);

};

extern "C"
//usrd windows
{
}
#endif // ! CONVERS_H

