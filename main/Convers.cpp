///* -*- mode: c; tab-width: 4; c-basic-offset: 4; c-file-style: "linux" -*- */
//
// Copyright (c) 2021-2026, Wu Dong.
// 
// All rights reserved.
//
// This file is part of SDLPAL.
//
// SDLPAL is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifdef _WIN32
#include <Windows.h>
#endif
#include <string>
#include "Convers.h"

using namespace std;

#ifdef _WIN32
std::string Cls_Iconv::Big5ToGb2312(const std::string& SrcStr)
{
    //1、转换Big5到Unicode
    //2、转换Unicode到Gb繁体
    //3、转换Gb繁体到Gb简体

    std::string pszBig5; //Big5编码的字符
    std::wstring wszUnicode{ 0 }; //Unicode编码的字符
    std::string pszGbt{ 0 }; //Gb编码的繁体字符
    std::string pszGbs; //Gb编码的简体字符
    std::string sGb; //返回的字符串
    pszBig5 = SrcStr; //读入需要转换的字符参数
    //计算转换的字符数
    auto lLen = MultiByteToWideChar(950, 0, pszBig5.c_str(), -1, NULL, 0);
    //给wszUnicode分配内存
    wszUnicode.resize(lLen + 10);
    //转换Big5码到Unicode码，使用了API函数MultiByteToWideChar
    MultiByteToWideChar(950, 0, pszBig5.c_str(), -1, (LPWSTR)wszUnicode.c_str(), lLen);
    //计算转换的字符数
    auto iLen = WideCharToMultiByte(936, 0, (PWSTR)wszUnicode.c_str(), -1, NULL, 0, NULL, NULL);  //给pszGbt分配内存
    pszGbt.resize(iLen + 4);
    //给pszGbs分配内存
    pszGbs.resize(iLen + 4);
    //转换Unicode码到Gb码繁体，使用API函数WideCharToMultiByte
    WideCharToMultiByte(936, 0, (PWSTR)wszUnicode.c_str(), -1, (LPSTR)pszGbt.c_str(), iLen, NULL, NULL);
    //转换Gb码繁体到Gb码简体，使用API函数LCMapString
    LCMapStringA(0x0804, LCMAP_SIMPLIFIED_CHINESE, pszGbt.c_str(), -1, (LPSTR)pszGbs.c_str(), iLen);
    //返回Gb码简体字符
    sGb = pszGbs;
    return sGb;
}
    //usrd windows

    //简体到繁体
std::string Cls_Iconv::Gb2312ToBig5(const std::string SrcStr)
{
    char* pszGbt = NULL; //Gb编码的繁体字符
    char* pszGbs = NULL; //Gb编码的简体字符
    wchar_t* wszUnicode = NULL; //Unicode编码的字符
    char* pszBig5 = NULL; //Big5编码的字符
    string sBig5; //返回的字符串
    int iLen = 0; //需要转换的字符数

    pszGbs = (char*)((LPCSTR)SrcStr.c_str()); //读入需要转换的字符参数

    //计算转换的字符数
    iLen = MultiByteToWideChar(936, 0, pszGbs, -1, NULL, 0);

    //给pszGbt分配内存
    pszGbt = new char[iLen * 2 + 1];
    //转换Gb码简体到Gb码繁体，使用API函数LCMapString
    LCMapStringA(0x0804, LCMAP_TRADITIONAL_CHINESE, pszGbs, -1, pszGbt, iLen * 2);

    //给wszUnicode分配内存
    wszUnicode = new wchar_t[iLen + 1];
    //转换Gb码到Unicode码，使用了API函数MultiByteToWideChar
    MultiByteToWideChar(936, 0, pszGbt, -1, wszUnicode, iLen);

    //计算转换的字符数
    iLen = WideCharToMultiByte(950, 0, (PWSTR)wszUnicode, -1, NULL, 0, NULL, NULL);
    //给pszBig5分配内存
    pszBig5 = new char[iLen + 1];
    //转换Unicode码到Big5码，使用API函数WideCharToMultiByte
    WideCharToMultiByte(950, 0, (PWSTR)wszUnicode, -1, pszBig5, iLen, NULL, NULL);
    //返回Big5码字符
    sBig5 = pszBig5;
    //释放内存
    delete[] wszUnicode;
    delete[] pszGbt;
    delete[] pszBig5;
    //返回
    return sBig5;
}

#else

    //繁体-->简体
std::string  Cls_Iconv::Big5ToGb2312(const std::string& SrcStr)
{
    return SrcStr;
}
    //简体到繁体
std::string Cls_Iconv::Gb2312ToBig5(const string& _SrcString)
{
    return _SrcString;
}

#endif
