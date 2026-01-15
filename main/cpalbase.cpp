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


//本类定义文件系统相关
 
#include <sys/stat.h>
#include "sdl2_compat.h"
#include <iostream>
#include <cassert>
#include <memory>
#include <direct.h>
#include "cpalbase.h"
#include "cconfig.h"
#include "cpaldata.h"

LPCSTR PalFileName[] =
{
	"abc.mkf",
	"fbp.mkf",
	"mgo.mkf",
	"ball.mkf",
	"data.mkf",
	"f.mkf",
	"fire.mkf",
	"rgm.mkf",
	"sss.mkf",
	"map.mkf",
	"gop.mkf",
	"m.msg",
	"word.dat",
	"desc.dat",
	nullptr,
};
 
CPalBase::CPalBase()
{
}

CPalBase::~CPalBase()
{
	if (gpGlobals)
		delete gpGlobals;
	gpGlobals = nullptr;
}

PalErr CPalBase::InitPalBase()
{
	char buf[1200];
	std::string s;
#ifdef _WIN32
	if (_getcwd(buf, 1200) != nullptr) {
		s = buf;
		s += "\\";
	}
#else
	if (getcwd(buf, 1200) != nullptr) {
		s = buf;
	}
#endif

	if (!(isCorrectDir(s)|| isCorrectDir(s += "pal\\")))//s 不是游戏目录
	{
#if(_DEBUG && _IN_CMAKE)
		//使用cmake时有效，可以换成其他调试目录
		if (PalDir.empty())
			PalDir = s ="d:\\pal98\\";
		if (!(isCorrectDir(s) || isCorrectDir(s += "pal\\")))//s 不是游戏目录
#endif
		return 1;
	}
	PalDir = s;
	//以下装入游戏数据
	if (!gpGlobals)
	{
		gpGlobals = new CPalData(PalDir,p_DataCopy);
		if (!gpGlobals )
			return 1;
		if (gpGlobals->isErr)
		{
			delete gpGlobals;
			gpGlobals = nullptr;
			return 1;
		}
	}
	
	ggConfig = gpGlobals->gConfig;
	return 0;//成功
}

VOID CPalBase::destroyPalBase()
{
	if (!CheckHeapIntegrity())
		delete gpGlobals;
	else
		;
	gpGlobals = nullptr;
	//gConfig = nullptr;
}

BOOL CPalBase::isCorrectDir(const std::string & dir)
{
	
	std::string cdir;
	for (int i = 0; PalFileName[i]; i++)
	{
		cdir = dir;
		cdir += PalFileName[i];
		if((!CPalData::IsFileExist(cdir)) && std::string( PalFileName[i]) != "desc.dat")
			return FALSE;
	}

	//是正确的目录，往下进行
	return 1;
}


;
